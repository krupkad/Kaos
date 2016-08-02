#include "timer.h"
#include "isr.h"
#include "vga.h"
#include "common.h"
#include "mm.h"
#include "sched.h"

u32 ticks = 0;

extern struct task_t *current_task;
static void timer_cback(struct regs_t *regs) {
	ticks++;
	task_switch();
}

void kinit_timer(u32 freq) {
	isr_sethandler(IRQ0, timer_cback);
	
	u32 divisor = 1193180 / freq;
	outb(0x43, 0x36);
	
	outb(0x40, divisor & 0xff);
	outb(0x40, divisor >> 8);
}
