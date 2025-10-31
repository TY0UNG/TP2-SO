#ifndef INOUT_H
#define INOUT_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define STDOUT 1
#define STDERR 2

int print(const char* str);
int println(const char* str);
int printerr(const char* str);
size_t read(char* buffer);
void clear();

void printChar(char c);
void printHex(uint8_t value);
void printDec(uint64_t number);
void setTextSize(uint16_t height);

typedef struct key_event {
    uint8_t scancode;
    uint8_t ascii;
    bool is_release;
    bool printable;
} KeyEvent;

KeyEvent * getKey();

#endif