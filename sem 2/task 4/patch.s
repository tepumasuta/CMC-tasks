section .text
global transform_caps
global leave_unique
transform_caps:
	push ebp
	mov ebp, esp

	mov ecx, dword [ebp + 8] ; load char *string
	mov dl, 10 ; for division in cycle
.L:
	mov al, byte [ecx]
	test al, al ; check for null byte
	jz .Le
	; check 'A' <= char <= 'Z'
	cmp al, 'A'
	jb .Lie
	cmp al, 'Z'
	ja .Lie
	; get oridnal number of letter
	sub al, 'A'
	inc al
	; get remainder
	xor ah, ah
	div dl
	add ah, '0' ; make actually printable character
	mov byte [ecx], ah ; write remainder into string
.Lie:
	inc ecx
	jmp .L
.Le:
	pop ebp
	ret

leave_unique:
	push ebp
	mov ebp, esp
	sub esp, 4 ; 1 local variable

	mov ecx, dword [ebp + 8] ; base index
.L1: ; outer cycle on string cycle
	mov dl, byte [ecx] ; char that is being considered
	test dl, dl
	jz .L1e
	mov dword [ebp - 4], ecx ; saving current base index
	mov ecx, dword [ebp + 8] ; loading string start
.L2:
	cmp dword [ebp - 4], ecx ; skipping current char
	je .L2i
	mov dh, byte [ecx]
	test dh, dh
	jz .L2en
	cmp dh, dl ; if equal, skipping current char
	je .L2e
.L2i:
	inc ecx
	jmp .L2
.L2en:
	push dword [ebp - 4] ; if not found equal, pushing adress to stack
.L2e:
	mov ecx, dword [ebp - 4]
	inc ecx
	jmp .L1
.L1e:

	mov eax, dword [ebp + 8] ; write index
	; start of stack of adresses
	mov ecx, ebp
	sub ecx, 8

.L3: ; writing all characters from stack into string
	cmp ecx, esp
	jb .L3e
	mov edx, dword [ecx]
	mov dl, byte [edx]
	mov byte [eax], dl
	inc eax
	sub ecx, 4
	jmp .L3
.L3e:

	mov byte [eax], 0 ; write null byte

	mov esp, ebp
	add esp, 4
	mov ebp, dword [ebp]
	ret
