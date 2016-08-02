#include "mboot.h"
#include "common.h"
#include "types.h"

size_t phys_mem;

struct mboot_mmap *bios_mmap;

void kinit_mboot(struct mboot_info *minfo, u32 magic) {
	if(magic != 0x2badb002) {
		panic("multiboot isn't magical");
	}
	
	if(!(minfo->flags & 0x01)) {
		panic("can't see your ram");
	}
	
	phys_mem = minfo->mem_upper * 1024;
	bios_mmap = (struct mboot_mmap *)minfo->mmap_addr;
}
