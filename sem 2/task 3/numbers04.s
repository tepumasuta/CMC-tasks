%include 'io.inc'

section .text
global CMAIN
CMAIN:
    mov eax, 1
    mov ebx, 1

Lc0:
    mov ecx, ebx
    mov ebx, eax
    add eax, ecx
    cmp eax, 100000
    jb Lc0

    PRINT_UDEC 4, eax

    xor eax, eax
    ret
