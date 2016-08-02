#include "vga.h"
#include "common.h"
#include "types.h"
#include "string.h"
#include "stdarg.h"
#include "page.h"
#include "mm.h"

static struct {
	u16 *text;
	u8 attr;
	u16 cur_x, cur_y;
} vga;

void vga_scroll() {
	u16 blank = 0x20 | (vga.attr << 8);
	
	if(vga.cur_y >= 25) {
		int i;
		for(i = 0; i < 24 * 80; i++) {
			vga.text[i] = vga.text[i + 80];
		}
		for(i = 24 * 80; i < 25 * 80; i++) {
			vga.text[i] = blank;
		}
		vga.cur_y = 24;
	}
}

void vga_mvcur() {
	u16 pos = vga.cur_y * 80 + vga.cur_x;
	outb(0x3d4, 14);
	outb(0x3d5, pos >> 8);
	outb(0x3d4, 15);
	outb(0x3d5, pos);
}

void vga_clear() {
	u16 blank = 0x20 | (vga.attr << 8);
	int i;
	for(i = 0; i < 80 * 25; i++) {
		vga.text[i] = blank;
	}
	
	vga.cur_x = 0;
	vga.cur_y = 0;
	vga_mvcur();
}

void vga_putch(unsigned char c) {
	u16 *pos;
	u16 attr = vga.attr << 8;
	
	if(c == 0x08 && vga.cur_x) {
		--vga.cur_x;
	} else if(c == 0x09) {
		vga.cur_x = (vga.cur_x + 8) & ~(8 - 1);
	} else if(c == '\r') {
		vga.cur_x = 0;
	} else if(c == '\n') {
		vga.cur_x = 0;
		++vga.cur_y;
	} else if(c >= ' ') {
		pos = vga.text + vga.cur_y * 80 + vga.cur_x;
		*pos = c | attr;
		++vga.cur_x;
	}
	
	if(vga.cur_x >= 80) {
		vga.cur_x = 0;
		++vga.cur_y;
	}
	
	vga_scroll();
	vga_mvcur();
}

void vga_puts(const char *str) {
	int i;
	for(i = 0; i < strlen(str); i++) {
		vga_putch(str[i]);
	}
}

void vga_setattr(u8 front, u8 back) {
	vga.attr = (back << 4) | (front & 0x0f);
}

void vga_init() {
	vga.text = (unsigned short *)(0xb8000 + KERNEL_OFFSET);
	vga_setattr(7,0);
	vga_clear();
}

static char *strrev(char *str) {
	char *p1, *p2;

	if (!str || !*str)
		return str;

	for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}

	return str;
}


int vga_printnum(u32 num, u8 base) {
	static char digits[] = "0123456789abcdef";
	
	int i = 0;
	char buf[80];
	do {
		buf[i++] = digits[num % base];
	} while((num /= base) > 0);
	
	buf[i] = 0;
	strrev(buf);
	
	vga_puts(buf);
	return i;
}

int vga_printf(char *fmt, ...) {
	va_list vl;
	va_start(vl, fmt);
	int x = vga_vprintf(fmt, vl);
	va_end(vl);
	return x;
}

int vga_vprintf(char *fmt, va_list va) {
	int cnt = 0;
	while(*fmt) {
		if(*fmt != '%') {
			vga_putch(*fmt);
			++cnt;
		} else {
			++fmt;
			if(!*fmt) {
				break;
			}
			
			switch(*fmt) {
				case 'd':
				case 'i':
					cnt += vga_printnum(va_arg(va, int), 10);
					break;
				case 'p':
				case 'x':
					cnt += vga_printnum(va_arg(va, long), 16);
					break;
				case '%':
					vga_putch('%');
					++cnt;
					break;
				default:
					break;
			}
		}
		++fmt;
	}
	
	return cnt;
}
