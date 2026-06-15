#include <stdint.h>
#include <keyboard.h>
#include <video.h>
#include <terminal.h>
#include <interrupts.h>
#include <audio.h>
#include <processes.h>
#include <memory.h>
#include <lib.h>
#include <time.h>
#include <pipes.h>
#include <semaphores.h>

#define READ_BUFFER_MAX 256

extern const char * get_register_dump();
extern Process processes[];
extern size_t actual_index;

static file_t * lookup_fd(int fd) {
    if (fd < 0 || fd >= MAX_FDS) return NULL;
    if (actual_index == (size_t)-1) return NULL;
    return processes[actual_index].fds[fd];
}

typedef struct {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rdi;
} Registers;

static int syscall_write(Registers * registers);
static int syscall_read(Registers * registers);
static int syscall_clear(Registers * registers);
static int syscall_graphics_mode(Registers * registers);
static int syscall_draw_line(Registers * registers);
static int syscall_draw_rectangle(Registers * registers);
static int syscall_draw_filled_rectangle(Registers * registers);
static int syscall_draw_fill_screen(Registers * registers);
static int syscall_draw_circle(Registers * registers);
static int syscall_draw_filled_circle(Registers * registers);
static int syscall_draw_text(Registers * registers);
static int syscall_clear_canvas(Registers * registers);
static int syscall_swap_buffers(Registers * registers);
static int syscall_time(Registers *registers);
static uint64_t syscall_ms(Registers *registers);
static KeyEvent * syscall_get_key(Registers *registers);
static uint64_t sycall_getRegs(Registers * registers);
static int syscall_set_text_size(Registers * registers);
static int syscall_set_fps_overlay(Registers * registers);
static int syscall_play_sound(Registers *registers);
static int syscall_is_audio_buffer_empty(Registers *registers);
static int syscall_clear_audio_buffer(Registers *registers);
static void * syscall_malloc(Registers *registers);
static int syscall_free(Registers *registers);
static uint64_t syscall_get_total_memory(Registers *registers);
static uint64_t syscall_get_used_memory(Registers *registers);
static uint64_t syscall_create_process(Registers *registers);
static int syscall_exit(Registers *registers);
static int syscall_wait(Registers *registers);
static int syscall_yield(Registers *registers);
static int syscall_set_foreground(Registers *registers);
static int syscall_write_fd(Registers *registers);
static int syscall_read_fd(Registers *registers);
static int syscall_close_fd(Registers *registers);
static int syscall_sem_open(Registers *registers);
static int syscall_sem_init(Registers *registers);
static int syscall_sem_post(Registers *registers);
static int syscall_sem_wait(Registers *registers);
static int syscall_sem_close(Registers *registers);
static int syscall_set_terminal_mode(Registers *registers);
static uint64_t syscall_getpid(Registers *registers);
static int syscall_kill(Registers *registers);
static int syscall_block(Registers *registers);
static int syscall_unblock(Registers *registers);
static int syscall_nice(Registers *registers);
static int syscall_select_style(Registers *registers);
static int syscall_toggle_block(Registers *registers);
static int syscall_get_process_list(Registers *registers);
static int syscall_create_pipe(Registers * registers);
static int  syscall_replace_process_fd(Registers * registers);

