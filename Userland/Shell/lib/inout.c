#include <inout.h>
#include <stdint.h>

extern int sys_write(int fd, const char* str);
extern size_t sys_read(const char* buffer);
extern void sys_clear();
extern uint64_t sys_get_key();

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

// Obtiene la tecla sin bloquear
// Retorna scancode (0 si no hay tecla)
// is_release se guarda en *is_release si no es NULL
uint8_t getKey(int* is_release) {
    uint64_t result = sys_get_key();
    if (result == 0) return 0;  // No hay tecla
    
    uint8_t scancode = result & 0xFF;
    uint8_t ascii = (result >> 8) & 0xFF;
    int release = (result >> 16) & 0x1;
    if (is_release != 0) {
        *is_release = release;
    }
    return scancode;
}