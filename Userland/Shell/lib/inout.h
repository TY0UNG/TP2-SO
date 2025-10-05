#ifndef INOUT_H
#define INOUT_H

#include <stdlib.h>

#define STDOUT 1
#define STDERR 2

int print(const char* str);
int println(const char* str);
int printerr(const char* str);
size_t read(char* buffer);
void clear();

#endif