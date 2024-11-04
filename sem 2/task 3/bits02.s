%include 'io.inc'

section .bss
    max_ones resd 0

section .text
global CMAIN
CMAIN:
    xor eax, eax ; number
    mov dword [max_ones], 0

Lc1:
    GET_DEC 4, edx ; num
    test edx, edx
    jz Lc5
    xor ebx, ebx
    mov ecx, 32

Lc2:
    test edx, 1
    jz Lc3
    inc ebx
Lc3:
    ror edx, 1
    loop Lc2

    cmp dword [max_ones], ebx
    jge Lc4
    mov eax, edx
    mov dword [max_ones], ebx
Lc4:
    jmp Lc1

Lc5:
    PRINT_DEC 4, eax

    xor eax, eax
    ret
