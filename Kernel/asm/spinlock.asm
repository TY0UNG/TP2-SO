section .text
GLOBAL enter_region
GLOBAL leave_region

; uint64_t enter_region(volatile uint8_t * lock)
; Exclusion mutua mediante TSL (XCHG, atomico por bus lock) + enmascarado de
; interrupciones. Guarda RFLAGS y deshabilita IF ANTES de tomar el lock, asi la
; seccion critica corre con interrupciones apagadas: en single-core el holder no
; puede ser preemptado mientras la tiene (el spin nunca gira de verdad) y, lo
; clave, una ISR que tome el mismo lock no se deadlockea ni reabilita IF.
; Devuelve los RFLAGS previos, que el caller debe pasarle a leave_region.
;   RDI = puntero al lock (1 byte)   ->   RAX = RFLAGS previos
enter_region:
    pushfq
    pop rax             ; rax = RFLAGS previos (valor de retorno)
    cli
.spin:
    mov dl, 1
    xchg dl, [rdi]      ; TSL: lock <- 1, dl <- viejo
    test dl, dl
    jnz .spin           ; si estaba en 1, girar hasta liberarlo
    ret

; void leave_region(volatile uint8_t * lock, uint64_t flags)
; Libera el lock y restaura los RFLAGS que devolvio enter_region (re-habilita IF
; solo si estaba habilitado antes; nunca lo prende dentro de una ISR).
;   RDI = puntero al lock (1 byte), RSI = flags
leave_region:
    mov byte [rdi], 0   ; liberar el lock
    push rsi
    popfq               ; restaurar RFLAGS (incluye IF)
    ret
