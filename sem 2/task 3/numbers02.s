%include 'io.inc'

section .bss
    prime resd 1

section .text
global CMAIN
CMAIN:
    GET_UDEC 4, ebx
    mov ecx, 2
    mov dword [prime], 1

Lc0:
    mov eax, ecx
    mul ecx
    jc Lc2
    cmp ebx, eax
    jb Lc2
    mov eax, ebx
    xor edx, edx
    div ecx
    test edx, edx
    jz Lc1
    inc ecx
    jmp Lc0

Lc1:
    mov dword [prime], 0
Lc2:
    mov eax, dword [prime]
    PRINT_UDEC 4, eax

    xor eax, eax
    ret
