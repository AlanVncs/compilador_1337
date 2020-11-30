.text
.globl fatorial
.type fatorial, @function
.comm n,4,4
fatorial:
	push 	%rbp
	mov 	%rsp, %rbp
	mov 	%rdi, n(%rip)
	mov 	n(%rip), %r8
	mov 	$1, %r9
	cmp 	%r9, %r8
	je  	.LC2
	mov 	$0, %r8
	jmp 	.LC3
.LC2:
	mov 	$1, %r8
.LC3:
	cmp 	$1, %r8
	jz  	.LC0
	jmp 	.LC1
.LC0:
	mov 	$1, %r8
	mov 	%r8, %rax
.LC1:
	mov 	n(%rip), %r8
	mov 	n(%rip), %r9
	mov 	$1, %r10
	sub 	%r10, %r9
	mov 	%r9, %rdi
	call 	fatorial
	mov 	%rax, %r10
	imul 	%r10, %r8
	mov 	%r8, %rax
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
	mov 	%rdi, argc(%rip)
.comm x,4,4
	mov 	$0, x(%rip)
	mov 	x(%rip), %r8
	mov 	%r8, %rdi
	call 	printint
	mov 	%rax, %r9
	mov 	$0, %r8
	mov 	%r8, %rax
	mov 	%rbp, %rsp
	pop 	%rbp
	ret
