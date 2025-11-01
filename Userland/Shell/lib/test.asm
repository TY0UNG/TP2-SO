SECTION .text
GLOBAL dividezero_test
GLOBAL invalidopcode_test

dividezero_test:
    mov eax, 100
    mov ebx, 0
    idiv ebx
    ret

invalidopcode_test:
    ud2
    ret