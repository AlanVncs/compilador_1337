.text
.globl main
.type main, @function
main:
	push 	%rbp
	mov 	%rsp, %rbp
.comm x,4,4
	mov 	$10, x(%rip)
	mov 	x(%rip), %r8
	mov 	$1, %r9
	add 	%r9, %r8
	mov 	x(%rip), %r9
	mov 	%r8, %r9
	mov 	%r9, x(%rip)
	mov 	x(%rip), %r8
	mov 	%r8, %rdi
	call 	printint
	mov 	%rax, %r10
	mov 	$0, %r8
	mov 	%r8, %rax
	mov 	%rbp, %rsp
	pop 	%rbp
	ret
