#include <stdint.h>
#include "test_util.h"
#include "inout.h"

extern uint64_t sys_getpid(void);

// El RNG (GetUint/GetUniform) se movio a utils.c por ser de uso general.

// Memory
uint8_t memcheck(void *start, uint8_t value, uint32_t size) {
    uint8_t *p = (uint8_t *) start;
    uint32_t i;

    for (i = 0; i < size; i++, p++)
        if (*p != value)
            return 0;

    return 1;
}

// Parameters
int64_t satoi(char *str) {
    uint64_t i = 0;
    int64_t res = 0;
    int8_t sign = 1;

    if (!str)
        return 0;

    if (str[i] == '-') {
        i++;
        sign = -1;
    }

    for (; str[i] != '\0'; ++i) {
        if (str[i] < '0' || str[i] > '9')
            return 0;
        res = res * 10 + str[i] - '0';
    }

    return res * sign;
}

// Dummies
void bussy_wait(uint64_t n) {
    uint64_t i;
    for (i = 0; i < n; i++)
        ;
}

void endless_loop(void) {
    while (1)
        ;
}

// Catedra: printf("%d ", pid). Este SO no tiene printf -> usamos printDec.
void endless_loop_print(uint64_t wait) {
    int64_t pid = (int64_t) sys_getpid();

    while (1) {
        printDec((uint64_t) pid);
        print(" ");
        bussy_wait(wait);
    }
}
