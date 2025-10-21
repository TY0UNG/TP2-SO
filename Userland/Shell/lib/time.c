#include "time.h"

extern void sys_get_time(uint8_t* buffer);
extern uint64_t sys_get_ms();

void getDateTime(uint8_t* buffer) {
    sys_get_time(buffer);
}

uint64_t getMilisFromBoot() {
    return sys_get_ms();
}

void sleep(uint64_t ms) {
    uint64_t start = getMilisFromBoot();
    while (getMilisFromBoot() < start + ms);
}