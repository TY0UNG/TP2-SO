#ifndef INOUT_H
#define INOUT_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define STDIN  0
#define STDOUT 1
#define STDERR 2

// Modo de la terminal (estilo termios).
#define TERM_COOKED 0   // linea con echo (read devuelve lineas) — default
#define TERM_RAW    1   // teclas crudas via getKey(), sin echo (juegos)

int print(const char* str);
// Fija el color de escritura del proceso (pegajoso). El byte 'style' codifica FG
// en el nibble bajo y BG en el alto (paleta de 16). Ej: 0x0C = rojo claro sobre
// negro. Afecta a print/println/printChar/write_fd siguientes de este proceso.
void selectStyle(char style);
int println(const char* str);
int printerr(const char* str);
size_t read(char* buffer);
void clear();

int write_fd(int fd, const char *buf, int count);
int read_fd(int fd, char *buf, int count);
int close_fd(int fd);

int set_foreground(int pid);
void set_terminal_mode(int mode);

void printChar(char c);
void printHex(uint64_t value);

void printDec(uint64_t number);
void setTextSize(uint16_t height);

typedef struct key_event {
    uint8_t scancode;
    uint8_t ascii;
    bool is_release;
    bool printable;
} KeyEvent;

KeyEvent * getKey();

#endif