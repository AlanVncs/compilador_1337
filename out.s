.text
.globl main
.type main, @function
.comm argc,8,8
main:
	push 	%rbp
	mov 	%rsp, %rbp
	mov 	%rdi, argc(%rip)
.comm i,8,8
	mov 	$0, i(%rip)
.LC0:
	mov 	i(%rip), %r8
	mov 	$1, %r9
	add 	%r9, %r8
	mov 	i(%rip), %r9
	mov 	%r8, %r9
	mov 	%r9, i(%rip)
	mov 	$1, %r8
	sub 	%r8, %r9
	mov 	%r9, %rdi
	call 	printint
	mov 	%rax, %r8
	mov 	i(%rip), %r9
	mov 	$5, %r10
	cmp 	%r10, %r9
	jb  	.LC1
	mov 	$0, %r9
	jmp 	.LC2
.LC1:
	mov 	$1, %r9
.LC2:
	cmp 	$1, %r9
	jz  	.LC0
	mov 	i(%rip), %r9
	mov 	%r9, %rdi
	call 	printint
	mov 	%rax, %r10
	mov 	$0, %r9
	mov 	%r9, %rax
	mov 	%rbp, %rsp
	pop 	%rbp
	ret
