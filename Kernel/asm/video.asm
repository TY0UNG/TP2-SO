section .text
global vsync_wait

vsync_wait:
    ; El puerto de estado de la VGA es 0x3DA
    mov dx, 0x3DA

; Primero, esperamos a que termine cualquier V-Blank que ya esté en progreso.
; Esto asegura que capturemos el *inicio* del siguiente.
.wait_for_not_retrace:
    in al, dx          ; Lee el byte de estado del puerto en AL
    test al, 0x08      ; Comprueba si el bit 3 (Vertical Retrace) está activado
    jnz .wait_for_not_retrace ; Si está activado (no es cero), sigue esperando

; Ahora, esperamos a que comience el siguiente V-Blank.
.wait_for_retrace:
    in al, dx          ; Lee el byte de estado de nuevo
    test al, 0x08      ; Comprueba si el bit 3 está activado
    jz .wait_for_retrace   ; Si está desactivado (es cero), sigue esperando

    ; En este punto, el V-Blank acaba de comenzar. La función puede retornar.
    ret