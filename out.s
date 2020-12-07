.text
.globl main
.type main, @function
.comm argc,8,8
main:
	push 	%rbp
	mov 	%rsp, %rbp
	mov 	%rdi, argc(%rip)
.comm i,8,8
	mov 	$5, i(%rip)
.LC0:
	mov 	i(%rip), %r8
	mov 	$1, %r9
	sub 	%r9, %r8
	mov 	i(%rip), %r9
	mov 	%r8, %r9
	mov 	%r9, i(%rip)
	mov 	$1, %r8
	add 	%r8, %r9
	mov 	%r9, %rdi
	call 	printint
	mov 	%rax, %r8
	mov 	i(%rip), %r9
	cmp 	$0, %r9
	ja  	.LC0
	mov 	i(%rip), %r9
	mov 	%r9, %rdi
	call 	printint
	mov 	%rax, %r10
	mov 	$0, %r9
	mov 	%r9, %rax
	mov 	%rbp, %rsp
	pop 	%rbp
	ret
