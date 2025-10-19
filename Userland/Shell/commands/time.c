#include <commands.h>
#include <stdint.h>
#include <inout.h>
#include "../lib/time.h"

int time() {
    uint8_t datetime[6];  
    getDateTime(datetime);
    
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