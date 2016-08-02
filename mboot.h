#ifndef MBOOT_H
#define MBOOT_H

#include "types.h"

struct aout_symbol_table {
	u32 tabsize;
	u32 strsize;
	u32 addr;
	u32 reserved;
};
struct elf_section_header_table {
	u32 num;
	u32 size;
	u32 addr;
	u32 shndx;
};

struct mboot_info {
	u32 flags;
	u32 mem_lower;
	u32 mem_upper;
	u32 boot_device;
	u32 cmdline;
	u32 mods_count;
	u32 mods_addr;
	union {
		struct aout_symbol_table aout_sym;
		struct elf_section_header_table elf_sec;
	} u;
	u32 mmap_length;
	u32 mmap_addr;
};

struct mboot_mmap {
	u32 size;
	u32 base_addr_low;
	u32 base_addr_high;
	u32 length_low;
	u32 length_high;
	u32 type;
};

/* we need to know how much memory we have. */
extern size_t phys_mem;

/* provide the bios's map of memory */
extern struct mboot_mmap *bios_mmap;

void kinit_mboot(struct mboot_info *minfo, u32 magic);

#endif /* MBOOT_H */
