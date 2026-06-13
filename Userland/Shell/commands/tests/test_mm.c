#include "commands.h"
#include "../lib/memory.h"
#include <stdint.h>

#define MAX_BLOCKS 128

typedef struct MM_rq {
    void *address;
    uint32_t size;
} mm_rq;

static uint64_t satoi(const char *str) {
    if (str == 0 || *str == '\0') {
        return 0;
    }

    uint64_t value = 0;
    while (*str >= '0' && *str <= '9') {
        value = value * 10 + (uint64_t)(*str - '0');
        str++;
    }

    return value;
}

static uint32_t rng_state = 0x12345678;

static uint32_t GetUniform(uint32_t max) {
    if (max == 0) {
        return 0;
    }

    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state % max;
}

static int memcheck(void *address, uint8_t value, uint32_t size) {
    uint8_t *ptr = (uint8_t *)address;
    for (uint32_t i = 0; i < size; i++) {
        if (ptr[i] != value) {
            return 0;
        }
    }
    return 1;
}

static void memfill(void *address, uint8_t value, uint32_t size) {
    uint8_t *ptr = (uint8_t *)address;
    for (uint32_t i = 0; i < size; i++) {
        ptr[i] = value;
    }
}

uint64_t test_mm(uint64_t argc, char *argv[]) {

    mm_rq mm_rqs[MAX_BLOCKS];
    uint8_t rq;
    uint32_t total;
    uint64_t max_memory;

    if (argc != 1) {
        println("Uso: testmm <bytes>");
        return (uint64_t)-1;
    }

    max_memory = satoi(argv[0]);
    if (max_memory == 0) {
        println("testmm: el parametro debe ser un entero positivo");
        return (uint64_t)-1;
    }

    println("test_mm: corriendo (Ctrl+C para terminar)");

    while (1) {
        rq = 0;
        total = 0;

        while (rq < MAX_BLOCKS && total < max_memory) {
            mm_rqs[rq].size = GetUniform(max_memory - total - 1) + 1;
            mm_rqs[rq].address = malloc(mm_rqs[rq].size);

            if (mm_rqs[rq].address) {
                total += mm_rqs[rq].size;
                rq++;
            }
        }

        for (uint32_t i = 0; i < rq; i++)
            if (mm_rqs[i].address)
                memfill(mm_rqs[i].address, (uint8_t)i, mm_rqs[i].size);

        for (uint32_t i = 0; i < rq; i++)
            if (mm_rqs[i].address)
                if (!memcheck(mm_rqs[i].address, (uint8_t)i, mm_rqs[i].size)) {
                    println("test_mm ERROR");
                    return (uint64_t)-1;
                }

        for (uint32_t i = 0; i < rq; i++)
            if (mm_rqs[i].address)
                free(mm_rqs[i].address);
    }
}

int testmm(char **argv, int argc) {
    test_mm((uint64_t)(argc - 1), argv + 1);
    return 0;
}
