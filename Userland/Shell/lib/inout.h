#ifndef INOUT_H
#define INOUT_H

#include <stdlib.h>
#include <stdint.h>

#define STDOUT 1
#define STDERR 2

int print(const char* str);
int println(const char* str);
int printerr(const char* str);
size_t read(char* buffer);
void clear();

void printChar(char c);
void printHex(uint8_t value);
void printDec(uint64_t number);

uint8_t getKey(int* is_release);

#endif