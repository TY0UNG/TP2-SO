#include "time.h"

extern uint64_t getMilis();
extern uint8_t getSeconds();
extern uint8_t getMinutes();
extern uint8_t getHour();
extern uint8_t getDayOfMonth();
extern uint8_t getMonth();
extern uint8_t getYear();

double ticksPerMs;

void calibrateMilis() {
    uint8_t seconds = getSeconds(), aux;
    while (seconds == (aux = getSeconds()));
    uint64_t initialTicks = getMilis();
    while(aux == getSeconds());
    ticksPerMs = (double) (getMilis() - initialTicks) / 1000;
}

uint64_t getMilisFromBoot() {
    return getMilis() / ticksPerMs;
}

void getTime(DateTime *output) {
    if (!output) return;
    output->year = getYear();
    output->month = getMonth();
    output->daymonth = getDayOfMonth();
    output->hour = getHour();
    output->minutes = getMinutes();
    output->seconds = getSeconds();
}

