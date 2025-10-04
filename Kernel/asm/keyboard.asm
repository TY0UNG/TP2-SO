section .text
global get_keyboard_output

get_keyboard_output:
    xor rax, rax
    in al, 60h
    movzx rax, al
    ret