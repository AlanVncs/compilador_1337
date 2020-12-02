	.file	"main.c"
	.text
	.globl	teste
	.type	teste, @function
teste:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	%edi, -4(%rbp)
	cmpl	$2, -4(%rbp)
	jne	.L2
	movl	-4(%rbp), %eax
	movl	%eax, %edi
	call	printint@PLT
	movl	$1, %eax
	jmp	.L3
.L2:
	movl	-4(%rbp), %eax
	movl	%eax, %edi
	call	printint@PLT
	movl	$2, %eax
.L3:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	teste, .-teste
	.globl	main
	.type	main, @function
main:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, -20(%rbp)
	movl	$2, -16(%rbp)
	movl	$3, -12(%rbp)
	movl	-16(%rbp), %eax
	movl	%eax, %edi
	call	teste
	movl	%eax, -8(%rbp)
	movl	-12(%rbp), %eax
	movl	%eax, %edi
	call	teste
	movl	%eax, -4(%rbp)
	movl	-8(%rbp), %eax
	movl	%eax, %edi
	call	printint@PLT
	movl	-4(%rbp), %eax
	movl	%eax, %edi
	call	printint@PLT
	movl	$0, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	main, .-main
	.ident	"GCC: (GNU) 10.2.0"
	.section	.note.GNU-stack,"",@progbits
