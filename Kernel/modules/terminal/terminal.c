#include <terminal.h>
#include <files.h>
#include <video.h>
#include <keyboard.h>
#include <memory.h>
#include <lib.h>
#include <processes.h>
#include <semaphores.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define TEXT_BUFFER_SIZE 32768  // 32KB total

#define MIN_TEXT_SIZE 12
#define MAX_TEXT_SIZE 64
#define MAX_SCREEN_LINES (768 / MIN_TEXT_SIZE)  // 64 lineas
#define LINES_TO_REMEMBER (MAX_SCREEN_LINES * 3 / 2)  // ~1.5 pantallas

extern Process processes[];
extern size_t actual_index;
extern void _sti();
extern void _cli();

static unsigned char text_buffer[TEXT_BUFFER_SIZE];
static int cursor = 0;
static int offset = 0;
static char selected_style = 0x0F;
static uint16_t text_size = 16;

static pid_t foreground_pid = 0;

static int terminal_mode = TERM_COOKED;

// Linea cocida lista para entregar byte a byte a sys_read (la arma terminal_read
// haciendo echo y edicion). Solo el foreground la consume, asi que un unico
// estado estatico alcanza.
#define LINE_MAX 256
static char ready_line[LINE_MAX];
static int  ready_len = 0;
static int  ready_pos = 0;

typedef struct terminal_t {
    int dummy;
} terminal_t;

static terminal_t g_terminal;
static file_t * g_terminal_fd = NULL;

static file_ops_t terminal_ops;

static void addToBuffer(char c);
static void printBuffer(int from, int to, bool fullRedraw);
static int checkAndAutoScroll(bool redraw_all);
static void checkBufferOverflow(void);
static int calculateTailOffset(void);

int write_terminal_fd(file_t *f, const char *buf, int count);
int close_terminal_fd(file_t *f);
static int terminal_read(file_t *f, char *buf, int count);

// API de texto (la que antes estaba en video.c)

void selectStyle(char style) {
    selected_style = style;
}

void setTextSize(uint16_t height) {
    if (height < MIN_TEXT_SIZE || height > MAX_TEXT_SIZE) return;
    uint16_t old_size = text_size;
    text_size = height;
    if (old_size != text_size) checkAndAutoScroll(true);
    printBuffer(offset, cursor, true);
}

void textMode() {
    setTextMode(true);
    clearCanvas();
    swapBuffers();
    checkAndAutoScroll(true);
    printBuffer(offset, cursor, true);
}

void clearTextBuffer() {
    for (int i = 0; i < cursor; i++) {
        text_buffer[i] = 0;
    }
    cursor = 0;
    offset = 0;
    clearCanvas();
    swapBuffers();
}

void print(const char *text) {
    int initial = cursor;
    while (*text != 0) {
        addToBuffer(*text);
        text++;
    }

    if (isTextMode()) {
        if (checkAndAutoScroll(true)) {
            printBuffer(offset, cursor, true);
        } else {
            printBuffer(initial, cursor, false);
        }
    }
}

void printChar(char c) {
    addToBuffer(c);
    if (isTextMode()) {
        printBuffer(cursor - 2, cursor, false);
        if (checkAndAutoScroll(false)) {
            printBuffer(offset, cursor, true);
        }
    }
}

void deleteChar() {
    if (cursor <= 1) return;
    text_buffer[cursor - 2] = ' ';
    printBuffer(cursor - 2, cursor, false);
    text_buffer[cursor--] = 0;
    text_buffer[cursor--] = 0;
}

void scrollDown() {
    if (!isTextMode()) return;
    while (*(text_buffer + offset) != '\n') offset++;
    offset++;
    printBuffer(offset, cursor, true);
}

void scrollUp() {
    if (!isTextMode()) return;
    if (offset < 1) return;
    offset -= 2;
    while (*(text_buffer + offset) != '\n') offset--;
    offset++;
    printBuffer(offset, cursor, true);
}

void printHex64(uint64_t value) {
    char hex[17];
    const char *digits = "0123456789ABCDEF";
    for (int i = 15; i >= 0; i--) {
        hex[15 - i] = digits[(value >> (i * 4)) & 0xF];
    }
    hex[16] = '\0';
    print("0x");
    print(hex);
}

void printHex32(uint32_t value) {
    char hex[9];
    const char *digits = "0123456789ABCDEF";
    for (int i = 7; i >= 0; i--) {
        hex[7 - i] = digits[(value >> (i * 4)) & 0xF];
    }
    hex[8] = '\0';
    print("0x");
    print(hex);
}

void printHex16(uint16_t value) {
    char hex[5];
    const char *digits = "0123456789ABCDEF";
    for (int i = 3; i >= 0; i--) {
        hex[3 - i] = digits[(value >> (i * 4)) & 0xF];
    }
    hex[4] = '\0';
    print("0x");
    print(hex);
}

