#include "dtable.h"
#include "common.h"
#include "string.h"
#include "isr.h"

struct gdt_entry {
	u16 lim_low;
	u16 base_low;
	u8 base_mid;
	u8 access;
	u8 gran;
	u8 base_hi;
} __attribute__((packed));

struct gdt_ptr {
	u16 limit;
	u32 base;
} __attribute__((packed));

struct idt_entry {
	u16 base_low;
	u16 sel;
	u8 zero;
	u8 flags;
	u16 base_hi;
} __attribute__((packed));

struct idt_ptr {
	u16 limit;
	u32 base;
} __attribute__((packed));


struct tss_entry {
	u32 prev;
	u32 esp0;
	u32 ss0;        
	u32 esp1;       
	u32 ss1;
	u32 esp2;
	u32 ss2;
	u32 cr3;
	u32 eip;
	u32 eflags;
	u32 eax;
	u32 ecx;
	u32 edx;
	u32 ebx;
	u32 esp;
	u32 ebp;
	u32 esi;
	u32 edi;
	u32 es;         
	u32 cs;         
	u32 ss;         
	u32 ds;         
	u32 fs;         
	u32 gs;         
	u32 ldt;        
	u16 trap;
	u16 iomap_base;
} __attribute__((packed));

static struct gdt_entry gdt[6];
static struct gdt_ptr gptr;
static struct idt_entry idt[256];
static struct idt_ptr iptr;
static struct tss_entry tss;

extern void gdt_flush(u32);
extern void idt_flush(u32);
extern void tss_flush();

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr128();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

static void gdt_gate(s32 num, u32 base, u32 lim, u8 acc, u8 gran) {
	gdt[num].base_low = base & 0xffff;
	gdt[num].base_mid = (base >> 16) & 0xff;
	gdt[num].base_hi = (base >> 24) & 0xff;
	
	gdt[num].lim_low = lim & 0xffff;
	
	gdt[num].gran = (lim >> 16) & 0x0f;
	gdt[num].gran |= gran & 0xf0;
	
	gdt[num].access = acc;
}

static void idt_gate(u8 num, u32 base, u16 sel, u8 flags) {
	idt[num].base_low = base & 0xffff;
	idt[num].base_hi = (base >> 16) & 0xffff;
	
	idt[num].sel = sel;
	idt[num].zero = 0;
	
	idt[num].flags = flags;
}

static void write_tss(s32 num, u16 ss0, u32 esp0) {
	u32 base = (u32)&tss;
	u32 limit = base + sizeof(tss);
	
	gdt_gate(num, base, limit, 0xe9, 0x00);
	memset(&tss, 0, sizeof(tss));
	tss.ss0 = ss0;
	tss.esp0 = esp0;
	tss.cs = 0x0b;
	tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;
}

void kernel_set_stack(void *stack, size_t size) {
	tss.esp0 = (uaddr_t)stack;// + size;
}

extern u32 kstack;
static void kinit_gdt() {
	gptr.limit = sizeof(*gdt) * 6 - 1;
	gptr.base = (u32)&gdt;
	
	gdt_gate(0,0,0,0,0);
	gdt_gate(1,0,0xffffffff,0x9a,0xcf);
	gdt_gate(2,0,0xffffffff,0x92,0xcf);
	gdt_gate(3,0,0xffffffff,0xfa,0xcf);
	gdt_gate(4,0,0xffffffff,0xf2,0xcf);
	write_tss(5, 0x10, kstack);
	
	gdt_flush((u32)&gptr);
	tss_flush();
}