uint64_t sysCallDispatcher(Registers * registers) {
    switch ((*registers).rax) {
    case 1:
        return syscall_write(registers);
    case 2:
        return syscall_read(registers);
    case 3:
        return syscall_clear(registers);
    case 4:
        return syscall_graphics_mode(registers);
    case 5:
        return syscall_draw_line(registers);
    case 6:
        return syscall_draw_rectangle(registers);
    case 7:
        return syscall_draw_filled_rectangle(registers);
    case 8:
        return syscall_draw_circle(registers);
    case 9:
        return syscall_draw_filled_circle(registers);
    case 10:
        return syscall_draw_text(registers);
    case 11:
        return syscall_clear_canvas(registers);
    case 12:
        return syscall_swap_buffers(registers);
    case 13:
        return syscall_time(registers);
    case 14:
        return syscall_ms(registers);
    case 15:
        return (uint64_t)syscall_get_key(registers);
    case 16:
        return sycall_getRegs(registers);
    case 17:
        return syscall_set_text_size(registers);
    case 18:
        return syscall_play_sound(registers);
    case 19:
        return syscall_is_audio_buffer_empty(registers);
    case 20:
        return syscall_clear_audio_buffer(registers);
    case 21:
        return syscall_draw_fill_screen(registers);
    case 22:
        return syscall_set_fps_overlay(registers);
    case 23:
        return (uint64_t) syscall_malloc(registers);
    case 24:
        return syscall_free(registers);
    case 25:
        return syscall_get_total_memory(registers);
    case 26:
        return syscall_get_used_memory(registers);
    case 27:
        return syscall_create_process(registers);
    case 28:
        return syscall_exit(registers);
    case 29:
        return syscall_wait(registers);
    case 30:
        return syscall_yield(registers);
    case 31:
        return syscall_set_foreground(registers);
    case 32:
        return syscall_write_fd(registers);
    case 33:
        return syscall_read_fd(registers);
    case 34:
        return syscall_close_fd(registers);
    case 35:
        return syscall_sem_open(registers);
    case 36:
        return syscall_sem_init(registers);
    case 37:
        return syscall_sem_post(registers);
    case 38:
        return syscall_sem_wait(registers);
    case 39:
        return syscall_sem_close(registers);
    case 40:
        return syscall_set_terminal_mode(registers);
    case 41:
        return syscall_getpid(registers);
    case 42:
        return syscall_kill(registers);
    case 43:
        return syscall_block(registers);
    case 44:
        return syscall_unblock(registers);
    case 45:
        return syscall_nice(registers);
    case 46:
        return syscall_get_process_list(registers);
    case 47:
        return syscall_toggle_block(registers);
    case 48:
        return syscall_select_style(registers);
    case 49:
        return syscall_create_pipe(registers);
    case 50:
        return syscall_replace_process_fd(registers);
    default:
        break;
    }
    return 0;
}

static int syscall_replace_process_fd(Registers * registers){
    int indice = (int)registers->rbx;
    file_t * fd = (file_t*)registers->rcx;
    int pid = (int)registers->rdx;
    return replace_process_fd(indice, fd, pid);
}

static int syscall_create_pipe(Registers *  registers){
    file_t **fd=(file_t **)registers->rbx;
    return create_pipe(&(fd[0]), &(fd[1]));
}

static int syscall_set_terminal_mode(Registers * registers) {
    terminal_set_mode((int) registers->rbx);
    return 0;
}

static int syscall_select_style(Registers * registers) {
    set_current_style((char) registers->rbx);
    return 0;
}

static uint64_t syscall_getpid(Registers * registers) {
    (void) registers;
    return (uint64_t) get_actual_pid();
}

static int syscall_kill(Registers * registers) {
    kill_process((pid_t) registers->rbx);
    return 0;
}

static int syscall_block(Registers * registers) {
    block_process((size_t) registers->rbx);
    return 0;
}

static int syscall_toggle_block(Registers * registers) {
    return toggle_block_process((size_t) registers->rbx);
}

static int syscall_unblock(Registers * registers) {
    unblock_process((size_t) registers->rbx);
    return 0;
}

static int syscall_nice(Registers * registers) {
    modify_process_priority_by_pid((size_t) registers->rbx, (int) registers->rcx);
    return 0;
}

static int syscall_sem_open(Registers * registers) {
    const char * name = (const char *) registers->rbx;
    int initialValue = (int) registers->rcx;
    return sem_open(name, initialValue);
}

static int syscall_sem_init(Registers * registers) {
    const char * name = (const char *) registers->rbx;
    int initialValue = (int) registers->rcx;
    return sem_init(name, initialValue);
}

static int syscall_sem_post(Registers * registers) {
    return sem_post((const char *) registers->rbx);
}

static int syscall_sem_wait(Registers * registers) {
    return sem_wait((const char *) registers->rbx);
}

static int syscall_sem_close(Registers * registers) {
    return sem_close((const char *) registers->rbx);
}

static uint64_t syscall_create_process(Registers *registers) {
    const char *name = (const char *) registers->rbx;
    void (*entry)() = (void (*)()) registers->rcx;
    // argc en rdx (no se usa directamente; create_process lo recalcula desde argv)
    (void) registers->rdx;
    const char **argv = (const char **) registers->rdi;
    return (uint64_t) create_process(name, entry, argv);
}

