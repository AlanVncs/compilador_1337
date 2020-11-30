.text
.globl main
.type main, @function
.comm argc,4,4
main:
	push 	%rbp
	mov 	%rsp, %rbp
	mov 	%rdi, argc(%rip)
	mov 	$1, %r8
	mov 	$2, %r9
	cmp 	 %r9, %r8
	je  	.LC0
	mov 	$0, %r8
	jmp 	.LC1
.LC0:
	mov 	$1, %r8
.LC1:
	mov 	%r8, %rdi
	call 	printint
	mov 	%rax, %r9
	mov 	$2, %r8
	mov 	$2, %r9
	cmp 	 %r9, %r8
	je  	.LC2
	mov 	$0, %r8
	jmp 	.LC3
.LC2:
	mov 	$1, %r8
.LC3:
	mov 	%r8, %rdi
	call 	printint
	mov 	%rax, %r9
	mov 	$3, %r8
	mov 	$2, %r9
	cmp 	 %r9, %r8
	je  	.LC4
	mov 	$0, %r8
	jmp 	.LC5
.LC4:
	mov 	$1, %r8
.LC5:
	mov 	%r8, %rdi
	call 	printint
	mov 	%rax, %r9
	mov 	$0, %r8
	mov 	%r8, %rax
	mov 	%rbp, %rsp
	pop 	%rbp
	ret
