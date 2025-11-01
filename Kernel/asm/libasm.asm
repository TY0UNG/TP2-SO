section .text
GLOBAL cpuVendor
GLOBAL fast_memcpy
GLOBAL fast_memset
	
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
    push rcx
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
    pop rcx
    ; 6. Retorno
    ; RAX ya contiene el puntero de destino original (dest).
    ret

    ; extern void *fast_memset(void *s, int c, size_t n);
; Argumentos:
; RDI = dest (puntero de destino)
; RSI = c (valor, solo el byte menos significativo es relevante)
; RDX = n (número de bytes)
; Retorno:
; RAX = dest original

fast_memset:
    ; 1. Guardar el puntero de destino original para el valor de retorno
    ; RAX = RDI (dest original)
    mov rax, rdi

    ; 2. Preparar el valor a escribir (c) como un QWORD (8 bytes)
    ; RSI tiene 'c' (un entero, solo el byte bajo es relevante: (int)c & 0xFF)
    ; La instrucción STOSQ utiliza RAX para el valor.
    
    ; a. Aislar el byte (c & 0xFF)
    movzx r8, sil      ; Mover el byte menos significativo de RSI (c) a R8

    ; b. Replicar el byte 8 veces en RAX
    ; R8 ahora contiene el byte replicado 8 veces: [b | b | b | b | b | b | b | b]
    ; Esto se hace para que 'rep stosq' escriba 8 bytes a la vez.
    mov r9, r8         ; r9 = b
    shl r9, 8          ; r9 = b << 8 = [0b | b]
    or r8, r9          ; r8 = [0b | b] | [0 | b] = [0b | bb]
    
    mov r9, r8         ; r9 = [0b | bb]
    shl r9, 16         ; r9 = [bb | 00 | 00 | 00]
    or r8, r9          ; r8 = [bb | bb]
    
    mov r9, r8         ; r9 = [bb | bb]
    shl r9, 32         ; r9 = [bb | bb | 00 | 00 | 00 | 00 | 00 | 00]
    or r8, r9          ; r8 = [bb | bb | bb | bb] (QWORD completo)

    mov rax, r8        ; RAX ahora tiene el QWORD replicado (el valor a escribir)
    
    ; 3. Determinar el número de bloques de 64 bits (QWORDs)
    ; El contador de repeticiones es RCX.
    mov rcx, rdx       ; RCX = RDX (n)
    shr rcx, 3         ; RCX = n / 8 (número de QWORDs)

    ; 4. Escribir los bloques de 64 bits
    ; RDI = Destino
    ; RAX = Valor QWORD a escribir
    ; RCX = Contador de repeticiones (QWORDs)
    
    cld                ; Clear Direction Flag (DF=0 -> auto-incremento de RDI)
    rep stosq          ; Almacena RCX QWORDs ([RAX]) en [RDI]
                       ; RDI se incrementa en 8 * RCX

    ; 5. Manejar el resto (bytes sobrantes)
    ; Calcular el resto de bytes
    ; RDX aún contiene el 'n' original. Usamos AND para obtener el resto: n % 8
    mov rcx, rdx       ; RCX = n
    and rcx, 7         ; RCX = n % 8 (solo los 3 bits menos significativos)

    ; 6. Escribir los bytes restantes
    ; RDI ya está en la posición correcta.
    ; AL = Byte menos significativo de RAX (el valor 'c' preparado).
    ; RCX = Contador de repeticiones (bytes sobrantes: 0 a 7)
    rep stosb          ; Almacena RCX bytes ([AL]) en [RDI]

    ; 7. Retorno
    ; RAX ya contiene el puntero de destino original (dest).
    ret