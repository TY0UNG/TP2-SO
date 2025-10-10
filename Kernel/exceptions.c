#include <video.h>
#include <interrupts.h>

#define ZERO_EXCEPTION_ID 0
#define INVALID_OPCODE_ID 6

extern void * getShellAddress();  // declaras la función

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

void exceptionDispatcher(ExceptionStackFrame *frame, int exception) {
    if (exception == ZERO_EXCEPTION_ID)
        zero_division(frame);
	if (exception == INVALID_OPCODE_ID)
        invalid_opcode(frame);
}

static void zero_division(ExceptionStackFrame *frame) {
    selectStyle(0x04);
    print("EXCEPTION: Division by Zero\n");
    frame->rip = (uint64_t)getShellAddress();  // vuelve al inicio de la shell
}

static void invalid_opcode(ExceptionStackFrame *frame) {
    selectStyle(0x04);
    print("EXCEPTION: Invalid Opcode\n");
    frame->rip = (uint64_t)getShellAddress();
}