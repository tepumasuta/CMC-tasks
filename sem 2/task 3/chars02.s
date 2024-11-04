%include 'io.inc'

section .text
global CMAIN
CMAIN:

Lc0:
    GET_CHAR al
    cmp al, '.'
    jz Lc3

    cmp al, 'A'
    jl Lc2
    cmp al, 'Z'
    jg Lc2
    mov ah, 'Z'
    add ah, 'A'
    sub ah, al
    mov al, ah
Lc2:
    PRINT_CHAR al
    jmp Lc0
Lc3:

    xor eax, eax
    ret