static int syscall_exit(Registers *registers) {
    int status = (int) registers->rbx;
    exit_current_process(status);
    return 0;   // no llega
}

static int syscall_wait(Registers *registers) {
    pid_t pid = (pid_t) registers->rbx;
    return wait_pid(pid);
}

static int syscall_yield(Registers *registers) {
    (void) registers;
    yield();
    return 0;
}

static uint64_t sycall_getRegs(Registers * registers){
     return (uint64_t)get_register_dump();
}

static int syscall_write(Registers * registers) {
    int fd_num = (int) registers->rbx;
    const char * s = (const char *) registers->rcx;
    file_t * f = lookup_fd(fd_num);
    if (f == NULL || f->ops == NULL || f->ops->write == NULL) return -1;
    // El color lo aplica el write de la terminal usando el estilo del proceso
    // (get_current_style). Aca solo despachamos al fd (terminal o pipe).
    return f->ops->write(f, s, (int) strlen(s));
}

static int syscall_read(Registers * registers) {
    char * input = (char *) registers->rbx;
    file_t * f = lookup_fd(0);  // stdin (pipe por proceso)
    if (f == NULL || f->ops == NULL || f->ops->read == NULL) return -1;

    // Lectura de linea (modo canonico): leemos del pipe byte a byte hasta el
    // '\n' (que el hilo de terminal incluye al final de cada linea cocida) o
    // hasta llenar el buffer. El '\n' se descarta y dejamos el resto en el pipe
    // (type-ahead). Devolvemos la linea null-terminated, sin el '\n'.
    int n = 0;
    while (n < READ_BUFFER_MAX - 1) {
        char c;
        int r = f->ops->read(f, &c, 1);
        if (r <= 0) break;          // EOF (sin writers)
        if (c == '\n') break;       // fin de linea
        input[n++] = c;
    }
    input[n] = 0;
    return n;
}

static int syscall_set_foreground(Registers * registers) {
    pid_t target = (pid_t) registers->rbx;
    pid_t fg = get_foreground_pid();
    pid_t me = get_actual_pid();
    // Si hay un fg actual, solo el puede ceder. Si es 0, cualquiera.
    if (fg != 0 && me != fg) return -1;
    set_foreground_pid(target);
    return 0;
}

static int syscall_write_fd(Registers * registers) {
    int fd_num = (int) registers->rbx;
    const char * buf = (const char *) registers->rcx;
    int count = (int) registers->rdx;
    file_t * f = lookup_fd(fd_num);
    if (f == NULL || f->ops == NULL || f->ops->write == NULL) return -1;
    return f->ops->write(f, buf, count);
}

static int syscall_read_fd(Registers * registers) {
    int fd_num = (int) registers->rbx;
    char * buf = (char *) registers->rcx;
    int count = (int) registers->rdx;
    file_t * f = lookup_fd(fd_num);
    if (f == NULL || f->ops == NULL || f->ops->read == NULL) return -1;
    return f->ops->read(f, buf, count);
}

static int syscall_close_fd(Registers * registers) {
    int fd_num = (int) registers->rbx;
    if (fd_num < 0 || fd_num >= MAX_FDS) return -1;
    if (actual_index == (size_t)-1) return -1;
    file_t * f = processes[actual_index].fds[fd_num];
    if (f == NULL) return -1;
    processes[actual_index].fds[fd_num] = NULL;
    if (f->ops != NULL && f->ops->close != NULL) {
        return f->ops->close(f);
    }
    return 0;
}

static int syscall_play_sound(Registers *registers) {
    uint16_t frequency = (uint16_t)registers->rbx;
    uint16_t duration = (uint16_t)registers->rcx;
    play_sound(frequency, duration);
    return 0;
}

static int syscall_is_audio_buffer_empty(Registers *registers) {
    return isAudioBufferEmpty() ? 1 :0;
}

static int syscall_clear_audio_buffer(Registers *registers) {
    clearAudioBuffer();
    return 0;
}

static int syscall_clear(Registers * registers) {
    clearTextBuffer();  
    return 0;
}

static int syscall_graphics_mode(Registers * registers) {
    if (registers->rbx) {
        canvasMode();
    } else {
        textMode();
    }
    return 0;
}

typedef struct {
    uint64_t x1, y1, x2, y2;
    uint16_t thickness;
    uint32_t color;
} LineParameters;

