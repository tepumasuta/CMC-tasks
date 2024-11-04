%include 'io.inc'

section .text
global CMAIN
CMAIN:
    GET_UDEC 4, eax
    xor ebx, ebx
    mov ecx, 10

Lc0:
    test eax, eax
    jz Lc1
    xor edx, edx
    div ecx
    cmp edx, ebx
    jle Lc0
    mov ebx, edx
    jmp Lc0
Lc1:
    PRINT_UDEC 4, ebx

    xor eax, eax
    ret
