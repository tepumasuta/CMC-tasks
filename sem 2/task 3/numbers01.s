%include 'io.inc'

section .text
global CMAIN
CMAIN:
    GET_UDEC 4, ecx
    xor eax, eax
    inc eax
    GET_DEC 4, ebx
    dec ecx
    jz Lc3

Lc0:
    GET_DEC 4, edx
    cmp edx, ebx
    jl Lc2
    je Lc1
    xor eax, eax
    mov ebx, edx
Lc1:
    inc eax
Lc2:
    loop Lc0
Lc3:

    PRINT_UDEC 4, eax

    xor eax, eax
    ret
