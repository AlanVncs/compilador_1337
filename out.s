.text
.globl teste
.type teste, @function
.comm k,4,4
teste:
	push 	%rbp
	mov 	%rsp, %rbp
	mov 	%rdi, k
	mov 	k, %r8
	mov 	$2, %r9
	cmp 	%r9, %r8
	je  	.LC3
	mov 	$0, %r8
	jmp 	.LC4
.LC3:
	mov 	$1, %r8
.LC4:
	cmp 	$1, %r8
	jz  	.LC0
	jmp 	.LC1
.LC0:
	mov 	k, %r8
	mov 	%r8, %rdi
	call 	printint
	mov 	%rax, %r9
	mov 	$1, %r8
	mov 	%r8, %rax
	jmp 	.LC2
.LC1:
	mov 	k, %r8
	mov 	%r8, %rdi
	call 	printint
	mov 	%rax, %r9
	mov 	$2, %r8
	mov 	%r8, %rax
.LC2:
	mov 	%rbp, %rsp
	pop 	%rbp
	ret
.text
.globl main
.type main, @function
.comm argc,4,4
main:
	push 	%rbp
	mov 	%rsp, %rbp
	mov 	%rdi, argc
.comm x,4,4
	mov 	$2, x
.comm y,4,4
	mov 	$3, y
.comm j,4,4
.comm l,4,4
	mov 	x, %r8
	mov 	%r8, %rdi
	call 	teste
	mov 	%rax, %r9
	mov 	j, %r8
	mov 	%r9, %r8
	mov 	%r8, j
	mov 	y, %r9
	mov 	%r9, %rdi
	call 	teste
	mov 	%rax, %r10
	mov 	l, %r9
	mov 	%r10, %r9
	mov 	%r9, l
	mov 	j, %r9
	mov 	%r9, %rdi
	call 	printint
	mov 	%rax, %r10
	mov 	l, %r9
	mov 	%r9, %rdi
	call 	printint
	mov 	%rax, %r10
	mov 	$0, %r9
	mov 	%r9, %rax
	mov 	%rbp, %rsp
	pop 	%rbp
	ret
