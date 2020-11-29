	.file	"main.c"
	.intel_syntax noprefix
	.text
	.globl	y
	.data
	.align 4
	.type	y, @object
	.size	y, 4
y:
	.long	6
	.text
	.globl	main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	mov	rbp, rsp
	.cfi_def_cfa_register 6
	mov	DWORD PTR -4[rbp], 4
	mov	eax, DWORD PTR y[rip]
	add	DWORD PTR -4[rbp], eax
	mov	eax, 0
	pop	rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (GNU) 10.2.0"
	.section	.note.GNU-stack,"",@progbits


mov $1, %rdi
mov msg, %rsi
mov 5, %rdx
mov 1, %rax