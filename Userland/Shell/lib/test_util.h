#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <stdint.h>

// Utilidades compartidas por los tests de la catedra (test_mm, test_sync,
// test_processes, test_prio, ...). Implementaciones identicas a las de la
// catedra salvo endless_loop_print, adaptado a la API de salida de este SO.

uint32_t GetUint(void);
uint32_t GetUniform(uint32_t max);

uint8_t memcheck(void *start, uint8_t value, uint32_t size);

int64_t satoi(char *str);

void bussy_wait(uint64_t n);
void endless_loop(void);
void endless_loop_print(uint64_t wait);

#endif
