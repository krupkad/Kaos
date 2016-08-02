#ifndef MM_H
#define MM_H

/* kernel space will live here in virtual addressing */
#define KERNEL_OFFSET 0xC0000000
#define STACK_SIZE 4096
#define MACHINE_BITS 32

#if !defined(__ASSEMBLY__)

#include "types.h"

#define PHYS_TO_VIRT(addr) ((void *)(((uaddr_t)(addr)) + KERNEL_OFFSET))
#define VIRT_TO_PHYS(addr) ((uaddr_t)(((void *)(addr)) - KERNEL_OFFSET))

#define MM_ALIGN (1 << 0)
#define MM_ZERO (1 << 1)

void kinit_memory();
void *kmalloc(size_t size, int flags);
void kfree(void *ptr);
extern void *kimg_end;

#endif /* !defined(__ASSEMBLY__) */

#endif /* MM_H */
