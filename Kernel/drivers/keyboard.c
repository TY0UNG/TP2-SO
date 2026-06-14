#include <keyboard.h>
#include <semaphores.h>
#include <processes.h>
#include <stddef.h>

#define BUFFER_LENGHT 256   // potencia de 2: indices uint8_t envuelven solos
#define RAW_RING      256

extern uint8_t get_keyboard_output(void);

static bool isPrintable(unsigned char scancode);

void dump_registers();

// ============================================================================
//   Anillo CRUDO (scancodes): productor = ISR, consumidor = hilo de terminal.
//   SPSC lock-free: el productor toca solo raw_head, el consumidor solo raw_tail.
//   No hay contador compartido -> sin data race (vacio: head==tail).
// ============================================================================
static volatile uint8_t raw_buf[RAW_RING];
static volatile uint8_t raw_head = 0;
static volatile uint8_t raw_tail = 0;

// Devuelve false si el anillo esta lleno (se dropea el byte).
static bool raw_push(uint8_t b) {
    uint8_t next = (uint8_t)(raw_head + 1);
    if (next == raw_tail) return false;   // lleno
    raw_buf[raw_head] = b;
    raw_head = next;
    return true;
}

bool raw_pop(uint8_t * out) {
    if (raw_head == raw_tail) return false;  // vacio
    *out = raw_buf[raw_tail];
    raw_tail = (uint8_t)(raw_tail + 1);
    return true;
}

static KeyEvent buffer[BUFFER_LENGHT];
static uint8_t head = 0;
static uint8_t tail = 0;

static bool isShiftPressed = false;
static bool isCtrlPressed = false;
static bool keyboard_enabled = true;

static const char scancode_to_ascii[] = {
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0,  ' '
};

static const char scancode_to_ascii_shift[] = {
    0,   27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0,  ' '
};

static bool isFull(void) {
    return (uint8_t)(head + 1) == tail;
}

// isEmpty
bool isKeyBufferEmpty() {
    return head == tail;
}

void clearKeyBuffer() {
    head = 0;
    tail = 0;
    raw_head = 0;
    raw_tail = 0;
    isShiftPressed = false;
}

void keyboard_set_enabled(bool enabled) {
    keyboard_enabled = enabled;
    if (!keyboard_enabled) {
        clearKeyBuffer();
    }
}

bool keyboard_is_enabled() {
    return keyboard_enabled;
}

// Encola un KeyEvent decodificado. Lo llama SOLO el hilo de terminal (productor
// unico). En lleno dropea el evento nuevo (no toca tail, que es del consumidor).
void queue(KeyEvent event) {
    if (isFull()) return;
    buffer[head] = event;
    head = (uint8_t)(head + 1);
}

KeyEvent null = {
    0, 0, 0, 0
};

// dequeue
KeyEvent getNextKey() {
    if (isKeyBufferEmpty()) return null;
    KeyEvent event = buffer[tail];
    tail = (uint8_t)(tail + 1);
    return event;
}

// Cola de procesos esperando teclado. La acotamos a PROCESSES_LIMIT: como cada
// proceso se encola a lo sumo una vez (dedup), nunca puede desbordar y ningun
// lector queda bloqueado fuera de la lista de wake.
#define KBD_MAX_WAITERS PROCESSES_LIMIT
static pid_t kbd_waiters[KBD_MAX_WAITERS];
static size_t kbd_waiter_count = 0;

void kbd_block_self(void) {
    pid_t me = get_actual_pid();
    bool already = false;
    for (size_t i = 0; i < kbd_waiter_count; i++)
        if (kbd_waiters[i] == me) { already = true; break; }
    if (!already && kbd_waiter_count < KBD_MAX_WAITERS)
        kbd_waiters[kbd_waiter_count++] = me;
    block_current(WAIT_KBD);
}

void kbd_wake(void) {
    size_t n = kbd_waiter_count;
    kbd_waiter_count = 0;
    for (size_t i = 0; i < n; i++)
        unblock_process(kbd_waiters[i]);
}

// La ISR hace lo minimo posible.
void keyboard_handler() {
    uint8_t raw = get_keyboard_output();
    if (!keyboard_enabled) {
        return;
    }
    if (raw_push(raw)) {
        kbd_wake();
    }
}

KeyEvent decode_scancode(uint8_t raw_scancode) {
    int is_release = raw_scancode & 0x80;
    uint8_t scancode = raw_scancode & 0x7F;
    if (scancode == SCANCODE_LSHIFT || scancode == SCANCODE_RSHIFT)
        isShiftPressed = !is_release;
    if (scancode == SCANCODE_LCTRL)
        isCtrlPressed = !is_release;
    char ascii = 0;
    if (scancode < sizeof(scancode_to_ascii)) {
        ascii = isShiftPressed ? scancode_to_ascii_shift[scancode] : scancode_to_ascii[scancode];
    }
    if (isCtrlPressed && ascii >= 'a' && ascii <= 'z')  // ctrl+(letra)
        ascii = ascii - 'a' + 1;
    KeyEvent event = {
        raw_scancode,
        ascii,
        is_release,
        isPrintable(scancode)
    };

    // Hotkey de debug: shift + tecla 0x29 (`) dumpea registros.
    if (!is_release && scancode == 0x29 && isShiftPressed) {
        dump_registers();
    }
    return event;
}

bool isPrintable(unsigned char scancode) {
    scancode &= 0x7F;
    char c = (scancode < sizeof(scancode_to_ascii)) ? scancode_to_ascii[scancode] : 0;
    return (c >= 32 && c < 127);
}
