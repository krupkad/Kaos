#include "common.h"
#include "types.h"
#include "vga.h"

extern void _hang();

void sti() {
	asm volatile("sti");
}

void cli() {
	asm volatile("cli");
}

u8 inb(u16 port) {
	unsigned char rv;
	asm volatile("inb %1, %0" : "=a" (rv) : "dN" (port));
	return rv;
}

void outb(u16 port, u8 data) {
	asm volatile("outb %1, %0" : : "dN" (port), "a" (data));
}

void wrmsr(u32 msr, u32 low, u32 high) {
	asm volatile("wrmsr" : : "c"(msr), "a"(low), "d"(high) : "memory");
}

u64 rdmsr(u32 msr) {
	u64 val;
	asm volatile("rdmsr" : "=A"(val) : "c"(msr));
	return val;
}

void panic(char *msg, ...) {
	va_list vl;
	va_start(vl, msg);
	vga_printf("kernel panic: ");
	vga_vprintf(msg, vl);
	va_end(vl);
	_hang();
}
