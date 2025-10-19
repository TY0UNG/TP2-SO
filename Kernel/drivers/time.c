#include "time.h"

extern uint64_t getMilis();
extern uint8_t getSeconds();
extern uint8_t getMinutes();
extern uint8_t getHour();
extern uint8_t getDayOfMonth();
extern uint8_t getMonth();
extern uint8_t getYear();

int ticksPerMs;

void calibrateMilis() {
    uint64_t initialTicks = getMilis();
    int timerTicks = ticks_elapsed();
    while(ticks_elapsed() - timerTicks < 5);
    ticksPerMs = (getMilis() - initialTicks) / ((ticks_elapsed() - timerTicks) * 52);
}

uint64_t getMilisFromBoot() {
    return getMilis() / ticksPerMs;
}

void getTime(uint8_t *time_buffer) {
    if (!time_buffer) return;
    time_buffer[0] = getYear();
    time_buffer[1] = getMonth();
    time_buffer[2] = getDayOfMonth();
    time_buffer[3] = getHour();
    time_buffer[4] = getMinutes();
    time_buffer[5] = getSeconds();
}

