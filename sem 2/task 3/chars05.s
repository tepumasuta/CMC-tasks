%include 'io.inc'

section .text
global CMAIN
CMAIN:
    xor eax, eax ; count
    xor cl, cl   ; previous char

Lc0:
    mov ch, cl
    GET_CHAR cl
    cmp cl, '.'
    je Lc1

    cmp cl, ','
    jne Lc0
    cmp ch, 'z'
    jne Lc0
    inc eax
    jmp Lc0
Lc1:
    cmp ch, 'z'
    jne Lc2
    inc eax
Lc2:

    PRINT_UDEC 4, eax

    xor eax, eax
    ret
