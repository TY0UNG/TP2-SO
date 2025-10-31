#include <inout.h>
#include <stdint.h>

extern int sys_write(int fd, const char* str);
extern size_t sys_read(const char* buffer);
extern void sys_clear();
extern KeyEvent * sys_get_key();
extern void sys_set_text_size(uint16_t height);

int print(const char* str) {
    return sys_write(STDOUT, str);
}

int println(const char* str) {
    return print(str) && print("\n");
}

int printerr(const char* str) {
    return sys_write(STDERR, str);
}

size_t read(char* buffer) {
    return sys_read(buffer);
}

void clear() {  
    sys_clear();
}

void printChar(char c) {
    char str[2] = {c, '\0'};
    print(str);
}

char hex_to_char(uint8_t value) {
    if (value < 10)
        return '0' + value;
    else
        return 'A' + (value - 10);
}
/*
void printFullHex(uint64_t value) {
    for (int shift = 60; shift >= 0; shift -= 4) {
        uint8_t nibble = (value >> shift) & 0x0F;
        printChar(hex_to_char(nibble));
    }
}*/

void printHex(uint64_t value) {
    int flag = 0; 
    for (int shift = 60; shift >= 0; shift -= 4) {
        uint8_t nibble = (value >> shift) & 0x0F;
        if (nibble != 0 || flag) {
            printChar(hex_to_char(nibble));
            flag = 1;
        }
    }
    if (!flag)  
        printChar('0');
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

KeyEvent * getKey() {
    return sys_get_key();
}

void setTextSize(uint16_t height) {
    sys_set_text_size(height);
}