%include 'io.inc'

section .text
global CMAIN
CMAIN:
    GET_DEC 4, eax
    xor ecx, ecx
    cmp eax, 0
    jle Li1
    mov ebx, 3

Lc0:
    cmp eax, 1
    je Li2
    xor edx, edx
    idiv ebx
    test edx, edx
    jnz Li1
    inc ecx
    jmp Lc0

Li1:
    mov ecx, -1
Li2:
    PRINT_DEC 4, ecx

    xor eax, eax
    ret
