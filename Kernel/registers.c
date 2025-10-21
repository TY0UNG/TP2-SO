
#include <video.h>
#include <stdint.h>

typedef struct {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
    uint64_t rip;
    uint64_t rflags;
    uint16_t cs, ds, es, fs, gs, ss;
} RegisterDump;

void dump_registers() {
    RegisterDump regs;
    
    // Capturar registros
    __asm__ volatile (
        "movq %%rax, %0\n"
        "movq %%rbx, %1\n"
        "movq %%rcx, %2\n"
        "movq %%rdx, %3\n"
        "movq %%rsi, %4\n"
        "movq %%rdi, %5\n"
        "movq %%rbp, %6\n"
        "movq %%rsp, %7\n"
        : "=m"(regs.rax), "=m"(regs.rbx), "=m"(regs.rcx), "=m"(regs.rdx),
          "=m"(regs.rsi), "=m"(regs.rdi), "=m"(regs.rbp), "=m"(regs.rsp)
    );
    
    __asm__ volatile (
        "movq %%r8, %0\n"
        "movq %%r9, %1\n"
        "movq %%r10, %2\n"
        "movq %%r11, %3\n"
        "movq %%r12, %4\n"
        "movq %%r13, %5\n"
        "movq %%r14, %6\n"
        "movq %%r15, %7\n"
        : "=m"(regs.r8), "=m"(regs.r9), "=m"(regs.r10), "=m"(regs.r11),
          "=m"(regs.r12), "=m"(regs.r13), "=m"(regs.r14), "=m"(regs.r15)
    );
    
    __asm__ volatile (
        "movw %%cs, %0\n"
        "movw %%ds, %1\n"
        "movw %%es, %2\n"
        "movw %%fs, %3\n"
        "movw %%gs, %4\n"
        "movw %%ss, %5\n"
        : "=m"(regs.cs), "=m"(regs.ds), "=m"(regs.es),
          "=m"(regs.fs), "=m"(regs.gs), "=m"(regs.ss)
    );
    
    __asm__ volatile ("pushfq; popq %0" : "=r"(regs.rflags));
    __asm__ volatile ("lea (%%rip), %0" : "=r"(regs.rip));
    
    // Imprimir 
    print("\n========== REGISTER DUMP ==========\n");
    
    print(" RAX: "); printHex64(regs.rax);
    print("  RBX: "); printHex64(regs.rbx); print("\n");
    
    print(" RCX: "); printHex64(regs.rcx);
    print("  RDX: "); printHex64(regs.rdx); print("\n");
    
    print(" RSI: "); printHex64(regs.rsi);
    print("  RDI: "); printHex64(regs.rdi); print("\n");
    
    print(" RBP: "); printHex64(regs.rbp);
    print("  RSP: "); printHex64(regs.rsp); print("\n");
    
    print(" R8:  "); printHex64(regs.r8);
    print("  R9:  "); printHex64(regs.r9); print("\n");
    
    print(" R10: "); printHex64(regs.r10);
    print("  R11: "); printHex64(regs.r11); print("\n");
    
    print(" R12: "); printHex64(regs.r12);
    print("  R13: "); printHex64(regs.r13); print("\n");
    
    print(" R14: "); printHex64(regs.r14);
    print("  R15: "); printHex64(regs.r15); print("\n");
    
    print("RIP: "); printHex64(regs.rip); print("\n");
    print("RFLAGS: "); printHex64(regs.rflags); print("\n");
    
    print(" CS: "); printHex16(regs.cs);
    print("  DS: "); printHex16(regs.ds);
    print("  ES: "); printHex16(regs.es); print("\n");
    
    print(" FS: "); printHex16(regs.fs);
    print("  GS: "); printHex16(regs.gs);
    print("  SS: "); printHex16(regs.ss); print("\n");
    
    print("===================================\n");
}