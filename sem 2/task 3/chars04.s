%include 'io.inc'

section .text
global CMAIN
CMAIN:
    xor eax, eax
    xor ebx, ebx

Lc0:
    GET_CHAR cl
    cmp cl, '.'
    je Lc4

    cmp cl, ','
    je Lc1
    inc ebx
    jmp Lc3
Lc1:
    test ebx, 1
    jnz Lc2
    inc eax
Lc2:
    xor ebx, ebx
Lc3:
    jmp Lc0
Lc4:

    test ebx, 1
    jnz Lc5
    inc eax
Lc5:
    PRINT_UDEC 4, eax

    xor eax, eax
    ret