void printDec(uint64_t number) {
    char buffer[21];
    int i = 0;
    if (number == 0) {
        printChar('0');
        return;
    }
    while (number > 0) {
        buffer[i++] = '0' + (number % 10);
        number /= 10;
    }
    while (i--) {
        printChar(buffer[i]);
    }
}

// Helpers privados de buffer/render

static void addToBuffer(char c) {
    text_buffer[cursor++] = c;
    text_buffer[cursor++] = selected_style;
    checkBufferOverflow();
}

static void printBuffer(int from, int to, bool fullRedraw) {
    if (fullRedraw) {
        clearCanvas();
    }

    uint16_t scaled_width = getCharWidth(text_size);
    if (scaled_width == 0) scaled_width = 1;
    int chars_per_line = getMaxCols(text_size);
    if (chars_per_line <= 0) chars_per_line = 1;

    int start_index = fullRedraw ? offset : from;
    int current_x = 0;
    int current_y = 0;
    int col = 0;

    // Para dibujado incremental: ubicar (x,y) del char 'from'
    if (!fullRedraw) {
        for (int i = offset; i < from; i += 2) {
            char c = text_buffer[i];
            if (c == '\n') {
                current_x = 0;
                current_y += text_size;
                col = 0;
            } else if (c >= 32 && c <= 126) {
                current_x += scaled_width;
                col++;
                if (col >= chars_per_line) {
                    current_x = 0;
                    current_y += text_size;
                    col = 0;
                }
            }
        }
    }

    bool directWrite = !fullRedraw;
    int screen_height = (int) getScreenHeight();

    for (int i = start_index; i < to; i += 2) {
        if (current_y >= screen_height) break;

        char c = text_buffer[i];
        char style = text_buffer[i + 1];

        if (c == '\n') {
            current_x = 0;
            current_y += text_size;
            col = 0;
            continue;
        }
        if (c < 32 || c > 126) continue;

        drawStyledChar((uint64_t)current_x, (uint64_t)current_y, c, style, text_size, directWrite);

        current_x += scaled_width;
        col++;
        if (col >= chars_per_line) {
            current_x = 0;
            current_y += text_size;
            col = 0;
        }
    }

    if (fullRedraw) {
        swapBuffers();
    }
}

static int calculateTailOffset(void) {
    if (cursor <= 0) return 0;

    uint16_t max_lines = getMaxRows(text_size);
    uint16_t chars_per_line = getMaxCols(text_size);
    if (chars_per_line == 0) chars_per_line = 1;

    int capacity = max_lines + 2;
    if (capacity > (MAX_SCREEN_LINES + 2)) capacity = MAX_SCREEN_LINES + 2;

    int line_starts[MAX_SCREEN_LINES + 2];
    int count = 1;
    line_starts[0] = 0;
    int current_line_start = 0;
    int col = 0;

    for (int i = 0; i < cursor; i += 2) {
        char ch = text_buffer[i];
        bool start_new_line = false;

        if (ch == '\n') {
            current_line_start = i + 2;
            col = 0;
            start_new_line = true;
        } else if (ch >= 32 && ch <= 126) {
            col++;
            if (col >= chars_per_line) {
                current_line_start = i + 2;
                col = 0;
                start_new_line = true;
            }
        }

        if (start_new_line) {
            if (count == capacity) {
                for (int j = 0; j < capacity - 1; j++) {
                    line_starts[j] = line_starts[j + 1];
                }
                count = capacity - 1;
            }
            if (count < capacity) {
                line_starts[count++] = current_line_start;
            }
        }
    }

    if (count <= max_lines) return 0;

    int index = count - max_lines;
    if (index < 0) index = 0;
    if (index >= count) index = count - 1;
    return line_starts[index];
}

static int checkAndAutoScroll(bool redraw_all) {
    int previous_offset = offset;
    int optimal_offset = calculateTailOffset();

    if (redraw_all) {
        offset = optimal_offset;
    } else if (optimal_offset > offset) {
        offset = optimal_offset;
    }

    return offset != previous_offset;
}

static void checkBufferOverflow(void) {
    if (cursor < TEXT_BUFFER_SIZE - 512) return;

    int total_logical_lines = 0;
    for (int i = 0; i < cursor; i += 2) {
        if (text_buffer[i] == '\n') total_logical_lines++;
    }

    if (total_logical_lines > LINES_TO_REMEMBER) {
        int lines_to_delete = 20;
        int new_start = 0;
        int deleted = 0;
        for (int i = 0; i < cursor && deleted < lines_to_delete; i += 2) {
            if (text_buffer[i] == '\n') {
                deleted++;
                new_start = i + 2;
            }
        }
        int bytes_to_move = cursor - new_start;
        for (int i = 0; i < bytes_to_move; i++) {
            text_buffer[i] = text_buffer[new_start + i];
        }
        cursor = bytes_to_move;
        if (offset >= new_start) offset -= new_start;
        else offset = 0;
    }
}

