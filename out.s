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
	mov 	$1, %r8
	mov 	%r8, %rax
	jmp 	.LC2
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
	mov 	%rdi, argc(%rip)
.comm f,4,4
	mov 	$5, %r8
	mov 	%r8, %rdi
	call 	fatorial
	mov 	%rax, %r9
	mov 	f(%rip), %r8
	mov 	%r9, %r8
	mov 	%r8, f(%rip)
	mov 	f(%rip), %r9
	mov 	%r9, %rdi
	call 	printint
	mov 	%rax, %r10
	mov 	$0, %r9
	mov 	%r9, %rax
	mov 	%rbp, %rsp
	pop 	%rbp
	ret
