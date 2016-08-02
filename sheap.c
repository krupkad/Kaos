#include "sheap.h"
#include "types.h"
#include "page.h"
#include "kmalloc.h"
#include "common.h"
#include "mtwist.h"

/* assorted macros for testing header flagsibutes */

/* MAGIC_BITS defaults to unused */
#define MAGIC_BITS (0xfeedbead & 0x7fffffff)
/* SANE_MAGIC also works for footers! */
#define SANE_MAGIC(head) (~(((head)->magic & ~(0x80000000)) & MAGIC_BITS))
#define IS_USED(head) ((head)->magic & 0x80000000)
#define SET_UNUSED(head) ((head)->magic &= ~(0x80000000))
#define SET_USED(head) ((head)->magic |= 0x80000000)

#define SHEAP_MIN_SIZE 0x20000

/* we'll need this to get memory frames */
extern struct pdir_t kpdir;

/* I decided to integrate the block-indexing structure with the block structure.
 * A seperate index would have to artificially limited in size because multiple,
 * heaps may exist, we don't know where they are, and even if we did, it would
 * be a pain to manage both their growth *and* the growth of their indices.
 * Fixed-size indices would just be bad mojo. And I will probably implement some
 * sort of heap coherence daemon in the future, so heaps will eventually be
 * tracked. */

/* Some rationale: Why a linked list? Because of reality. Yes, we run the
 * risk of a worst-case O(n) search, if there are no blocks large enough.
 * In terms of worst-case, really large blocks will usually force a slow resize,
 * and always shorten the list, so the inefficiency for those blocks is usually
 * inevitable, and *some* improvement always results. But the tricks are the
 * best reason. First is iteration. Linked list iteration is always a guaranteed
 * O(1) per step, while trees have a worst case O(log(n)) when the successor is
 * higher up in the tree. Don't even mention heaps: with a tree structure, heap
 * iteration requires a second heap, and merges are hellish (and I don't want
 * to have the overhead of more complex heaps). Second, the worst-case O(n) is
 * rarely reached, with a very nice O(log(n)) for average cases, and the
 * blocks that are created from splitting and filling page-alignment gaps are
 * relatively small and quickly inserted, which should become rarer as the heap
 * is populated by more decent-sized blocks. */

/* I'll shut up soon. Memory overhead is 20 bytes per allocation. Compared to
 * dmalloc and friends, its a lot. But remember: this is a kernel.
 * malloc() on *NIX is userspace code, and its "tiny" overhead is more than
 * compensated for by that of the respective kernel's internal allocators.
 * There is no SLAB/SLUB/buddy allocator to deal with here. */

typedef struct __header_t {
	u32 magic;
	u32 size;
	struct __header_t *next;
} _header_t;

typedef struct {
	u32 magic;
	_header_t *head;
} _footer_t;

/* simple heap creation function. the only thing new worth noting is pmerge,
 * which is used during sheap_free() to tweak block merging. */
struct sheap_t *sheap_create(uaddr_t start, uaddr_t end, u8 flags) {
	/* allocate the heap structure (we don't care how) */
	struct sheap_t *heap = (struct sheap_t *)kmalloc(sizeof *heap,0,0);
	
	/* align the start address (up) */
	if(!IS_PALIGNED(start)) {
		start &= ~PAGE_MASK;
		start += PAGE_SIZE;
	}
	
	/* align the end address (down) */
	if(!IS_PALIGNED(end)) {
		end &= ~PAGE_MASK;
	}
	
	/* fill out the heap structure */
	heap->start_addr = start;
	heap->end_addr = end;
	
	/* place a hole spanning the entire memory span */
	_header_t *head = (_header_t *)start;
	head->size = end - start;
	head->magic = MAGIC_BITS;
	SET_UNUSED(head);
	head->next = NULL;
	heap->root = (uaddr_t)head;
	heap->flags = flags;
	heap->pmerge = 0xff;
	
