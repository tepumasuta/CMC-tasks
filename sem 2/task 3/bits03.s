%include 'io.inc'

section .bss
    count resd 1
    left  resd 1

section .text
global CMAIN
CMAIN:
    mov dword [count], 0
    GET_UDEC 4, eax
    mov dword [left], eax

Lc0:
    GET_DEC 4, edx ; num
    mov ecx, 32    ; loop counter
    xor eax, eax   ; max streak
    xor ebx, ebx   ; current streak

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

    ror edx, 1
    loop Lc1

    cmp eax, 5
    jl Lj4
    inc dword [count]
Lj4:
    dec dword [left]
    jnz Lc0

    mov eax, dword [count]
    PRINT_DEC 4, eax

    xor eax, eax
    ret
