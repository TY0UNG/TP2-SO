section .text
GLOBAL switch_rsp
GLOBAL switch_to_idle
GLOBAL trampoline
EXTERN idle

trampoline:
	pop rsi
    pop rdi
	iretq

; void switch_to_idle(uint64_t *save_rsp, uint64_t idle_rsp)
;   rdi = donde guardar el rsp del proceso saliente (o &dummy si no hay)
;   rsi = tope del kernel stack sobre el que corre idle
; Guarda el rsp saliente con LA MISMA convencion que switch_rsp (para que un
; switch_rsp posterior pueda reanudar al proceso bloqueado), resetea rsp al
; kernel stack y corre idle(), que nunca retorna: hace hlt hasta que el timer
; tick vuelva a interrumpir y el scheduler switchee a un proceso real.
switch_to_idle:
    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15

    mov [rdi], rsp          ; guardo MI rsp (resumible por switch_rsp)
    mov rsp, rsi            ; reseteo al tope del kernel stack
    call idle              ; idle = while(1) _hlt(); no retorna

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