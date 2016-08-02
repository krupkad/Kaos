#include "page.h"
#include "mm.h"
#include "common.h"
#include "vga.h"
#include "string.h"
#include "mboot.h"
#include "isr.h"
#include "sched.h"

/* the kernel's page directory */
extern pdir_t *kern_pdir;
pdir_t *cur_pdir;

static pte_t mkpte(uaddr_t paddr, u32 flags) {
	pte_t page = paddr;
	SET_FLAG(page, flags);
	
	return page;
}

/* the process for a general page table walk, using our system:
 * 1) get _pteN_t from table N via PTEn_INDEX()
 * 2a) translate it into the next _pteN_t * via PTEn_MASK
 * 2b) perform various checks (if necessary) on the _pteN_t
 * 3) repeat until you get the necessary page */

/* Retrieves a page entry in a directory, NULL if its not there */
static pte_t *page_pte(void *vaddr, pdir_t *dir, int force) {
	_pte0_t *pte0 = &dir[PTE0_INDEX(vaddr)];
	if(!TEST_FLAG(*pte0, PTE_PRESENT)) {
		if(force) {
			_pte1_t *tmp = kmalloc(PTE1_UNITS * sizeof(_pte1_t),  MM_ALIGN);
			uaddr_t phys = VIRT_TO_PHYS(tmp);
			memset(tmp, 0, PTE1_UNITS * sizeof(_pte1_t));
			*pte0 = mkpte(phys, PTE_PRESENT | PTE_WRITE);
		} else {
			return NULL;
		}
	}
	_pte1_t *tbl = (_pte1_t *)PHYS_TO_VIRT((uaddr_t)*pte0 & ~PAGE_MASK);
	
	_pte1_t *pte1 = &tbl[PTE1_INDEX(vaddr)];
	
	return pte1;
}

/* flags for handling page fault error code */
#define PF_PRESENT (1 << 0)
#define PF_WRITE (1 << 1)
#define PF_USER (1 << 2)
#define PF_RESERVED (1 << 3)

void page_fault(struct regs_t *regs) {
	void *bad_addr;
	asm volatile("mov %%cr2, %0" : "=r" (bad_addr));
	
	if(TEST_FLAG(regs->err_code, PF_PRESENT)) {
		vga_printf("(present)");
	} else {
		vga_printf("(missing)");
	}
	
	if(TEST_FLAG(regs->err_code, PF_WRITE)) {
		vga_printf("(write)");
	} else {
		vga_printf("(read)");
	}
	
	if(TEST_FLAG(regs->err_code, PF_USER)) {
		vga_printf("(user)");
	} else {
		vga_printf("(kernel)");
	}
	
	if(TEST_FLAG(regs->err_code, PF_RESERVED)) {
		vga_printf("(reserved)");
	}
	
	
	vga_printf(" at 0x%p\n", bad_addr);
	
	pte_t *pte = page_pte(bad_addr, cur_pdir, 0);
	if(!pte) {
		vga_printf("mapping: none\n");
	} else {
		vga_printf("mapping: 0x%x\n", *pte);
		if(TEST_FLAG(*pte, PTE_COW)) {
			vga_printf("cowing 0x%p\n", bad_addr);
			CLEAR_FLAG(*pte, PTE_COW);
		}
	}
	
	extern struct task_t *current_task;
	vga_printf("eip: 0x%p 0x%p\n", regs->eip, current_task->id);
	panic("page fault");
}

extern const void vdso_page;
extern u32 kstack;
void kinit_memory() {
	/* register page fault handler */
	isr_sethandler(0x0e, page_fault);
	
	/* set up the kernel's page tables by mapping the higher half */
	uaddr_t i;
	for(i = 0; (i < phys_mem) && (i < ~KERNEL_OFFSET); i += PAGE_SIZE) {
		page_map(PHYS_TO_VIRT(i), i, kern_pdir, PTE_PRESENT | PTE_WRITE);
	}
	
	/* unmap the initial identity map */
	kern_pdir[0] = 0;
	kern_pdir[1] = 0;
	
	/* reload page directory */
	cur_pdir = clone_pdir(kern_pdir);
	//page_map((void *)(KERNEL_OFFSET - PAGE_SIZE), VIRT_TO_PHYS(&vdso_page), cur_pdir, PTE_USER | PTE_PRESENT);
	page_chdir(cur_pdir);
	
	kernel_set_stack(&kstack, STACK_SIZE);
}

void page_chdir(pdir_t *dir) {
	cur_pdir = dir;
	uaddr_t addr = VIRT_TO_PHYS(dir);
	asm volatile("mov %0, %%cr3" : : "r" (addr));
	u32 cr0;
	asm volatile("mov %%cr0, %0" : "=r" (cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0" : : "r" (cr0));
}

pte_t *page_map(void *vaddr, uaddr_t addr, pdir_t *dir, int pte_flags) {
	pte_t *pte = page_pte(vaddr, dir, 1);
	if(!pte) {
		/* the MM_MKPTE flag should guarantee page_pte never returns NULL.
		 * if it does, something bad happened and a seizure is warranted. */
		panic("active page_pte returned NULL for address 0x%p\n", vaddr);
	}
	*pte = mkpte(addr, pte_flags);
	return pte;
}


static uaddr_t clone_pte1(_pte1_t *src) {
	_pte1_t *tbl = kmalloc(PTE1_UNITS * sizeof(_pte1_t), MM_ALIGN);
	uaddr_t phys = VIRT_TO_PHYS(tbl);
	memset(tbl, 0, PTE1_UNITS * sizeof(_pte0_t));
	
	int i;
	for(i = 0; i < PTE1_UNITS; i++) {
		uaddr_t base = src[i] & ~PAGE_MASK;
		uaddr_t flags = src[i] & PAGE_MASK;
		if(base == 0) {
			continue;
		}
		
		/* pte1 is the final level, leave copying for COW */
		tbl[i] = mkpte(base, flags | PTE_COW);
	}
	
	return phys;
}

static uaddr_t clone_pte0(_pte0_t *src) {
	_pte0_t *tbl = kmalloc(PTE0_UNITS * sizeof(_pte0_t), MM_ALIGN);
	uaddr_t phys = VIRT_TO_PHYS(tbl);
	memset(tbl, 0, PTE0_UNITS * sizeof(_pte0_t));
	
	int i;
	for(i = 0; i < PTE0_UNITS; i++) {
		uaddr_t base = src[i] & ~PAGE_MASK;
		uaddr_t flags = src[i] & PAGE_MASK;
		if(base == 0) {
			continue;
		}
		
		if(kern_pdir[i] == src[i]) {
			tbl[i] = src[i];
		} else {
			_pte1_t *tbl1 = PHYS_TO_VIRT(base);
			uaddr_t phys1 = clone_pte1(tbl1);
			tbl[i] = mkpte(phys1, flags);
		}
	}
	
	return phys;
}

pdir_t *clone_pdir(pdir_t *src) {
	return PHYS_TO_VIRT(clone_pte0(src));
}
