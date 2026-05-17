#include <video.h>
#include <terminal.h>
#include <interrupts.h>
#include <processes.h>

#define ZERO_EXCEPTION_ID 0
#define INVALID_OPCODE_ID 6

static void zero_division(StackFrame *frame);
static void invalid_opcode(StackFrame *frame);
static void print_exception_info(const char *exception_name, StackFrame *frame);

void exceptionDispatcher(StackFrame *frame, int exception) {
    if (exception == ZERO_EXCEPTION_ID) {
        zero_division(frame);
    } else if (exception == INVALID_OPCODE_ID) {
        invalid_opcode(frame);
    }
    kill_process(get_actual_pid());
    scheduler();
}

static void print_exception_info(const char *exception_name, StackFrame *frame) {
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
    print("Killing actual process...\n");
    print("========================================================\n");
    selectStyle(0x0F);
}

static void zero_division(StackFrame *frame) {
    print_exception_info("Division by Zero", frame);
}

static void invalid_opcode(StackFrame *frame) {
    print_exception_info("Invalid Opcode", frame);
}