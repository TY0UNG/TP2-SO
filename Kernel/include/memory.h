#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdbool.h>

void initializeMemoryManager(void * memoryStart, size_t memorySize);
void * malloc(size_t size);
void free(void * ptr);

size_t getTotalMemory();
size_t getUsedMemory();

#endif