#ifndef MEMORY_H
#define MEMORY_H

    #include <stdlib.h>

    void * malloc(size_t size);
    void free(void * ptr);
    size_t getTotalMemory(void);
    size_t getUsedMemory(void);

#endif