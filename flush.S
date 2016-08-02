.global gdt_flush
.global idt_flush
.global tss_flush

idt_flush:
	mov 4(%esp), %eax
	lidt (%eax)
	ret

gdt_flush:
	mov 4(%esp), %eax
	lgdt (%eax)
	
	mov $0x10,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs
	mov %ax,%ss
	jmp $0x08,$stub

tss_flush:
	mov $0x2b, %ax
	ltr %ax
	ret

stub:
	ret
