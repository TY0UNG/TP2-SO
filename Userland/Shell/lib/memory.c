#include "./memory.h"

extern void * sys_malloc(size_t size);
extern void sys_free(void * ptr);
extern size_t sys_get_total_memory(void);
extern size_t sys_get_used_memory(void);

void * malloc(size_t size) {
    return sys_malloc(size);
}

void free(void * ptr) {
    return sys_free(ptr);
}

size_t getTotalMemory(void) {
    return sys_get_total_memory();
}

size_t getUsedMemory(void) {
    return sys_get_used_memory();
}