static void kinit_idt() {
	iptr.limit = 256 * sizeof(*idt) - 1;
	iptr.base = (u32)&idt;
	
	memset(&idt, 0, sizeof(*idt) * 256);
	
	outb(0x20, 0x11);
	outb(0xa0, 0x11);
	outb(0x21, 0x20);
	outb(0xa1, 0x28);
	outb(0x21, 0x04);
	outb(0xa1, 0x02);
	outb(0x21, 0x01);
	outb(0xa1, 0x01);
	outb(0x21, 0x0);
	outb(0xa1, 0x0);
	
	idt_gate(0, (u32)isr0, 0x08, 0x8e);
	idt_gate(1, (u32)isr1, 0x08, 0x8e);
	idt_gate(2, (u32)isr2, 0x08, 0x8e);
	idt_gate(3, (u32)isr3, 0x08, 0x8e);
	idt_gate(4, (u32)isr4, 0x08, 0x8e);
	idt_gate(5, (u32)isr5, 0x08, 0x8e);
	idt_gate(6, (u32)isr6, 0x08, 0x8e);
	idt_gate(7, (u32)isr7, 0x08, 0x8e);
	idt_gate(8, (u32)isr8, 0x08, 0x8e);
	idt_gate(9, (u32)isr9, 0x08, 0x8e);
	idt_gate(10, (u32)isr10, 0x08, 0x8e);
	idt_gate(11, (u32)isr11, 0x08, 0x8e);
	idt_gate(12, (u32)isr12, 0x08, 0x8e);
	idt_gate(13, (u32)isr13, 0x08, 0x8e);
	idt_gate(14, (u32)isr14, 0x08, 0x8e);
	idt_gate(15, (u32)isr15, 0x08, 0x8e);
	idt_gate(16, (u32)isr16, 0x08, 0x8e);
	idt_gate(17, (u32)isr17, 0x08, 0x8e);
	idt_gate(18, (u32)isr18, 0x08, 0x8e);
	idt_gate(19, (u32)isr19, 0x08, 0x8e);
	idt_gate(20, (u32)isr20, 0x08, 0x8e);
	idt_gate(21, (u32)isr21, 0x08, 0x8e);
	idt_gate(22, (u32)isr22, 0x08, 0x8e);
	idt_gate(23, (u32)isr23, 0x08, 0x8e);
	idt_gate(24, (u32)isr24, 0x08, 0x8e);
	idt_gate(25, (u32)isr25, 0x08, 0x8e);
	idt_gate(26, (u32)isr26, 0x08, 0x8e);
	idt_gate(27, (u32)isr27, 0x08, 0x8e);
	idt_gate(28, (u32)isr28, 0x08, 0x8e);
	idt_gate(29, (u32)isr29, 0x08, 0x8e);
	idt_gate(30, (u32)isr30, 0x08, 0x8e);
	idt_gate(31, (u32)isr31, 0x08, 0x8e);
	
	/* 0xee to allow ring 3 */
	idt_gate(128, (u32)isr128, 0x08, 0xee);
	
	idt_gate(32, (u32)irq0, 0x08, 0x8e);
	idt_gate(33, (u32)irq1, 0x08, 0x8e);
	idt_gate(34, (u32)irq2, 0x08, 0x8e);
	idt_gate(35, (u32)irq3, 0x08, 0x8e);
	idt_gate(36, (u32)irq4, 0x08, 0x8e);
	idt_gate(37, (u32)irq5, 0x08, 0x8e);
	idt_gate(38, (u32)irq6, 0x08, 0x8e);
	idt_gate(39, (u32)irq7, 0x08, 0x8e);
	idt_gate(40, (u32)irq8, 0x08, 0x8e);
	idt_gate(41, (u32)irq9, 0x08, 0x8e);
	idt_gate(42, (u32)irq10, 0x08, 0x8e);
	idt_gate(43, (u32)irq11, 0x08, 0x8e);
	idt_gate(44, (u32)irq12, 0x08, 0x8e);
	idt_gate(45, (u32)irq13, 0x08, 0x8e);
	idt_gate(46, (u32)irq14, 0x08, 0x8e);
	idt_gate(47, (u32)irq15, 0x08, 0x8e);
	
	idt_flush((u32)&iptr);
}


#define GPF_EXTERNAL 0x01

static void gpf_handler(struct regs_t *regs) {
	u16 gpf_index = (u16)(regs->err_code & ~0x07) >> 3;
	u8 gpf_table = (regs->err_code & 0x06) >> 1;
	
	if(TEST_FLAG(regs->err_code, GPF_EXTERNAL)) {
		vga_printf("(external)");
	} else {
		vga_printf("(internal)");
	}
	
	switch(gpf_table) {
		case 0x00:
			vga_printf("(gdt)");
			break;
		case 0x10:
			vga_printf("(ldt)");
			break;
		case 0x01:
		case 0x11:
			vga_printf("(idt)");
			break;
		default:
			vga_printf("()");
	}
	vga_printf(" at index 0x%x\n", gpf_index);
	
	vga_printf("\ncs:eip = 0x%p:0x%p\n", regs->cs, regs->eip);
	panic("gpfault");
}

void kinit_dtable() {
	kinit_gdt();
	kinit_idt();
	isr_sethandler(0x0d, gpf_handler);
	vga_printf("tss: 0x%p\n", &tss);
}
