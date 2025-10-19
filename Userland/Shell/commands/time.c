#include <commands.h>
#include <stdint.h>
#include <inout.h>

extern void sys_get_time(uint8_t* buffer);
extern uint64_t sys_get_ms();

int getTime() {
    uint8_t datetime[6];  
    sys_get_time(datetime);
    
    print("Fecha: ");
    printHex(datetime[2]);  // Dia
    printChar('/');
    printHex(datetime[1]);  // Mes
    printChar('/');
    printHex(datetime[0]);  // Año
    printChar('\n');
    
    print("Hora:  ");
    printHex((datetime[3] - 3));  // Hora
    printChar(':');
    printHex(datetime[4]);  // Minutos
    printChar(':');
    printHex(datetime[5]);  // Segundos
    printChar('\n');

    print("MS: ");
    printDec(sys_get_ms());
    printChar('\n');
    
    return 0;
}