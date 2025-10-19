#include <inout.h>
#include <stdint.h>

extern int sys_write(int fd, const char* str);
extern size_t sys_read(const char* buffer);
extern void sys_clear();

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

void printHex(uint8_t value) {
    printChar(hex_to_char((value >> 4) & 0x0F));
    printChar(hex_to_char(value & 0x0F));
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