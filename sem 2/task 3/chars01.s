%include 'io.inc'

section .text
global CMAIN
CMAIN:

Lc0:
    GET_CHAR al
    cmp al, '.'
    jz Lc3

    cmp al, '0'
    jl Lc2
    cmp al, '9'
    jg Lc2
    je Lc1
    inc al
    jmp Lc2
Lc1:
    mov al, '0'
Lc2:
    PRINT_CHAR al
    jmp Lc0
Lc3:

    xor eax, eax
    ret
