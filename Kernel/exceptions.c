#include <video.h>
#include <interrupts.h>

#define ZERO_EXCEPTION_ID 0
#define INVALID_OPCODE_ID 6

extern void * getShellAddress();
extern uint64_t getShellRSP();

typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rsi, rdi, rbp, rdx, rcx, rbx, rax;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} ExceptionStackFrame;

static void zero_division(ExceptionStackFrame *frame);
static void invalid_opcode(ExceptionStackFrame *frame);
static void print_exception_info(const char *exception_name, ExceptionStackFrame *frame);

void exceptionDispatcher(ExceptionStackFrame *frame, int exception) {
    if (exception == ZERO_EXCEPTION_ID) {
        zero_division(frame);
    } else if (exception == INVALID_OPCODE_ID) {
        invalid_opcode(frame);
    }
    frame->rip = (uint64_t)getShellAddress();
    frame->rsp = getShellRSP();
}

static void print_exception_info(const char *exception_name, ExceptionStackFrame *frame) {
    selectStyle(0x04); // Rojo para error
    print("\n");
    print("================== EXCEPTION OCCURRED ==================\n");
    print("Exception: ");
    print(exception_name);
    print("\n");
    print("========================================================\n");
    
    // selectStyle(0x0F); // Blanco normal
    
    print("  RIP: ");
    printHex64(frame->rip);
    print("\n");
    
    // La CPU hace push de 5 valores (SS, RSP, RFLAGS, CS, RIP) = 40 bytes
    print("  RSP: ");
    printHex64(frame->rsp);
    print("\n");
    
    print("  RAX: ");
    printHex64(frame->rax);
    print("  RBX: ");
    printHex64(frame->rbx);
    print("\n");
    print("  RCX: ");
    printHex64(frame->rcx);
    print("  RDX: ");
    printHex64(frame->rdx);
    print("\n");
    print("  RSI: ");
    printHex64(frame->rsi);
    print("  RDI: ");
    printHex64(frame->rdi);
    print("\n");
    print("  RBP: ");
    printHex64(frame->rbp);
    print("\n");
    print("  R8 : ");
    printHex64(frame->r8);
    print("  R9 : ");
    printHex64(frame->r9);
    print("\n");
    print("  R10: ");
    printHex64(frame->r10);
    print("  R11: ");
    printHex64(frame->r11);
    print("\n");
    print("  R12: ");
    printHex64(frame->r12);
    print("  R13: ");
    printHex64(frame->r13);
    print("\n");
    print("  R14: ");
    printHex64(frame->r14);
    print("  R15: ");
    printHex64(frame->r15);
    print("\n");
    
    // Flags
    print("  RFLAGS: ");
    printHex64(frame->rflags);
    print("\n");
    
    // Segmentos
    print("  CS: ");
    printHex16((uint16_t)frame->cs);
    print("  SS: ");
    printHex16((uint16_t)frame->ss);
    print("\n");
    
    selectStyle(0x0E); // Amarillo -> para advertencia
    print("========================================================\n");
    print("Returning to shell...\n");
    print("========================================================\n");
    selectStyle(0x0F);
}

static void zero_division(ExceptionStackFrame *frame) {
    print_exception_info("Division by Zero", frame);
}

static void invalid_opcode(ExceptionStackFrame *frame) {
    print_exception_info("Invalid Opcode", frame);
}