// Foreground process

pid_t get_foreground_pid() {
    return foreground_pid;
}

void set_foreground_pid(pid_t pid) {
    foreground_pid = pid;
    // Al cambiar de foreground reseteamos a modo cooked y descartamos cualquier
    // entrada acumulada: el type-ahead del proceso anterior NO debe filtrarse al
    // nuevo (flush del anillo crudo + de la linea a medio entregar).
    terminal_mode = TERM_COOKED;
    clearKeyBuffer();
    ready_len = 0;
    ready_pos = 0;
}

void terminal_set_mode(int mode) {
    terminal_mode = (mode == TERM_RAW) ? TERM_RAW : TERM_COOKED;
}

int terminal_get_mode(void) {
    return terminal_mode;
}

// Arma una linea en modo cooked: bloquea hasta recibir un '\n', haciendo echo y
// edicion (backspace) sobre el texto. Corre en el contexto del proceso que llamo
// read() (puede bloquearse). Solo el foreground drena el teclado; los demas
// esperan su turno. Devuelve la cantidad de bytes en ready_line (incluido '\n').
static int read_cooked_line(void) {
    int len = 0;
    while (1) {
        uint8_t raw = 0;
        // Si no soy el foreground, o el anillo esta vacio, me bloqueo y reintento.
        // El '||' corta: si no soy foreground NO llamo raw_pop (no robo la tecla).
        while (get_actual_pid() != foreground_pid || !raw_pop(&raw)) {
            kbd_block_self();
        }

        KeyEvent ev = decode_scancode(raw);
        if (ev.is_release) continue;

        if (ev.ascii == 0x04) {   // Ctrl+D (EOF)
            break;
        }

        if (ev.ascii == '\n') {
            sem_wait("tty_out");
            print("\n");
            sem_post("tty_out");
            ready_line[len++] = '\n';   // incluimos el '\n' como terminador
            break;
        } else if (ev.ascii == '\b') {
            if (len > 0) {
                len--;
                sem_wait("tty_out");
                deleteChar();
                sem_post("tty_out");
            }
        } else if (ev.printable && len < LINE_MAX - 1) {
            ready_line[len++] = ev.ascii;
            sem_wait("tty_out");
            printChar(ev.ascii);
            sem_post("tty_out");
        }
    }
    return len;
}

// File-op read de la terminal: la terminal ES el stdin del teclado. sys_read la
// llama byte a byte; servimos desde la linea cocida ya armada y, cuando se agota,
// armamos una nueva (bloqueante). El type-ahead sobrante queda en el anillo crudo.
static int terminal_read(file_t *f, char *buf, int count) {
    (void) f;
    if (buf == NULL || count <= 0) return 0;

    if (ready_pos >= ready_len) {
        ready_len = read_cooked_line();
        ready_pos = 0;
    }

    int copied = 0;
    while (copied < count && ready_pos < ready_len) {
        buf[copied++] = ready_line[ready_pos++];
    }
    return copied;
}

//   File ops (write / close / create) La terminal queda como stdout/stderr (write a pantalla).
int write_terminal_fd(file_t *f, const char *buf, int count) {
    if (f == NULL || buf == NULL || count <= 0) return 0;

    // Un proceso en background SI puede escribir (no se bloquea): su salida se
    // intercala con la del foreground.
    sem_wait("tty_out");
    // El estilo es por-proceso: aplicamos el del que escribe, atomico bajo el
    // lock (asi dos procesos concurrentes no se pisan el color), y restauramos.
    char prev = selected_style;
    selectStyle(get_current_style());
    int i;
    for (i = 0; i < count; i++) {
        if (buf[i] == 0) break;  // null-terminator corta como print() clasico
        printChar(buf[i]);
    }
    selectStyle(prev);
    sem_post("tty_out");
    return i;
}

int close_terminal_fd(file_t *f) {
    if (f == NULL) return -1;
    if (--f->ref_count > 0) return 0;
    // El backing terminal_t es singleton: nunca se libera.
    if (f != g_terminal_fd) {
        free(f);
    }
    return 0;
}

static file_ops_t terminal_ops = {
    .read  = terminal_read,   // stdin del teclado: line discipline en contexto del lector
    .write = write_terminal_fd,
    .close = close_terminal_fd,
};

file_t * create_terminal_fd() {
    file_t *f = (file_t *) malloc(sizeof(file_t));
    if (f == NULL) return NULL;
    f->ops = &terminal_ops;
    f->data = &g_terminal;
    f->ref_count = 1;
    return f;
}

file_t * get_terminal_fd() {
    return g_terminal_fd;
}

void initialize_terminal() {
    if (g_terminal_fd != NULL) return;
    g_terminal_fd = create_terminal_fd();
}
