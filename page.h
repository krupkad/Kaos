#ifndef PAGE_H
#define PAGE_H

#include "types.h"

/* batshit insane paging details. */
 
/* first, we describe the layout of the processors paging structures.
 * unlike *NIXen, we don't impose any particular number of layers.
 * instead, we provide macros and typedefs to make implementing code
 * for various systems uniform, and easier (admit it, boilerplate *is*
 * easy). Each level is a _pteN_t[], and the index in the table is
 * calculated by PTEN_INDEX(vaddr), which extracts the proper index
 * from a virtual address. */

/* the 1024 x 4 byte "page directory" */
#define PTE0_WIDTH 10
#define PTE0_SHIFT 22
#define PTE0_UNITS (1 << PTE0_WIDTH)
#define PTE0_MASK (PTE0_UNITS - 1)
#define PTE0_INDEX(vaddr) ((((uaddr_t)(vaddr)) >> PTE0_SHIFT) & PTE0_MASK)
typedef u32 _pte0_t;

/* the 1024 x 4 byte "page table" */
#define PTE1_WIDTH 10
#define PTE1_SHIFT 12
#define PTE1_UNITS (1 << PTE1_WIDTH)
#define PTE1_MASK (PTE1_UNITS - 1)
#define PTE1_INDEX(vaddr) ((((uaddr_t)(vaddr)) >> PTE1_SHIFT) & PTE1_MASK)
typedef u32 _pte1_t;

/* this is where magic happens, and that stuff becomes useful. in the
 * end, the only things most other kernel code cares about are the
 * first and last levels, which are almost always called the 'directory'
 * and the 'page' (because systems almost always implement these, and
 * were uncreative/sane and took the names from Intel). So we typedef
 * those _pteN_t's. */

typedef _pte0_t pdir_t;
typedef _pte1_t pte_t;

/* finally, info on pages themselves */

/* the 4096 x 1 byte "page" */
#define PAGE_WIDTH 12
#define PAGE_SIZE (1 << PAGE_WIDTH)
#define PAGE_MASK (PAGE_SIZE - 1)
#define PAGE_OFFSET(addr) ((uaddr_t)(addr) & PAGE_MASK)
#define PAGE_ALIGN(addr) ((void *)((uaddr_t)(addr) & (~PAGE_MASK)))
#define IS_PAGE_ALIGNED(addr) (PAGE_OFFSET(addr) == 0)
#define PAGE_COUNT (phys_mem / PAGE_SIZE)

/* flags for pte's */
#define PTE_PRESENT (0x01)
#define PTE_WRITE (0x01 << 1)
#define PTE_USER (0x01 << 2)
#define PTE_ACCESSED (0x01 << 5)
#define PTE_DIRTY (0x01 << 6)
#define PTE_COW (0x01 << 9) /* bit 9 is "available" */

extern pdir_t *cur_pdir;
void page_chdir(pdir_t *dir);
pte_t *page_map(void *vaddr, uaddr_t addr, pdir_t *dir, int pte_flags);
pte_t page_create(uaddr_t frame, u32 flags);
pdir_t *clone_pdir(pdir_t *src);

#endif /* PAGE_H */
