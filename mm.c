#include "mm.h"
#include "types.h"
#include "page.h"
#include "common.h"
 
void *kmalloc(size_t size, int flags) {
	if(TEST_FLAG(flags, MM_ALIGN)) {
		kimg_end = PAGE_ALIGN(kimg_end) + PAGE_SIZE;
	}
	void *addr = kimg_end;
	kimg_end += size;
	if(TEST_FLAG(flags, MM_ZERO)) {
		char *p = addr;
		memset(addr, 0, size);
	}
	return addr;
}

typedef u32 zone_unit;
#define ZONE_UNIT_BITS 32
#define ZONE_UNIT_GET(unit, idx) (((unit) >> (idx)) & 1)
#define ZONE_UNIT_ALLOC(unit, idx) ((unit) | (1 << (idx)))
#define ZONE_UNIT_FREE(unit, idx) ((unit) & ~(1 << (idx)))

struct zone_info {
	uaddr_t start;
	unsigned int frames;
	unsigned int levels;
	unsigned int free_frames;
	zone_unit **map;
};

void zone_init(uaddr_t begin, uaddr_t end, struct zone_info *zone) {
	int lvl_count = 0;
	unsigned int frames = (end - begin) / PAGE_SIZE + 1;
	while(frames > ZONE_UNIT_BITS) {
		++lvl_count;
		frames /= ZONE_UNIT_BITS;
	}
	
	zone->start = begin;
	zone->levels = lvl_count;
	zone->frames = (end - begin) / PAGE_SIZE + 1;
	zone->free_frames = zone->frames;
	
	zone->map = pmalloc(lvl_count * sizeof(*zone->map), 0);
	unsigned int i = lvl_count, count = zone->frames;
	do {
		zone->map[i - 1] = pmalloc(count * sizeof(*zone->map[i - 1]) / 8, MM_ZERO);
		count /= ZONE_UNIT_BITS;
		--i;
	} while(count > ZONE_BITS);
}


void kfree(void *ptr) {
	ptr = 0;
}
