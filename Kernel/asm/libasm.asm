GLOBAL cpuVendor
GLOBAL getTime


section .text
	
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


getTime:
	; recibe en rdi el puntero al buffer de respuesta
	push rbp
	mov rbp, rsp 

	call getYear
	mov [rdi], al

	call getMonth
	mov [rdi+1], al

	call getHour

	;;;;;;;;	CAMBIO DE HORA A ARGENTINA  	;;;;;

	; Cambio formato para restar 
	mov bl, al          ; Backup del valor BCD
	shr al, 4           ; al = decenas ( alto)
	mov cl, 10
	mul cl              ; al = decenas * 10
	and bl, 0x0F        ; bl = unidades ( bajo)
	add al, bl          ; al = hora en binario (0-23)
	
	; Restar 3 horas  
	sub al, 3
	jns wrap         	; Si no es negativo, continuar
	add al, 24          ; Si es negativo, sumar 24
	
	wrap:
	; pasa de binaro a formato anteriro 
	xor ah, ah          ; Limpiar ah
	mov cl, 10
	div cl              ; al = decenas, ah = unidades
	shl al, 4           ; Mover decenas al alto
	or al, ah           ; Combinar: (decenas << 4) | unidades

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	

	mov [rdi+2], al

	call getMinutes
	mov [rdi+3], al

	call getSeconds
	mov [rdi+4], al 

	leave
	ret



getSeconds:
	mov al, 0
	out 0x70, al
	in al, 0x71
	ret

getMinutes:
	mov al, 2
	out 0x70, al
	in al, 0x71
	ret

getHour:
	mov al, 4
	out 0x70, al
	in al, 0x71
	ret

getDayOfWeek:
	mov al, 6
	out 0x70, al
	in al, 0x71
	ret

getMonth:
	mov al, 8
	out 0x70, al
	in al, 0x71
	ret

getYear:
	mov al, 9
	out 0x70, al
	in al, 0x71
	ret