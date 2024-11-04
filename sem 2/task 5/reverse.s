section .text
global reverse
reverse:
	push ebp
	mov ebp, esp
	sub esp, 4

	mov ecx, dword [ebp + 12]
	sar ecx, 1
	xor edx, edx
	mov eax, dword [ebp + 8]
.L:
	push dword [eax + edx * 4]
	push eax
	mov eax, dword [eax + ecx * 4 - 4]
	loop ecx

	add esp, 4
	pop ebp
	ret

