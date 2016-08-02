#include "syscall.h"
#include "isr.h"
#include "stdarg.h"

#define MSR_SYSENTER_CS 0x174
#define MSR_SYSENTER_ESP 0x175
#define MSR_SYSENTER_EIP 0x176
	
extern void scent_handler(struct regs_t *regs);
void kinit_syscall() {
	//wrmsr(MSR_SYSENTER_CS, 0x08, 0);
	//wrmsr(MSR_SYSENTER_ESP, tss.esp1, 0);
	//wrmsr(MSR_SYSENTER_EIP, (uaddr_t)sysenter_begin, 0);
	
	isr_sethandler(0x80, scent_handler);
}
