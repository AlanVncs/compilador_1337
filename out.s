.text
.globl teste
.type teste, @function
.comm k,8,8
teste:
	nop
	push 	%rbp
	mov 	%rsp, %rbp
	mov 	%rdi, k(%rip)
	mov 	k(%rip), %r8
	mov 	$2, %r9
	cmp 	%r9, %r8
	je  	.LC3
	mov 	$0, %r8
	jmp 	.LC4
.LC3:
	mov 	$1, %r8
.LC4:
	cmp 	$0, %r8
	ja  	.LC0
	jmp 	.LC1
.LC0:
	mov 	k(%rip), %r8
	mov 	%r8, %rdi
	call 	printint
	mov 	%rax, %r9
	mov 	$1, %r8
	mov 	%r8, %rax
	mov 	%rbp, %rsp
	pop 	%rbp
	ret
	jmp 	.LC2
.LC1:
	mov 	k(%rip), %r8
	mov 	%r8, %rdi
	call 	printint
	mov 	%rax, %r9
	mov 	$2, %r8
	mov 	%r8, %rax
	mov 	%rbp, %rsp
	pop 	%rbp
	ret
.LC2:
.text
.globl main
.type main, @function
.comm argc,8,8
main:
	nop
	push 	%rbp
	mov 	%rsp, %rbp
	mov 	%rdi, argc(%rip)
.comm x,8,8
	mov 	$2, x(%rip)
.comm y,8,8
	mov 	$3, y(%rip)
.comm j,8,8
.comm l,8,8
	mov 	x(%rip), %r8
	mov 	%r8, %rdi
	call 	teste
	mov 	%rax, %r9
	mov 	j(%rip), %r8
	mov 	%r9, %r8
	mov 	%r8, j(%rip)
	mov 	y(%rip), %r9
	mov 	%r9, %rdi
	call 	teste
	mov 	%rax, %r10
	mov 	l(%rip), %r9
	mov 	%r10, %r9
	mov 	%r9, l(%rip)
	mov 	j(%rip), %r9
	mov 	%r9, %rdi
	call 	printint
	mov 	%rax, %r10
	mov 	l(%rip), %r9
	mov 	%r9, %rdi
	call 	printint
	mov 	%rax, %r10
	mov 	$0, %r9
	mov 	%r9, %rax
	mov 	%rbp, %rsp
	pop 	%rbp
	ret
