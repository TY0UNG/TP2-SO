#include <commands.h>
#include <stdint.h>

extern void sys_getTime(uint8_t* buffer);

// Implementar printChar usando print
void printChar(char c) {
    char str[2] = {c, '\0'};
    print(str);
}

// Convertir un dígito hexadecimal (0-15) a char
char hex_to_char(uint8_t value) {
    if (value < 10)
        return '0' + value;
    else
        return 'A' + (value - 10);
}

// Imprimir un byte en hexadecimal (2 dígitos)
void printByteHex(uint8_t value) {
    printChar(hex_to_char((value >> 4) & 0x0F));
    printChar(hex_to_char(value & 0x0F));
}

int getTime() {
    uint8_t datetime[5];  // ← IMPORTANTE: uint8_t, no char
    sys_getTime(datetime);
    
    print("Fecha: ");
    printByteHex(datetime[1]);  // Mes
    printChar('/');
    printByteHex(datetime[0]);  // Año
    printChar('\n');
    
    print("Hora:  ");
    printByteHex(datetime[2]);  // Hora
    printChar(':');
    printByteHex(datetime[3]);  // Minutos
    printChar(':');
    printByteHex(datetime[4]);  // Segundos
    printChar('\n');
    
    return 0;
}