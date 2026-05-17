#ifndef LIB_H
#define LIB_H

#include <stdlib.h>
#include <stdint.h>

typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rsi, rdi, rbp, rdx, rcx, rbx, rax;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} StackFrame;

void * memset(void * destination, int32_t character, uint64_t length);
void * memcpy(void * destination, const void * source, uint64_t length);

char *cpuVendor(char *result);

uint32_t uintToBase(uint64_t value, char * buffer, uint32_t base);

char * strcpy(char* dest, const char* src);
size_t strlen(const char* str);

#endif