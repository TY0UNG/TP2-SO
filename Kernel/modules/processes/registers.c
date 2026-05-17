#include <video.h>
#include <stdint.h>
#include <time.h>

typedef struct RegisterDump {
    uint64_t rax;      // Offset 0
    uint64_t rbx;      // Offset 8
    uint64_t rcx;      // Offset 16
    uint64_t rdx;      // Offset 24
    uint64_t rbp;      // Offset 32
    uint64_t rdi;      // Offset 40
    uint64_t rsi;      // Offset 48
    uint64_t r8;       // Offset 56
    uint64_t r9;       // Offset 64
    uint64_t r10;      // Offset 72
    uint64_t r11;      // Offset 80
    uint64_t r12;      // Offset 88
    uint64_t r13;      // Offset 96
    uint64_t r14;      // Offset 104
    uint64_t r15;      // Offset 112

    uint64_t rip;      // Offset 120
    uint64_t rflags;   // Offset 128
    uint64_t rsp;      // Offset 136
    uint16_t cs;       // Offset 144
    uint16_t ss;       // Offset 148

    uint16_t ds;       // Offset 152
    uint16_t es;       // Offset 156
    uint16_t fs;       // Offset 160
    uint16_t gs;       // Offset 164
    
    uint64_t int_no;   // Número de la interrupción
    uint64_t err_code; // Código de error

    DateTime time;
} __attribute__((packed)) RegisterDump;

static RegisterDump regs;

extern void addregs(RegisterDump* regs_ptr);

#define MAX_DUMP_SIZE 2048   // tamaño máximo del texto 

static char dump_buffer[MAX_DUMP_SIZE];
static int dump_index = 0;

static void appendChar(char c);

void printHexToBuffer(uint8_t value) {
    const char *hex = "0123456789ABCDEF";
    appendChar(hex[(value >> 4) & 0xF]);
    appendChar(hex[value & 0xF]);
}

void printHex16ToBuffer(uint16_t value) {
    printHexToBuffer(value >> 8);
    printHexToBuffer(value & 0xFF);
}

void printHex64ToBuffer(uint64_t value) {
    for (int i = 60; i >= 0; i -= 4)
        appendChar("0123456789ABCDEF"[(value >> i) & 0xF]);
}

// agrega una cadena al buffer
static void append(const char *text) {
    while (*text && dump_index < MAX_DUMP_SIZE - 1)
        dump_buffer[dump_index++] = *text++;
    dump_buffer[dump_index] = '\0'; // null-terminate
}

// agrega un solo carácter
static void appendChar(char c) {
    if (dump_index < MAX_DUMP_SIZE - 1)
        dump_buffer[dump_index++] = c;
    dump_buffer[dump_index] = '\0';
}

void printStruct() {
    dump_index = 0;

    append("\n========== REGISTER DUMP ==========\n");
    
    append(" RAX: "); printHex64ToBuffer(regs.rax);
    append("  RBX: "); printHex64ToBuffer(regs.rbx); append("\n");
    
    append(" RCX: "); printHex64ToBuffer(regs.rcx);
    append("  RDX: "); printHex64ToBuffer(regs.rdx); append("\n");
    
    append(" RSI: "); printHex64ToBuffer(regs.rsi);
    append("  RDI: "); printHex64ToBuffer(regs.rdi); append("\n");
    
    append(" RBP: "); printHex64ToBuffer(regs.rbp);
    append("  RSP: "); printHex64ToBuffer(regs.rsp); append("\n");
    
    append(" R8:  "); printHex64ToBuffer(regs.r8);
    append("  R9:  "); printHex64ToBuffer(regs.r9); append("\n");
    
    append(" R10: "); printHex64ToBuffer(regs.r10);
    append("  R11: "); printHex64ToBuffer(regs.r11); append("\n");
    
    append(" R12: "); printHex64ToBuffer(regs.r12);
    append("  R13: "); printHex64ToBuffer(regs.r13); append("\n");
    
    append(" R14: "); printHex64ToBuffer(regs.r14);
    append("  R15: "); printHex64ToBuffer(regs.r15); append("\n");
    
    append(" RIP: "); printHex64ToBuffer(regs.rip); append("\n");
    append(" RFLAGS: "); printHex64ToBuffer(regs.rflags); append("\n");
    
    append(" CS: "); printHex16ToBuffer(regs.cs);
    append("  DS: "); printHex16ToBuffer(regs.ds);
    append("  ES: "); printHex16ToBuffer(regs.es); append("\n");
    
    append(" FS: "); printHex16ToBuffer(regs.fs);
    append("  GS: "); printHex16ToBuffer(regs.gs);
    append("  SS: "); printHex16ToBuffer(regs.ss); append("\n");
    
    append("===================================\n");
}

void dump_registers() {
    addregs(&regs);
    printStruct();
}

const char * get_register_dump() {
    return dump_buffer;
}