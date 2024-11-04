%include 'io.inc'

section .bss
    num resd 1
    len resd 1

section .text
global CMAIN
CMAIN:
    GET_UDEC 4, eax
    mov dword [num], eax
    
    ; calculting it's length
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
    mov dword [len], ebx

    ; comparing least digit of the number with highest until number ends
Lc3:
    mov eax, dword [num]
    test eax, eax
    jz Le0
    mov ecx, 10
    xor edx, edx
    div ecx
    mov ebx, edx ; least digit

    mov ecx, dword [len]
    cmp ecx, 1
    je Le0
    mov edx, 1
    sub ecx, 2
    jz Lce4
Lc4:
    imul edx, 10
    loop Lc4
Lce4:

    mov ecx, edx
    xor edx, edx
    div ecx
    mov dword [num], edx
    ; highest digit in eax
    cmp ebx, eax
    jne Li0
    sub dword [len], 2
    jmp Lc3

Li0:
    mov eax, 0
    jmp Lie0
Le0:
    mov eax, 1
Lie0:
    PRINT_UDEC 4, eax

    xor eax, eax
    ret
