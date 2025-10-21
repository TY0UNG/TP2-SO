#include <keyboard.h>
#define BUFFER_LENGHT 255

extern char get_keyboard_output();

static bool isPrintable(unsigned char scancode);



//volatile bool dump_registers_requested = false;

KeyEvent buffer[BUFFER_LENGHT];
uint8_t size = 0;
uint8_t head = 0;
uint8_t tail = 0;

bool isShiftPressed = false;

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

bool isFull() {
    return size == BUFFER_LENGHT;
}

// isEmpty
bool isKeyBufferEmpty() {
    return size == 0;
}

void clearKeyBuffer() {
    size = 0;
    head = 0;
    tail = 0;
}

void queue(KeyEvent event) {
    if (isFull()) getNextKey(); 
    buffer[head] = event;
    head = (head+1)%BUFFER_LENGHT;
    size++;
}

// dequeue
KeyEvent getNextKey() {
    KeyEvent event = buffer[tail];
    tail = (tail+1)%BUFFER_LENGHT;
    size--;
    return event;
}

void keyboard_handler() {
    uint8_t raw_scancode = get_keyboard_output();
    int is_release = raw_scancode & 0x80;
    uint8_t scancode = raw_scancode & 0x7F;
    if (scancode == SCANCODE_LSHIFT || scancode == SCANCODE_RSHIFT) 
        isShiftPressed = !is_release;
    char ascii = isShiftPressed ? scancode_to_ascii_shift[scancode] : scancode_to_ascii[scancode];
    KeyEvent event = {
        raw_scancode,
        ascii,
        is_release,
        isPrintable(scancode)
    };
    queue(event);
}

bool isPrintable(unsigned char scancode) {
    scancode &= 0x7F;
    char c = (scancode < sizeof(scancode_to_ascii)) ? scancode_to_ascii[scancode] : 0;
    return (c >= 32 && c < 127);
}