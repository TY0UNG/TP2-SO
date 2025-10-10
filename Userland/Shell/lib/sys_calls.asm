SECTION .text
GLOBAL sys_write
GLOBAL sys_read
GLOBAL sys_clear
GLOBAL sys_shutdown
GLOBAL sys_graphics_mode
GLOBAL sys_draw_line
GLOBAL sys_draw_rectangle
GLOBAL sys_draw_filled_rectangle
GLOBAL sys_draw_circle
GLOBAL sys_draw_filled_circle
GLOBAL sys_draw_text
GLOBAL sys_clear_canvas
GLOBAL sys_swap_buffers


%macro START_SYSCALL 0
    push rbp
    mov rbp, rsp
    push rbx
    push rdi
    push rsi
%endmacro

%macro END_SYSCALL 0
    pop rsi
    pop rdi
    pop rbx
    mov rsp, rbp
    pop rbp
    ret
%endmacro


sys_write:
    START_SYSCALL
    mov rax, 1
    mov rbx, rdi
    mov rcx, rsi
    int 80h
    END_SYSCALL


sys_read:
    START_SYSCALL
    mov rax, 2
    mov rbx, rdi
    int 80h
    END_SYSCALL


sys_clear:
    START_SYSCALL
    mov rax, 3
    int 80h
    END_SYSCALL


sys_shutdown:
    START_SYSCALL
    mov rax, 0
    int 80h
    END_SYSCALL

sys_graphics_mode:
    START_SYSCALL
    mov rax, 4
    mov rbx, rdi
    int 80h
    END_SYSCALL

sys_draw_line:
    START_SYSCALL
    mov rax, 5
    mov rbx, rdi
    int 80h
    END_SYSCALL


sys_draw_rectangle:
    START_SYSCALL
    mov rax, 6
    mov rbx, rdi
    int 80h
    END_SYSCALL


sys_draw_filled_rectangle:
    START_SYSCALL
    mov rax, 7
    mov rbx, rdi
    int 80h
    END_SYSCALL


sys_draw_circle:
    START_SYSCALL
    mov rax, 8
    mov rbx, rdi
    int 80h
    END_SYSCALL


sys_draw_filled_circle:
    START_SYSCALL
    mov rax, 9
    mov rbx, rdi
    int 80h
    END_SYSCALL


sys_draw_text:
    START_SYSCALL
    mov rax, 10
    mov rbx, rdi
    int 80h
    END_SYSCALL

sys_clear_canvas:
    START_SYSCALL
    mov rax, 11
    int 80h
    END_SYSCALL
    
sys_swap_buffers:
    START_SYSCALL
    mov rax, 12
    int 80h
    END_SYSCALL