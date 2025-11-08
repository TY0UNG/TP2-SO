SECTION .text
GLOBAL dividezero_test
GLOBAL invalidopcode_test

dividezero_test:
    push 10
    push 10
    push 10
    push 10
    push 10
    push 10
    
    mov eax, 100
    mov ebx, 0
    idiv ebx
    ret

invalidopcode_test:
    ud2
    ret