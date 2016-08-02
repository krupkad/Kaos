#include "mm.h"

.global _start
.global _hang
.global kern_pdir
.global get_eip
.global kinit_usermode
.global kimg_end
.global vdso_page
.global kstack
.global do_syscall

#define MB_ALIGN (1<<0)
#define MB_INFO (1<<1)
#define MB_FLAGS (MB_ALIGN | MB_INFO)
#define MB_MAGIC (0x1BADB002)
#define MB_CHKSUM (-(MB_MAGIC + MB_FLAGS))

.text

/* multiboot header */
.align 4
multiboot:
	.long MB_MAGIC
	.long MB_FLAGS
	.long MB_CHKSUM

_start:
	/* save the multiboot magic number, we're gonna trash %eax */
	mov %eax,%edx
	
	/* initialize two tables of identity mappings, which will
	 * be unmapped when we set up paging for real */
	mov $pg0 - KERNEL_OFFSET, %edi
	mov $0x07, %eax
1:
	stosl
	add $0x1000, %eax
	cmp $pg_stop - KERNEL_OFFSET, %edi
	jne 1b

	/* turn on paging */
	mov $init_pdir - KERNEL_OFFSET, %eax
	mov %eax, %cr3
	
	mov %cr0, %eax
	or $0x80000000, %eax
	mov %eax, %cr0

	/* jump to real startup */
	lea 2f,%eax
	jmp *%eax
2:
	/* initialize the stack pointer */
	mov $kstack,%esp
	mov $kstack,%ebp
	
	/* push multiboot magic number and header*/
	push %edx
	push %ebx
	
	/* disable interrupts, start the kernel, and derp if it finishes */
	cli
	call kmain
_hang:
	cli
	hlt

do_syscall:
	mov 4(%esp),%eax
	
	/* provide address of the arguments */
	lea 8(%esp),%ebx
	
	int $0x80
	ret

kinit_usermode:
	cli
	
	/* get and restore the next address to be executed */
	pop %ecx
	push %ecx
	
	mov $0x23, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	
	mov %esp, %eax
	push $0x23
	push %eax
	
	pushf
	pop %eax
	or $0x200, %eax
	push %eax
	
	push $0x1b
	push %ecx
	iret

kimg_end:
	.long _kimg_end

/* (virtual) pointer to the kernel page directory, used by almost everything */
kern_pdir:
	.long init_pdir

.data

/* We need to set up a couple tables */
init_pdir:
	.long pg0 - KERNEL_OFFSET + 0x07
	.long pg1 - KERNEL_OFFSET + 0x07
	.fill 766,4,0
	.long pg0 - KERNEL_OFFSET + 0x07
	.long pg1 - KERNEL_OFFSET + 0x07
	.fill 254,4,0

pg0:
	.fill 1024,4,0

pg1:
	.fill 1024,4,0

pg_stop:
	.fill 1024,4,0

/* claim one page for the vdso */
vdso_page:
	.fill 1024,4,0

.bss

/* we also need a stack, multiboot won't give us one =( */
.align 32
kstack:
	.fill 1024,4,0
