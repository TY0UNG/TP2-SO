#ifndef PIPES_H
#define PIPES_H

#include <files.h>

// Crea un pipe anonimo. Devuelve 0 en exito, -1 si no hay memoria.
// *read_end queda con un file_t solo-lectura y *write_end solo-escritura.
int create_pipe(file_t **read_end, file_t **write_end);

#endif
