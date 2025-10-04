#ifndef INOUT_H
#define INOUT_H

#include <stdlib.h>

#define STDOUT 1
#define STDERR 2

int print(const char* str, int fd);
size_t read(char* buffer);

#endif