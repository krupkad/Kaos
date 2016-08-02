.macro isr_noerr n
.global isr\n
isr\n:
	cli
	push $0
	push $\n
	jmp isr_stub
.endm

.macro isr_err n
.global isr\n
isr\n:
	cli
	push $\n
	jmp isr_stub
.endm

.macro irq_map m, n
.global irq\m
irq\m:
	cli
	push $0
	push $\n
	jmp irq_stub
.endm

isr_noerr 0
isr_noerr 1
isr_noerr 2
isr_noerr 3
isr_noerr 4
isr_noerr 5
isr_noerr 6
isr_noerr 7
isr_err 8
isr_noerr 9
isr_err 10
isr_err 11
isr_err 12
isr_err 13
isr_err 14
isr_noerr 15
isr_noerr 16
isr_noerr 17
isr_noerr 18
isr_noerr 19
isr_noerr 20
isr_noerr 21
isr_noerr 22
isr_noerr 23
isr_noerr 24
isr_noerr 25
isr_noerr 26
isr_noerr 27
isr_noerr 28
isr_noerr 29
isr_noerr 30
isr_noerr 31
isr_noerr 128

irq_map 0, 32
irq_map 1, 33
irq_map 2, 34
irq_map 3, 35
irq_map 4, 36
irq_map 5, 37
irq_map 6, 38
irq_map 7, 39
irq_map 8, 40
irq_map 9, 41
irq_map 10, 42
irq_map 11, 43
irq_map 12, 44
irq_map 13, 45
irq_map 14, 46
irq_map 15, 47

isr_stub:
	/* save all general registers */
	pusha
	
	/* save the current data segment */
	mov %ds,%ax
	push %eax
	
	/* use kernel data segment */
	mov $0x10,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs
	
	call isr_handler
	
	/* restore data segment */
	pop %eax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs
	
	/* restore general registers */
	popa
	
	/* pop isr information (error code, interrupt number) */
	add $8,%esp
	
	sti
	iret

irq_stub:
	pusha
	
	mov %ds,%ax
	push %eax
	
	mov $0x10,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs
	
	call irq_handler
	
	pop %ebx
	mov %bx,%ds
	mov %bx,%es
	mov %bx,%fs
	mov %bx,%gs
	
	popa
	add $8,%esp
	sti
	iret
