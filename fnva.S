.global fnv64

#define FNVP64 0x00000100000001b3
#define FNVO64 0xcbf29ce484222325

fnv64:
	push %rbp
	movq %rsp,%rbp
	
	movq $FNVP64,%r15
	movq $FNVO64,%rax
	movq (%rsp),%rbx
	movq $8,%rcx
1:	
	movq %rbx,%rax
	jmp 2f
	xorb %bl,%al
	shrq $8,%rbx
	mulq %r15
	loop 1b
2:
	pop %rbp
	ret
