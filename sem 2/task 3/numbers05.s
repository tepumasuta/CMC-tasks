%include 'io.inc'

section .text
global CMAIN
CMAIN:
    GET_DEC 4, edx
    sar edx, 31
    mov ecx, edx
    xor eax, eax

Lc0:
    GET_DEC 4, edx
    test edx, edx
    jz Lce0
    sar edx, 31
    cmp ecx, edx
    je Lie0
    inc eax 
Lie0:
    mov ecx, edx
    jmp Lc0

Lce0:
    PRINT_UDEC 4, eax

    xor eax, eax
    ret