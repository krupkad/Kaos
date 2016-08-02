#include "isr.h"
#include "types.h"
#include "vga.h"
#include "common.h"

static isr_t isr[256];

void isr_sethandler(u8 n, isr_t handler) {
	isr[n] = handler;
}

void isr_handler(struct regs_t regs) {
	if(isr[regs.int_no] != 0) {
		isr_t handler = isr[regs.int_no];
		handler(&regs);
	}
}

void irq_handler(struct regs_t regs) {
	if(regs.int_no >= 40) {
		outb(0xa0, 0x20);
	}
	outb(0x20, 0x20);
	
	if(isr[regs.int_no] != 0) {
		isr_t handler = isr[regs.int_no];
		handler(&regs);
	}
}
