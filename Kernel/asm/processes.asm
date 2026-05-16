section .text
GLOBAL switch_rsp
GLOBAL trampoline

trampoline:
	pop rsi
    pop rdi
	iretq

switch_rsp:
    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15

    mov [rdi], rsp          ; guardo MI rsp
    mov rsp, rsi            ; cargo el de OTRO

    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp

    ret