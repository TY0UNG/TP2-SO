section .text

GLOBAL backupregs 
GLOBAL addregs

GLOBAL saved_stack_ptr             

    backupregs:

    mov [backrbp], rbp
    ;mov [backrsp], [rsp + ]           ; bajo del return 
    mov [backrsp], rsp 
    mov [backrax] ,rax
    mov [backrdi],rdi
    ret 

addregs:
   ; RDI = puntero a struct(destino)
    
    
    mov rsi, [saved_stack_ptr]
    
   
    mov rax, [rsi + 0]       ; rax
    mov [rdi + 6], rax
    mov rax, [rsi + 8]       ; rbx
    mov [rdi + 14], rax
    mov rax, [rsi + 16]      ; rcx
    mov [rdi + 22], rax
    mov rax, [rsi + 24]      ; rdx
    mov [rdi + 30], rax
    mov rax, [rsi + 32]      ; rbp
    mov [rdi + 38], rax
    mov rax, [rsi + 40]      ; rdi orig
    mov [rdi + 46], rax
    mov rax, [rsi + 48]      ; rsi orig
    mov [rdi + 54], rax
    
    ; RSP original
    lea rax, [rsi + 120 + 24]
    mov [rdi + 62], rax
    mov rax, [rsi + 56]      ; r8
    mov [rdi + 70], rax
    mov rax, [rsi + 64]      ; r9
    mov [rdi + 78], rax
    mov rax, [rsi + 72]      ; r10
    mov [rdi + 86], rax
    mov rax, [rsi + 80]      ; r11
    mov [rdi + 94], rax
    mov rax, [rsi + 88]      ; r12
    mov [rdi + 102], rax
    mov rax, [rsi + 96]      ; r13
    mov [rdi + 110], rax
    
    mov rax, [rsi + 104]     ; r14
    mov [rdi + 118], rax
    mov rax, [rsi + 112]     ; r15
    mov [rdi + 126], rax
    
    ; RIP 
    mov rax, [rsi + 120]
    mov [rdi + 134], rax
    
    ; RFLAGS
    mov rax, [rsi + 136]
    mov [rdi + 142], rax
    
    ; Segments
    mov ax, [rsi + 128]      ; CS
    mov [rdi + 150], ax
    mov ax, ds
    mov [rdi + 152], ax
    mov ax, es
    mov [rdi + 154], ax  
    mov ax, fs
    mov [rdi + 156], ax
    mov ax, gs
    mov [rdi + 158], ax
    mov ax, [rsi + 152]      ; SS
    mov [rdi + 160], ax
    
    ret

    

section .bss
backrax: resq 1                 
backrbp: resq 1                 
backrsp: resq 1 
backrdi: resq 1 

saved_stack_ptr resq 1      ;Donde estan guardados los registros 