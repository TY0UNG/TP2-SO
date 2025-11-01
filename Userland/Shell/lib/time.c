#include "time.h"

extern void sys_get_time(DateTime * buffer);
extern uint64_t sys_get_ms();

void getDateTime(DateTime * buffer) {
    sys_get_time(buffer);
}

uint64_t getMilisFromBoot() {
    return sys_get_ms();
}

void sleep(uint64_t ms) {
    uint64_t start = getMilisFromBoot();
    while (getMilisFromBoot() < start + ms);
}

void printTime(DateTime * datetime){

    print("Fecha: ");
    printHex(datetime->daymonth);  // Dia
    printChar('/');
    printHex(datetime->month);  // Mes
    printChar('/');
    printHex(datetime->year);  // Año
    printChar('\n');
    
    print("Hora:  ");
    printHex(datetime->hour);  // Hora
    printChar(':');
    printHex(datetime->minutes);  // Minutos
    printChar(':');
    printHex(datetime->seconds);  // Segundos
    printChar('\n');

    return;
}
