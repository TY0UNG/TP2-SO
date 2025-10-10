#include <commands.h>

// para probar la excepcion de division por cero
int dividezero() {
    print("Dividiendo por cero...\n");
    volatile int x = 1 / 0; // Genera una excepción de división por cero
    return x; // Esta línea nunca se alcanzará
}

// para probar la excepcion de invalid opcode
int invalidop() {
    __asm__ volatile("ud2");  // instrucción indefinida que dispara #UD
    return 0;
}