.text
.globl main
.type main, @function
.comm argc,4,4
main:
	push 	%rbp
	mov 	%rsp, %rbp
	mov 	%rdi, argc(%rip)
.comm x,4,4
	mov 	$1, x(%rip)
	mov 	$1, %r8
	mov 	$2, %r9
	test 	%r8, %r8
	je 	.LC0
	test 	%r9, %r9
	je 	.LC0
	mov 	$1, %r8
	jmp 	.LC1
.LC0:
	mov 	$0, %r8
.LC1:
	mov 	x(%rip), %r9
	mov 	%r8, %r9
	mov 	%r9, x(%rip)
	mov 	x(%rip), %r8
	mov 	%r8, %rdi
	call 	printint
	mov 	%rax, %r9
	mov 	$0, %r8
	mov 	%r8, %rax
	mov 	%rbp, %rsp
	pop 	%rbp
	ret
