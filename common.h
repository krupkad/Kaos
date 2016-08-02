#ifndef COMMON_H
#define COMMON_H

#include "types.h"

/* assert needs to quote line number */
#define _Q(x) __Q(x)
#define __Q(x) #x
#define assert(expr) \
	((expr) ? 0 : panic("assertion (" #expr ") failed, " __FILE__ ", "  _Q(__LINE__)))

#define NULL ((void *)0)

#define SET_FLAG(u, flag) do { (u) |= (flag); } while(0)
#define CLEAR_FLAG(u, flag) do { (u) &= ~(flag); } while(0)
#define TEST_FLAG(u, flag) ((u) & (flag))

void sti();
void cli();
unsigned char inb(u16 port);
void outb(u16 port, u8 data);
u64 rdmsr(u32 msr);
void wrmsr(u32 msr, u32 low, u32 high);
void panic(char *msg,...);

#endif /* COMMON_H */
