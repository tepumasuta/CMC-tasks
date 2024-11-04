%include 'io.inc'

section .text
global CMAIN
CMAIN:
    GET_CHAR ebx
    mov ecx, ebx
    xor eax, eax

Lc0:
    GET_CHAR edx
    cmp edx, '.'
    je Le

    cmp edx, ','
    jne Li0
    cmp ebx, ecx
    jne Li1
    inc eax
Li0:
    cmp ebx, ','
    jne Li1
    mov ecx, edx
Li1:
    mov ebx, edx
    jmp Lc0
    

Le:
    cmp ebx, ecx
    jne Li2
    inc eax
Li2:
    PRINT_DEC 4, eax

    xor eax, eax
    ret
