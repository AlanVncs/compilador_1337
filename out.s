.text
.globl teste
.type teste, @function
.comm a,4,4
teste:
	push 	%rbp
	mov 	%rsp, %rbp
	mov 	%rdi, a(%rip)
	mov 	a(%rip), %r8
	mov 	$1, %r9
	add 	%r9, %r8
	mov 	%r8, %rax
	mov 	%rbp, %rsp
	pop 	%rbp
	ret
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
	mov 	%rax, %r9
	mov 	x(%rip), %r8
	mov 	$5, %r9
	sub 	%r9, %r8
	mov 	x(%rip), %r9
	mov 	%r8, %r9
	mov 	%r9, x(%rip)
	mov 	x(%rip), %r8
	mov 	%r8, %rdi
	call 	printint
	mov 	%rax, %r9
	mov 	$2, %r8
	mov 	%r8, %rdi
	call 	teste
	mov 	%rax, %r9
	mov 	x(%rip), %r8
	mov 	%r9, %r8
	mov 	%r8, x(%rip)
	mov 	x(%rip), %r9
	mov 	%r9, %rdi
	call 	printint
	mov 	%rax, %r10
	mov 	$0, %r9
	mov 	%r9, %rax
	mov 	%rbp, %rsp
	pop 	%rbp
	ret
