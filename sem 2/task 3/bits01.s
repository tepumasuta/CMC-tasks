%include 'io.inc'

section .text
global CMAIN
CMAIN:
    GET_UDEC 4, edx ; num
    mov ecx, 32     ; loop counter
    xor eax, eax    ; max streak
    xor ebx, ebx    ; current streak

Lc1:
    test edx, 1
    jz Lj1
    inc ebx
    jmp Lj2
Lj1:
    xor ebx, ebx
Lj2:

    cmp eax, ebx
    jge Lj3
    mov eax, ebx
Lj3:

    rcr edx, 1
    loop Lc1

    PRINT_UDEC 4, eax

    xor eax, eax
    ret
