section .text
GLOBAL cpuVendor
GLOBAL fast_memcpy
	
cpuVendor:
	push rbp
	mov rbp, rsp

	push rbx

	mov rax, 0
	cpuid

	mov [rdi], ebx
	mov [rdi + 4], edx
	mov [rdi + 8], ecx

	mov byte [rdi+13], 0

	mov rax, rdi

	pop rbx

	mov rsp, rbp
	pop rbp
	ret

fast_memcpy:
    ; 1. Guardar el puntero de destino original para el valor de retorno
    ; El valor de retorno debe ser el puntero de destino original (RDI).
    ; Lo guardamos en RAX para devolverlo al final.
    mov rax, rdi

    ; 2. Determinar el número de bloques de 64 bits (QWORDs)
    ; Queremos mover 8 bytes a la vez (movsq).
    ; n (RDX) / 8 -> número de QWORDs
    mov rcx, rdx
    shr rcx, 3  ; RCX = RDX / 8 (n / 8)

    ; 3. Copiar los bloques de 64 bits
    ; RDI = Destino
    ; RSI = Fuente
    ; RCX = Contador de repeticiones (QWORDs)
    ; Dirección de incremento automático (DF=0)
    cld         ; Clear Direction Flag (DF=0 -> auto-incremento de RDI/RSI)
    rep movsq   ; Copia RCX QWORDs (8 bytes) de [RSI] a [RDI]

    ; 4. Manejar el resto (bytes sobrantes)
    ; RDX contiene ahora el número de bytes que quedan (RDX = RDX % 8)
    ; Esto se debe a que la operación 'shr rcx, 3' modificó RDX
    ; Solo necesitamos los bits 0-2 de RDX original.

    ; Calcular el resto de bytes
    mov rcx, rdx  ; RCX tiene ahora los bytes originales a copiar (n)
    and rcx, 7    ; RCX = RDX % 8 (solo los 3 bits menos significativos)

    ; 5. Copiar los bytes restantes
    ; RDI, RSI ya están incrementados a la posición correcta
    ; RCX = Contador de repeticiones (bytes sobrantes: 0 a 7)
    rep movsb   ; Copia RCX bytes de [RSI] a [RDI]

    ; 6. Retorno
    ; RAX ya contiene el puntero de destino original (dest).
    ret