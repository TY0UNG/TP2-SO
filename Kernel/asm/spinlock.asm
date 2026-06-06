section .text
GLOBAL enter_region
GLOBAL leave_region

; void enter_region(volatile uint8_t * lock)
; Exclusion mutua mediante TSL. En x86 la primitiva TSL es XCHG con un
; operando en memoria, que es atomica de forma implicita (bus lock).
; Copia el valor del lock a un registro y lo deja en 1 en una sola operacion;
; si el valor previo era 1 el lock estaba tomado y se hace busy-wait.
;   RDI = puntero al lock (1 byte)
enter_region:
.spin:
    mov al, 1
    xchg al, [rdi]      ; TSL: intercambia atomicamente lock <- 1, al <- viejo
    test al, al
    jnz .spin           ; si estaba en 1, seguir girando hasta liberarlo
    ret

; void leave_region(volatile uint8_t * lock)
;   RDI = puntero al lock (1 byte)
leave_region:
    mov byte [rdi], 0   ; liberar el lock
    ret