	return heap;
};

/* sheap_resize: Allocate/free pages for the heap  */
size_t sheap_resize(size_t size, struct sheap_t *heap) {
	
	/* before anything, page-align the size */
	if(!IS_PALIGNED(size)) {
		size &= ~PAGE_MASK;
		size += PAGE_SIZE;
	}
	
	/* get the current size of the heap */
	size_t old_size = heap->end_addr - heap->start_addr;
	
	/* perform the resize */
	if(size > old_size) {
		size_t i = heap->end_addr + PAGE_SIZE;
		while(i < size) {
			page_t *page = page_alloc(i, &kpdir);
			uaddr_t frame = frame_alloc();
			u32 write = TEST_FLAG(heap->flags, SHEAP_WRITE);
			u32 
			*page = page_create(frame, heap->flags 
			i += PAGE_SIZE;
		}
	} else if(size < old_size) {
		assert(size > 0);
		size_t i = old_size - PAGE_SIZE;
		while(i > size) {
			struct page_t *page = page_get(heap->start_addr + i, 0, kern_dir);
			frame_free(page);
			i -= PAGE_SIZE;
		}
	}
	
	heap->end_addr = heap->start_addr + size;
	return size;
}

/* insertion is not so simple because of root pointer fuss, so I split it off */
static inline void sheap_insert(_header_t *head, struct sheap_t *heap) {
	if(!heap->root) {
		heap->root = (uaddr_t)head;
		head->next = NULL;
	} else {
		_header_t *cur = (_header_t *)heap->root, *prev = NULL;
		while(cur && cur->size < head->size) {
			prev = cur;
			cur = cur->next;
		}
		if(prev) {
			prev->next = head;
		} else {
			/* since we have no parent, we must be root, and we make sure */
			assert(heap->root == (uaddr_t)cur);
			heap->root = (uaddr_t)head;
		}
		head->next = cur;
	}
}

/* okay, neither is deletion */
static inline void sheap_remove(_header_t *par, _header_t *head,
														struct sheap_t *heap) {
	if(par) {
		par->next = head->next;
	}
	if(head == (_header_t *)heap->root) {
		heap->root = (uaddr_t)head->next;
	}
	head->next = NULL;
}

/* this will become part of memory statistics, which we all love... */
u32 sheap_size(struct sheap_t *heap) {
	_header_t *cur = (_header_t *)heap->root;
	u32 i = 0;
	while(cur) {
		vga_puts("node ");
		vga_printnum(i, 10);
		vga_puts(" size ");
		vga_printnum(cur->size - sizeof(_header_t) - sizeof(_footer_t), 10);
		vga_puts(" at 0x");
		vga_printnum((u32)cur + sizeof(_header_t), 16);
		vga_puts("\n");
		cur = cur->next;
		i++;
	}
	return i;
}

/* A big meaty allocation function. In terms of speed, an act of god can make
 * us iterate through the list three times: once to find a hole (always done),
 * once if page alignment allows another block to be created, and once if we are
 * splitting an existing block. The rest is just a lot of pointer arithmetic.
 * Big-O notation lets us ignore that for the bigger picture: allocation is
 * worst-case O(log(n)) */

/* The only way this function can fail willingly is if we are out of usable
 * heap space for the creation of a new block, and we didn't set SHEAP_DYNSIZE.
 * This is the only time allocation will return NULL, and can always be resolved
 * by sheap_resize(), as long as there is room to do a resize. Of course, any
 * assortment of other faults can occur (I'm predicting page faults). */

/* sheap_alloc: allocate memory from a heap */
void *sheap_alloc(size_t size, struct sheap_t *heap, u8 flags) {
	/* get the total block size, including header and footer */
	size_t real_size = size + sizeof(_header_t) + sizeof(_footer_t);
	/* find a proper hole */
	_header_t *found_head = (_header_t *)heap->root, *parent = NULL;
	while(found_head->next && found_head->next->size < real_size) {
		parent = found_head;
		found_head = found_head->next;
	}
	
	 /* if we dont have a hole, resize and make one. otherwise, unlist ours */
	if(!found_head->next && found_head->size < real_size) {
		/* if the heap is allowed to dynamically resize, do so. if not, fail
		 * like a camel in a swamp */
		if(heap->flags & SHEAP_DYNSIZE) {
			/* resize the heap */
			size_t old_length = heap->end_addr - heap->start_addr;
			sheap_resize(old_length + real_size, heap);
			size_t new_length = heap->end_addr - heap->start_addr;
			
			/* place the new footer */
			found_head->size += new_length - old_length;
			uaddr_t floc = (uaddr_t)found_head + found_head->size - sizeof(_footer_t);
			_footer_t *foot = (_footer_t *)floc;
			foot->head = found_head;
			foot->magic = MAGIC_BITS;
		} else {
			return NULL;
		}
	} else {
		sheap_remove(parent, found_head, heap);
	}
	
	/* if the block isn't big enough to split, use it all */
	uaddr_t found_pos = (uaddr_t)found_head;
	size_t found_size = found_head->size;
	if(found_size - real_size < sizeof(_header_t) + sizeof(_footer_t)) {
		size += found_size - real_size;
		real_size = found_size;
	}
	
	/* align if wanted*/
	uaddr_t mem_pos = found_pos + sizeof(_header_t);
	if(TEST_FLAG(flags, MM_ALIGN) && !IS_PALIGNED(mem_pos)) {
		/* get our new position */
		uaddr_t new_pos = (mem_pos & PAGE_MASK) + PAGE_SIZE - sizeof(_header_t);
		u32 diff = new_pos - found_pos;
		
		/* if there is room, create a hole at the old position */
		if(new_pos - found_pos > sizeof(_header_t) + sizeof(_footer_t)) {
			/* create it */
			_header_t *head = (_header_t *)found_pos;
			head->size = PAGE_SIZE - (found_pos % PAGE_SIZE) - sizeof(_header_t);
			head->magic = MAGIC_BITS;
			SET_UNUSED(head);
			_footer_t *foot = (_footer_t *)((u32)new_pos - sizeof(_footer_t));
			foot->magic = MAGIC_BITS;
			foot->head = head;
			
			/* link it in */
			sheap_insert(head, heap);
		} else {
			/* we really should whine about this or something, because a bit
			 * of memory has disappeared... */
			vga_puts("warning: aligned alloc zapped ");
			vga_printnum(new_pos - found_pos);
			vga_puts(" bytes at 0x");
			vga_printnum(found_pos);
			vga_puts(". this is most definitely a bug.\n");
		}
			
		/* update our position and size */
		found_pos = new_pos;
		found_size -= diff;
	}
	
	/* prepare the final block */
	_header_t *blk_head = (_header_t *)found_pos;
	blk_head->magic = MAGIC_BITS;
	SET_USED(blk_head);
	blk_head->size = real_size;
	_footer_t *blk_foot = (_footer_t *)(found_pos + sizeof(_header_t) + size);
	blk_foot->magic = MAGIC_BITS;
	vga_puts("HERE\n");
	blk_foot->head = blk_head;
	
	/* if there is room before the end of the heap, add a hole */
	u32 next_head = found_pos + blk_head->size;
	if(found_size - real_size > 0
		&& next_head + sizeof(_header_t) + sizeof(_footer_t)< heap->end_addr) {
			
		/* initialize its header and footer */
		_header_t *head = (_header_t *)(next_head);
		head->magic = MAGIC_BITS;
		SET_UNUSED(head);
		head->size = found_size - real_size;
		uaddr_t floc = (uaddr_t)head + head->size - sizeof(_footer_t);
		_footer_t *foot = (_footer_t *)floc;
		foot->magic = MAGIC_BITS;
		foot->head = head;
		
		/* place it in the list */
		sheap_insert(head, heap);
	}
	
	return (void *)((uaddr_t)blk_head + sizeof(_header_t));
}

/* The only advice here: don't mix heaps. Virtual addressing is awesome, but
 * there is always potential for leaking allocated memory between address
 * spaces. The obvious solution is to check whether the pointer is within the
 * heap's known address range, but it fails if the foreign pointer *happens*
 * to fit in that range. Either way, you probably *want* a page fault if that
 * sort of thing starts happening. */

/* sheap_free: free heap-allocated pointers */
void sheap_free(void *ptr, struct sheap_t *heap) {
	/* dont freak out about null pointers */
	if(!ptr) {
		return;
	}

	/* get this pointer's header and footer */
	_header_t *head = (_header_t *)((uaddr_t)ptr - sizeof(_header_t));
	_footer_t *foot = (_footer_t *)((uaddr_t)head + head->size - sizeof(_footer_t));
	
	/* check this block's sanity */
	assert(SANE_MAGIC(head));
	assert(SANE_MAGIC(foot));
	
	/* check for double free */
	if(!IS_USED(head)) {
		if(heap->flags & SHEAP_DPPANIC) {
			panic("double free'd pointer");
		}
		return;
	}
	
	/* lots of heuristic improvement available here.
	 * page-aligned are only left-merged with other page-aligned blocks because
	 * those are popular, and we  make merges (pseudo)random, to balance
	 * availability of smaller blocks with that of larger ones. both of these
	 * reduce the chances of slow branches during allocation, as both
	 * page-alignment and block splitting can produce new blocks, which require
	 * ew searches of the list. also, merges require searches, and while we
	 * don't care as much about the speed of freeing, faster is always
	 * appreciated =) */
	
	/* Determine which merges should be done, and find the new block size */
	_footer_t *left_foot = (_footer_t *)((uaddr_t)head - sizeof(_footer_t));
	_header_t *left_head; /* will be NULL'd if wrong */
	_header_t *right_head = (_header_t *)((uaddr_t)foot + sizeof(_footer_t));
	size_t new_size = head->size;
	
	/* we use 32-bit mersenne twister (mt_rand) for random numbers */
	if((uaddr_t)left_foot >= heap->start_addr /* on the heap */
			&& SANE_MAGIC(left_foot) /* is a foot */
			&& !IS_USED(left_foot->head) /* this block is empty */
			&& (mt_rand() & 0xff) < heap->pmerge) /* probability */ {
		vga_puts("left merge\n");
		left_head = left_foot->head;
		new_size += left_head->size;
	} else {
		left_head = NULL; /* not left_foot, we don't care about that */
	}
	
	/* correct the above for page-aligment: only merge if the other block
	 * is also aligned */
	if(left_head && IS_PALIGNED(head + sizeof(_header_t))
						&& !IS_PALIGNED(left_head + sizeof(_header_t))) {
		new_size -= left_head->size;
		left_head = NULL;
	}
	if(right_head && IS_PALIGNED(head + sizeof(_header_t))
						&& !IS_PALIGNED(right_head + sizeof(_header_t))) {
		new_size -= right_head->size;
		right_head = NULL;
	}
	
	if((uaddr_t)right_head < heap->end_addr
			&& SANE_MAGIC(right_head)
			&& !IS_USED(right_head)
			&& (mt_rand() & 0xff) < heap->pmerge) {
		vga_puts("right merge\n");
		new_size += right_head->size;
	} else {
		right_head = NULL;
	}
	
	/* we can kill three birds with one stone here, and locate both adjacent
	 * blocks on the way to the insertion point, because the size of the new
	 * block is guaranteed to be >= to the size of its components. */
	
	/* A nice tidbit: I considered using 'prev' pointers for the list. If I
	 * would have done that, I would have never thought much about the
	 * (in)efficiency of traversal, and we wouldn't have this little speed
	 * boost. Thank god I'm thoughtful. */
	
	/* traverse the list, find insertion point, and record necessary parents */
	_header_t *left_par = NULL, *right_par = NULL, *cur = (_header_t *)heap->root;
	if(!cur) {
		/* memory management's life is probably too messed up to go on */
		panic("corrupted heap");
	}
	while(cur->next && cur->next->size < new_size) {
		if(left_head && cur->next == left_head) left_par = cur;
		if(right_head && cur->next == right_head) right_par = cur;
		cur = cur->next;
	}
	
	/* remove blocks to be merged */
	if(left_head) {
		sheap_remove(left_par, left_head, heap);
	}
	if(right_head) {
		sheap_remove(right_par, right_head, heap);
	}
	
	/* do the merges */
	if(left_head) {
		head = left_head;
	}
	if(right_head) {
		foot = (_footer_t *)((uaddr_t)right_head + right_head->size - sizeof(_footer_t));
	}
	head->size = new_size;
	foot->head = head;
	
	/* mark it free */
	SET_UNUSED(head);

	/* if we can shrink the heap and not reinsert, do it. */
	/* TODO: 'shrink aggression' should be per-heap configuration */
	if(heap->flags & SHEAP_DYNSIZE) {
		if((uaddr_t)foot + sizeof(_footer_t) == heap->end_addr) {
			size_t old_hsize = heap->end_addr - heap->start_addr;
			size_t new_hsize = old_hsize - head->size;
			if(new_hsize >= SHEAP_MIN_SIZE) {
				sheap_resize(new_hsize, heap);
			}
		}
	} else {
		/* insert the block */
		sheap_insert(head, heap);
	}

	sheap_insert(head, heap);
}

#if 0
void *sheap_realloc(void *ptr, size_t size, struct sheap_t *heap) {
	/* be normal and allocate if ptr == NULL */
	if(!ptr) {
		return sheap_alloc(size, 0, heap);
	}
	
	/* get this pointer's header and footer */
	_header_t *head = (_header_t *)((u32)ptr - sizeof(_header_t));
	_footer_t *foot = (_footer_t *)((u32)head + head->size - sizeof(_footer_t));
	
	/* check this block's sanity */
	assert(SANE_MAGIC(head));
	assert(SANE_MAGIC(foot));
	
	/* make sure the block is allocated */
	/* there is no normal behaviour for this, so fail */
	if(!IS_USED(head)) {
		if(TEST_FLAG(heap->flags, SHEAP_DPPANIC)) {
			panic("realloc'd free pointer");
		} else {
			return NULL;
		}
	}
	
	/* look for merge opportunities */
	_footer_t *left_foot = (_footer_t *)((u32)head - sizeof(_footer_t));
	_header_t *left_head; /* will be NULL'd if wrong */
	_header_t *right_head = (_header_t *)((u32)foot + sizeof(_footer_t));
	u32 new_size = head->size;
	
	/* we use 32-bit mersenne twister (mt_rand) for random numbers */
	/* I learned this painfully: don't even try if the possible foot / head
	 * isn't even on the heap */
	if(((u32)left_foot >= heap->start_addr /* on the heap */
			&& SANE_MAGIC(left_foot) /* is a foot */
			&& !IS_USED(left_foot->head) /* this block is empty */
			&& (mt_rand() & 0xff) < heap->pmerge) /* probability */
			&& !IS_PALIGNED(head + sizeof(_header_t))) /* alignment */ {
		vga_puts("left merge\n");
		left_head = left_foot->head;
		new_size += left_head->size;
	} else {
		left_head = NULL; /* not left_foot, we don't care about that */
	}
	
	if((u32)right_head < heap->end_addr
			&& SANE_MAGIC(right_head)
			&& !IS_USED(right_head)
			&& (mt_rand() & 0xff) < heap->pmerge) {
		vga_puts("right merge\n");
		new_size += right_head->size;
	} else {
		right_head = NULL;
	}
#endif
