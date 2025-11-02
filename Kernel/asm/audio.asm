section .text

GLOBAL output_audio_start  
GLOBAL stop_audio
GLOBAL start_T


output_audio_start:
    push rbp
    mov rbp, rsp
    push rax
    push rdx
    push rbx

    ; rdi = freq
    test rdi, rdi
    jz .stop_and_end        ; Si freq = 0, apagar altavoz

    ; Calcular divisor para la frecuencia
    mov rax, 1193180        ; Frecuencia base PIT
    xor rdx, rdx
    div rdi                 ; rax = divisor
    mov rbx, rax            ; Guardar divisor en rbx

    ; Configurar PIT canal 2 (generar la frecuencia)
    mov al, 0xB6            ; Modo 3, LSB/MSB, canal 2
    out 0x43, al
    mov al, bl              ; LSB del divisor
    out 0x42, al
    mov al, bh              ; MSB del divisor
    out 0x42, al

    ; Activar speaker (Puerto 0x61)
    in al, 0x61
    or al, 0x03             ; Poner bits 0 y 1 en ON
    out 0x61, al

    jmp .end

.stop_and_end:
    ; Apagar speaker (si freq = 0)
    in al, 0x61
    and al, 0xFC            ; Poner bits 0 y 1 en OFF
    out 0x61, al

.end:
    pop rbx
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

start_T:
    ; Configurar PIT canal 0 para ~18.2 Hz (tick ≈ 55 ms)
    mov al, 0x36            ; Modo 3, LSB/MSB, canal 0
    out 0x43, al
    mov ax, 0xFFFF          ; 65535 → 1193180 / 65535 ≈ 18.2 Hz
    out 0x40, al            ; LSB
    mov al, ah
    out 0x40, al            ; MSB
    ret