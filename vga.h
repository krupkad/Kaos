#ifndef VGA_H
#define VGA_H

#include "types.h"
#include "stdarg.h"

void vga_scroll();
void vga_mvcur();
void vga_clear();
void vga_putch(unsigned char c);
void vga_puts(const char *str);
void vga_setattr(u8 front, u8 back);
void vga_init();
int vga_printnum(u32 num, u8 base);
int vga_printf(char *fmt, ...);
int vga_vprintf(char *fmt, va_list va);

#endif /* VGA_H */
