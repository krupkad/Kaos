#include "common.h"
#include "vga.h"
#include "dtable.h"
#include "isr.h"
#include "page.h"
#include "timer.h"
#include "mboot.h"
#include "mm.h"
#include "sched.h"
#include "syscall.h"

extern pdir_t *kern_pdir;
extern struct task *cur_task;
extern void *kimg_end;
extern void kinit_usermode();
int p1(void *ptr);
int p2(void *ptr);
extern const void vdso_page;
void init();

int test_func() {
	vga_printf("test: %d 0x%p\n", 3, 0xdeadbeef);
	return 0xfeedbeef;
}

extern u32 kstack;
void kmain(struct mboot_info *minfo, u32 magic) {
	/* initialize VGA */
	vga_init();
	
	/* initialize mboot info */
	kinit_mboot(minfo, magic);
	
	struct mboot_mmap *mmap;
	for(mmap = bios_mmap;
		(unsigned long)mmap < minfo->mmap_addr + minfo->mmap_length;
		mmap = (struct mboot_mmap *)((unsigned long)mmap + mmap->size + sizeof(mmap->size))) {
			
		vga_printf("size = 0x%x, base_addr = 0x%x%x\n", mmap->size, mmap->base_addr_high, mmap->base_addr_low);
		vga_printf("length = 0x%x%x, type = %x\n\n", mmap->length_high, mmap->length_low, mmap->type);
	}
	
	/* say hello */
	vga_printf("Allo allo\n");
	vga_printf("%d MB RAM\n", phys_mem >> 20);
	vga_printf("vdso @ 0x%p\n", &kstack);
	
	/* initialize descriptor tables */
	kinit_dtable();
	
	/* initialize paging */
	kinit_memory();
	
	asm volatile("sti");
	kinit_timer(1000);
	
	kinit_sched();
	
	kinit_syscall();
	
	//int id = fork();
	
	//asm volatile("jmp _hang");
	
	//vga_printf("fork: 0x%x\n", id);
	
	//id = fork();
	//vga_printf("fork: 0x%x\n", id);
	//int i = 5;

	void *user_stack = kmalloc(STACK_SIZE, MM_ALIGN);
	kernel_set_stack(user_stack, STACK_SIZE);
	
	//kinit_usermode();
	
	//sys_clone(p1, 0x34, 0);
	//sys_clone(p2, 0x35, 0);
	
	//int r = do_syscall(2, 0x33, 0x35);
	//vga_printf("ret 0x%x\n", r);
	
	//int r = sys_test_func(0x33, 0x35);
	//vga_printf("ret 0x%x\n", r);
	//int id = sys_fork();
	
	int id = fork();
	vga_printf("fork: 0x%d\n", id);
	
	id = fork();
	vga_printf("fork: 0x%d\n", id);
	
	while(1);
}


int p1(void *ptr) {
	int i,j;
	for(i = 0; i < 50000000; i++) {};
	for(j = 0; j < 50000000; j++) {};
	vga_printf("p1 done 0x%p 0x%d 0x%d\n", ptr, i, j);
	return 4;
}

int p2(void *ptr) {
	vga_printf("p2 0x%p\n", ptr);
	return 5;
}