typedef struct {
    uint64_t x1, y1, x2, y2;
    uint16_t thickness;
    uint32_t color;
} RectangleParameters;

typedef struct {
    uint64_t x1, y1, x2, y2;
    uint32_t color;
    bool directWrite;
} FilledRectangleParameters;

typedef struct {
    uint32_t color;
} FillScreenParameters;

typedef struct {
    uint64_t x, y;
    uint16_t radius;
    uint16_t thickness;
    uint32_t color;
} CircleParameters;

typedef struct {
    uint64_t x, y;
    uint16_t radius;
    uint32_t color;
} FilledCircleParameters;

typedef struct {
    uint64_t x, y;
    const char* text;
    uint16_t height;
    uint32_t color;
} TextParameters;

static int syscall_draw_line(Registers * registers) {
    LineParameters * params = (LineParameters *) registers->rbx;
    drawLine(params->x1, params->y1, params->x2, params->y2, params->thickness, params->color);
    return 0;
}

static int syscall_draw_rectangle(Registers * registers) {
    RectangleParameters * params = (RectangleParameters *) registers->rbx;
    drawRectangle(params->x1, params->y1, params->x2, params->y2, params->thickness, params->color);
    return 0;
}

static int syscall_draw_filled_rectangle(Registers * registers) {
    FilledRectangleParameters * params = (FilledRectangleParameters *) registers->rbx;
    drawFilledRectangle(params->x1, params->y1, params->x2, params->y2, params->color, params->directWrite);
    return 0;
}

static int syscall_draw_fill_screen(Registers * registers) {
    FillScreenParameters * params = (FillScreenParameters *) registers->rbx;
    fillScreen(params->color);
    return 0;
}

static int syscall_draw_circle(Registers * registers) {
    CircleParameters * params = (CircleParameters *) registers->rbx;
    drawCircle(params->x, params->y, params->radius, params->thickness, params->color);
    return 0;
}

static int syscall_draw_filled_circle(Registers * registers) {
    FilledCircleParameters * params = (FilledCircleParameters *) registers->rbx;
    drawFilledCircle(params->x, params->y, params->radius, params->color);
    return 0;
}

static int syscall_draw_text(Registers * registers) {
    TextParameters * params = (TextParameters *) registers->rbx;
    drawText(params->x, params->y, params->text, params->height, params->color);
    return 0;
}

static int syscall_clear_canvas(Registers * registers) {
    clearCanvas();
    return 0;
}

static int syscall_swap_buffers(Registers * registers) {
    swapBuffers();
    return 0;
}

static int syscall_time(Registers * registers){
    DateTime * datetime_buffer = (DateTime *) registers->rbx;
    
    if (datetime_buffer == 0) {
        return -1;  // Error
    }
    
    getTime(datetime_buffer);
    
    return 0;
}

static uint64_t syscall_ms(Registers * registers){
    return getMilisFromBoot();
}

static KeyEvent event;

// Modo RAW (juegos): no bloqueante. Saca un scancode crudo del anillo y lo
// decodifica en el acto. Devuelve 0 si no hay tecla pendiente.
static KeyEvent * syscall_get_key(Registers * registers) {
    uint8_t raw;
    if (!raw_pop(&raw)) return 0;
    event = decode_scancode(raw);
    return &event;
}

static int syscall_set_text_size(Registers * registers) {
    uint16_t height = (uint16_t)registers->rbx;
    setTextSize(height);
    return 0;
}

static int syscall_set_fps_overlay(Registers * registers) {
    setFpsOverlayEnabled(registers->rbx != 0);
    return 0;
}

static void * syscall_malloc(Registers * registers) {
    return malloc((size_t) registers->rbx);
}


static int syscall_free(Registers * registers) {
    free((void *) registers->rbx);
    return 0;
}

static uint64_t syscall_get_total_memory(Registers *registers) {
    return (uint64_t) getTotalMemory();
}

static uint64_t syscall_get_used_memory(Registers *registers) {
    return (uint64_t) getUsedMemory();
}

static int syscall_get_process_list(Registers *regs) {
    ProcessInfo *buffer = (ProcessInfo *)regs->rbx;
    int maxCount = (int)regs->rcx;
    return get_processesInfo(buffer, maxCount);
}