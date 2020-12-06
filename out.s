.text
.globl main
.type main, @function
.comm argc,8,8
main:
	push 	%rbp
	mov 	%rsp, %rbp
	mov 	%rdi, argc(%rip)
	mov 	$0, %r8
	mov 	$1, %r9
	or 	%r9, %r8
	je 	.LC0
	mov 	$1, %r8
	jmp .LC1
.LC0:
	mov 	$0, %r8
.LC1:
	mov 	%r8, %rdi
	call 	printint
	mov 	%rax, %r9
	mov 	$0, %r8
	mov 	%r8, %rax
	mov 	%rbp, %rsp
	pop 	%rbp
	ret
