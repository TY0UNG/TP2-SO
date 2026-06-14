#ifndef utils_h
#define utils_h

#include <stdint.h>

// Generador pseudoaleatorio (catedra: MWC de George Marsaglia). Lo usan varios
// comandos (mvar, los tests, ...) para temporizaciones aleatorias.
//   GetUint()        -> entero crudo en [0, 2^32)
//   GetUniform(max)  -> entero uniforme en [0, max)
uint32_t GetUint(void);
uint32_t GetUniform(uint32_t max);

#endif
