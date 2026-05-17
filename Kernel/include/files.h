#ifndef FILES_H
#define FILES_H

struct file;

// Estructura de punteros a funciones (Operaciones)
typedef struct {
    int (*read)(struct file *f, char *buf, int count);
    int (*write)(struct file *f, const char *buf, int count);
    int (*close)(struct file *f);
} file_ops_t;

// La estructura UNIFICADA de archivo
typedef struct file {
    file_ops_t *ops;  // Comportamiento dinámico
    void *data; // Datos específicos (el pipe_t, el tty_t, etc.)
    int ref_count;      // Cuántos procesos lo están usando
} file_t;

#endif
