SECTION .text
GLOBAL sys_write
GLOBAL sys_read
GLOBAL sys_clear
GLOBAL sys_graphics_mode
GLOBAL sys_draw_line
GLOBAL sys_draw_rectangle
GLOBAL sys_draw_filled_rectangle
GLOBAL sys_draw_fill_screen
GLOBAL sys_draw_circle
GLOBAL sys_draw_filled_circle
GLOBAL sys_draw_text
GLOBAL sys_clear_canvas
GLOBAL sys_swap_buffers
GLOBAL sys_get_time
GLOBAL sys_get_ms
GLOBAL sys_get_key
GLOBAL sys_get_reg
GLOBAL sys_set_text_size
GLOBAL sys_play_sound
GLOBAL sys_is_audio_buffer_empty
GLOBAL sys_clear_audio_buffer
GLOBAL sys_set_fps_overlay
GLOBAL sys_malloc
GLOBAL sys_free
GLOBAL sys_get_total_memory
GLOBAL sys_get_used_memory
GLOBAL sys_create_process
GLOBAL sys_exit
GLOBAL sys_wait
GLOBAL sys_yield
GLOBAL sys_set_foreground
GLOBAL sys_write_fd
GLOBAL sys_read_fd
GLOBAL sys_close_fd
GLOBAL sys_get_process_list

EXTERN printHex

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

sys_get_time:
    START_SYSCALL
    mov rbx, rdi
    mov rax, 13
    int 80h
    END_SYSCALL

sys_get_ms:
    START_SYSCALL
    mov rax, 14
    int 80h
    END_SYSCALL

sys_get_key:
    START_SYSCALL
    mov rax, 15
    int 80h
    END_SYSCALL

sys_get_reg:
    START_SYSCALL
    mov rax, 16
    int 80h
    END_SYSCALL

sys_set_text_size:
    START_SYSCALL
    mov rax, 17
    mov rbx, rdi
    int 80h
    END_SYSCALL

sys_play_sound:
    START_SYSCALL
    mov rax, 18
    mov rbx, rdi    ; frec
    mov rcx, rsi    ; dur
    int 80h
    END_SYSCALL

sys_is_audio_buffer_empty:      
    START_SYSCALL
    mov rax, 19
    int 80h
    END_SYSCALL

sys_clear_audio_buffer:
    START_SYSCALL
    mov rax, 20
    int 80h
    END_SYSCALL

sys_draw_fill_screen:
    START_SYSCALL
    mov rax, 21
    mov rbx, rdi
    int 80h
    END_SYSCALL

sys_set_fps_overlay:
    START_SYSCALL
    mov rax, 22
    mov rbx, rdi
    int 80h
    END_SYSCALL

sys_malloc:
    START_SYSCALL
    mov rax, 23
    mov rbx, rdi
    int 80h
    END_SYSCALL

sys_free:
    START_SYSCALL
    mov rax, 24
    mov rbx, rdi
    int 80h
    END_SYSCALL

sys_get_total_memory:
    START_SYSCALL
    mov rax, 25
    int 80h
    END_SYSCALL

sys_get_used_memory:
    START_SYSCALL
    mov rax, 26
    int 80h
    END_SYSCALL

; sys_create_process(name, entry, argc, argv)
; Caller SysV: rdi=name, rsi=entry, rdx=argc, rcx=argv
; Kernel:      rbx=name, rcx=entry, rdx=argc, rdi=argv
sys_create_process:
    START_SYSCALL
    mov rax, 27
    mov rbx, rdi        ; name
    push rcx            ; preservar argv antes de pisar rcx
    mov rcx, rsi        ; entry
    pop rdi             ; argv -> rdi (4to arg al kernel)
    ; rdx ya tiene argc
    int 80h
    END_SYSCALL

; sys_exit(status)
sys_exit:
    START_SYSCALL
    mov rax, 28
    mov rbx, rdi        ; status
    int 80h
    END_SYSCALL

; sys_wait(pid) -> exit_status
sys_wait:
    START_SYSCALL
    mov rax, 29
    mov rbx, rdi        ; pid
    int 80h
    END_SYSCALL

; sys_yield()
sys_yield:
    START_SYSCALL
    mov rax, 30
    int 80h
    END_SYSCALL

; sys_set_foreground(pid)
sys_set_foreground:
    START_SYSCALL
    mov rax, 31
    mov rbx, rdi        ; pid
    int 80h
    END_SYSCALL

; sys_write_fd(fd, buf, count)
sys_write_fd:
    START_SYSCALL
    mov rax, 32
    mov rbx, rdi        ; fd
    mov rcx, rsi        ; buf
    ; rdx ya tiene count
    int 80h
    END_SYSCALL

; sys_read_fd(fd, buf, count)
sys_read_fd:
    START_SYSCALL
    mov rax, 33
    mov rbx, rdi        ; fd
    mov rcx, rsi        ; buf
    ; rdx ya tiene count
    int 80h
    END_SYSCALL

; sys_close_fd(fd)
sys_close_fd:
    START_SYSCALL
    mov rax, 34
    mov rbx, rdi        ; fd
    int 80h
    END_SYSCALL

; sys_get_process_list(buffer, maxCount);
sys_get_process_list:
    START_SYSCALL
    mov rax, 35
    mov rbx, rdi
    mov rcx, rsi
    int 80h
    END_SYSCALL