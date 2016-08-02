#include "mm.h"

.global switch_stub
.global fork

fork:
	pop %ecx
	mov %esp,%edx
	push %ecx
	
	push $1
	push %edx
	push %ecx
	mov $clone,%ebx
	call *%ebx
	add $0x0c,%esp
	ret
	
switch_stub:
	/* save context */
	mov 4(%esp), %eax
	mov %ebp, 4(%eax)
	mov %esp, 8(%eax)
	
	/* use new context */
	mov 8(%esp), %eax
	mov 4(%eax), %ebp
	mov 8(%eax), %esp
	
	/* enable interrupts after switch */
	pushf
	pop %ecx
	or $0x200,%ecx
	push %ecx
	popf
	
	/* load new page directory */
	mov 12(%eax), %ecx
	sub $KERNEL_OFFSET, %ecx
	mov %ecx, %cr3
	
	mov $0,%eax
	
	ret
