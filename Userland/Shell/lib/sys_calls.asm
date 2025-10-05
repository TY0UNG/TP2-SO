SECTION .text
GLOBAL sys_write
GLOBAL sys_read
GLOBAL sys_clear


sys_write:
    push rbp
    mov rbp, rsp

    push rax
    push rbx
    push rcx

    mov rax, 1
    mov rbx, rdi
    mov rcx, rsi
    int 80h

    pop rcx
    pop rbx
    pop rax

    mov rsp, rbp
    pop rbp
    ret

sys_read:
    push rbp
    mov rbp, rsp

    push rbx

    mov rax, 2
    mov rbx, rdi
    int 80h

    pop rbx

    mov rsp, rbp
    pop rbp
    ret


    sys_clear:
    push rbp
    mov rbp, rsp
    
    mov rax, 3      ; ver numeor 
    int 80h
    
    mov rsp, rbp
    pop rbp
    ret