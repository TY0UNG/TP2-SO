section .text
GLOBAL output_audio
GLOBAL stop_audio

output_audio:
    push rbp
    mov rbp, rsp
    push rax
    push rdx
    push rcx
    push rbx
    
    ; rdi = frec, rsi = dura
    test rdi, rdi
    jz .end
    
    ; Calcular divisor = 1193180 / frecuencia
    mov rax, 1193180
    xor rdx, rdx
    div rdi
    mov rbx, rax            
    

    mov al, 0xB6
    out 0x43, al
    mov al, bl
    out 0x42, al
    mov al, bh
    out 0x42, al
    
    ; Activar speaker
    in al, 0x61
    or al, 0x03
    out 0x61, al
    
    ; (rsi = ms)
    mov rcx, rsi
    imul rcx, 100000        ; Ajustar según necesites
.delay:
    pause
    dec rcx
    jnz .delay
    
    ; Desactivar speaker
    in al, 0x61
    and al, 0xFC
    out 0x61, al
    
.end:
    pop rbx
    pop rcx
    pop rdx
    pop rax
    pop rbp
    ret

stop_audio:
    push rax
    in al, 0x61
    and al, 0xFC
    out 0x61, al
    pop rax
    ret
    
