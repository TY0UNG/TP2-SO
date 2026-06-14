#ifndef str_h
#define str_h
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

int strcmp(const char* str1, const char* str2);
int strlen(const char* str);
void strcpy(char* dest, const char* src);
void strcat(char* dest, const char* src);
char * strchr(const char *s, char c);
int strparse(char * input, char ** output, char * splitter);

char *parseInt(int n, char *buffer, uint64_t max);

struct file;

typedef struct {
    int (*read)(struct file *f, char *buf, int count);
    int (*write)(struct file *f, const char *buf, int count);
    int (*close)(struct file *f);
} file_ops_t;

typedef struct file {
    file_ops_t *ops;  // Comportamiento dinámico
    void *data; // Datos específicos (el pipe_t, el tty_t, etc.)
    int ref_count;      // Cuántos procesos lo están usando
} file_t;

#endif