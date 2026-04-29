#include <memory.h>
#include <stdbool.h>

bool initialized = false;
char *baseAddress;
char *nextAddress;
char *finalAddress;

void initializeMemoryManager(void * memoryStart, size_t memorySize) {
    baseAddress = memoryStart;
    nextAddress = memoryStart;
    finalAddress = memoryStart + memorySize - 1;
    initialized = true;
}

void * malloc(size_t size) {
    if (!initialized || (size_t)(finalAddress - nextAddress) < size) {
        return 0;
    }

    char *allocation = nextAddress;
	nextAddress += size;
	return (void *) allocation;
}

void free(void * ptr) {
    if (!initialized) return;
    // Este memory manager no soporta free, no hace nada
}

size_t getTotalMemory() {
    return finalAddress - baseAddress;
}

size_t getUsedMemory() {
    return nextAddress - baseAddress;
}