%include 'io.inc'

section .text
global CMAIN
CMAIN:
    GET_UDEC 4, eax
    test eax, eax
    jz Lc1
    xor ebx, ebx
    mov ecx, 10

Lc0:
    test eax, eax
    jz Lc2
    xor edx, edx
    div ecx
    inc ebx
    jmp Lc0
Lc1:
    mov ebx, 1
Lc2:
    PRINT_UDEC 4, ebx

    xor eax, eax
    ret
