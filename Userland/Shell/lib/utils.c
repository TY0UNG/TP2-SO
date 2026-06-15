#include <stdint.h>
#include "utils.h"

// Random (catedra: George Marsaglia MWC). El estado es global al modulo: como
// todos los procesos comparten la imagen del shell, comparten el generador.
static uint32_t m_z = 362436069;
static uint32_t m_w = 521288629;

uint32_t GetUint(void) {
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;
}

// Entero puro: sin punto flotante. El kernel no preserva el estado de la FPU/x87
// en el cambio de contexto, asi que usar 'double' aca corrompe a los procesos
// concurrentes (mvar, testsync, testprocesses lanzan varios a la vez).
uint32_t GetUniform(uint32_t max) {
    if (max == 0) return 0;
    return GetUint() % max;
}
