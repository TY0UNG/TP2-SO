#ifndef TERMINAL_H
#define TERMINAL_H

#include <files.h>
#include <processes.h>
#include <stdint.h>
#include <stdbool.h>

// Inicializa el estado interno de la terminal y crea el file_t global.
// Llamar una sola vez al arrancar el kernel.
void initialize_terminal();

// file_t global de la terminal del sistema (read = stdin, write = stdout).
file_t * get_terminal_fd();

// Crea un nuevo file_t que apunta al mismo estado (terminal singleton).
// Cada llamada incrementa el ref_count del backing terminal_t.
file_t * create_terminal_fd();

// Foreground process: el unico pid al que read_terminal_fd le devuelve datos.
// Otros se bloquean hasta ser foreground.
pid_t get_foreground_pid();
void set_foreground_pid(pid_t pid);

// API de texto/render que antes vivia en video.h
void print(const char *text);
void printChar(char c);
void deleteChar();
void selectStyle(char style);
void setTextSize(uint16_t height);
void textMode();
void clearTextBuffer();
void scrollUp();
void scrollDown();

void printHex16(uint16_t value);
void printHex32(uint32_t value);
void printHex64(uint64_t value);
void printDec(uint64_t value);

#endif
