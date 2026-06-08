#ifndef STDIO_H
#define STDIO_H

// printf minimo para los tests de la catedra. Soporta: %d %u %x %c %s %%.
// (No hay libc; escribe por stdout via las primitivas de inout.)
int printf(const char *fmt, ...);

#endif
