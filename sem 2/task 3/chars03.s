%include 'io.inc'

section .text
global CMAIN
CMAIN:
    xor ebx, ebx

Lc0:
    GET_CHAR al
    cmp al, '.'
    jz Lc3

    cmp al, '0'
    jl Lc2
    cmp al, '9'
    jg Lc2
    sub al, '0'
    test al, 1
    jz Lc2
    inc ebx
Lc2:
    jmp Lc0
Lc3:
    PRINT_UDEC 4, ebx
    xor eax, eax
    ret
