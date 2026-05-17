#ifndef keyboard_h
#define keyboard_h
#include <stdbool.h>
#include <stdint.h>

#define SCANCODE_LSHIFT 0x2A
#define SCANCODE_RSHIFT 0x36

typedef struct key_event {
    uint8_t scancode;
    uint8_t ascii;
    bool is_release;
    bool printable;
} KeyEvent;

void keyboard_handler();

bool isKeyBufferEmpty();

KeyEvent getNextKey();

void clearKeyBuffer();

void keyboard_set_enabled(bool enabled);
bool keyboard_is_enabled();

#endif