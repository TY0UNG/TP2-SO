section .text
GLOBAL getRDTSC
GLOBAL getSeconds
GLOBAL getMinutes
GLOBAL getHour
GLOBAL getDayOfWeek
GLOBAL getDayOfMonth
GLOBAL getMonth
GLOBAL getYear

getRDTSC:
	rdtsc
    shl rdx, 32
    or rax, rdx
    ret

%macro getFromRTC 1
	mov al, %1
	out 0x70, al
	in al, 0x71
	ret
%endmacro

getSeconds:
	getFromRTC 0

getMinutes:
	getFromRTC 2

getHour:
	getFromRTC 4

getDayOfWeek:
	getFromRTC 6

getDayOfMonth:
	getFromRTC 7

getMonth:
	getFromRTC 8

getYear:
	getFromRTC 9