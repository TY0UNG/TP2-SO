section .text
GLOBAL addregs

GLOBAL saved_stack_ptr
GLOBAL initial_saved_stack_ptr       

addregs:
    mov rsi, [saved_stack_ptr]

    mov rax, [rsi + 112]    ; Cargar RAX desde el stack
    mov [rdi + 0], rax

    mov rax, [rsi + 104]    ; Cargar RBX
    mov [rdi + 8], rax

    mov rax, [rsi + 96]     ; Cargar RCX
    mov [rdi + 16], rax

    mov rax, [rsi + 88]     ; Cargar RDX
    mov [rdi + 24], rax

    mov rax, [rsi + 80]     ; Cargar RBP
    mov [rdi + 32], rax

    mov rax, [rsi + 72]     ; Cargar RDI
    mov [rdi + 40], rax

    mov rax, [rsi + 64]     ; Cargar RSI
    mov [rdi + 48], rax

    mov rax, [rsi + 56]     ; Cargar R8
    mov [rdi + 56], rax

    mov rax, [rsi + 48]     ; Cargar R9
    mov [rdi + 64], rax

    mov rax, [rsi + 40]     ; Cargar R10
    mov [rdi + 72], rax

    mov rax, [rsi + 32]     ; Cargar R11
    mov [rdi + 80], rax

    mov rax, [rsi + 24]     ; Cargar R12
    mov [rdi + 88], rax

    mov rax, [rsi + 16]     ; Cargar R13
    mov [rdi + 96], rax
    
    mov rax, [rsi + 8]      ; Cargar R14
    mov [rdi + 104], rax

    mov rax, [rsi + 0]      ; Cargar R15
    mov [rdi + 112], rax
    
; -----------------------------------------------------------------------------
; Copia del Stack Frame de la Interrupción (guardado por el CPU)
; -----------------------------------------------------------------------------

    ; RIP original (Instruction Pointer)
    mov rax, [rsi + 120]
    mov [rdi + 120], rax

    ; RFLAGS original
    mov rax, [rsi + 136]
    mov [rdi + 128], rax
    
    ; CS original (Code Segment)
    mov ax, [rsi + 128]
    mov [rdi + 144], ax ; Se guarda el QWORD completo por alineación
    mov cx, ax
    and cx, 0x7
    mov ax, cs
    and ax, 0x7
    cmp ax, cx
    jne .privilege_changed

    ; RSP original (Stack Pointer)
    mov rax, [initial_saved_stack_ptr]
    mov [rdi + 136], rax

    ; SS original (Stack Segment)
    mov ax, ss
    mov [rdi + 148], ax ; Se guarda el QWORD completo por alineación
    jmp .stack_done

.privilege_changed:

    ; RSP original (Stack Pointer)
    mov rax, [rsi + 144]
    mov [rdi + 136], rax

    ; SS original (Stack Segment)
    mov ax, [rsi + 152]
    mov [rdi + 148], ax ; Se guarda el QWORD completo por alineación

.stack_done:

; -----------------------------------------------------------------------------
; Copia de otros Segmentos (no se guardan automáticamente)
; Es correcto leerlos del estado actual de la máquina.
; Asumimos que la struct tiene espacio para ellos después de SS.
; -----------------------------------------------------------------------------
    mov ax, ds
    mov [rdi + 160], ax
    mov ax, es
    mov [rdi + 162], ax
    mov ax, fs
    mov [rdi + 164], ax
    mov ax, gs
    mov [rdi + 166], ax
    
    ret

section .bss
backrax: resq 1                 
backrbp: resq 1                 
backrsp: resq 1 
backrdi: resq 1 

saved_stack_ptr resq 1      ;Donde estan guardados los registros 
initial_saved_stack_ptr resq 1