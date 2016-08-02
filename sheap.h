#ifndef SHEAP_H
#define SHEAP_H

#include "types.h"

/* These macros are flags for sheap_create.
 * SHEAP_SUPER sets the supervisor flag in allocated pages.
 * SHEAP_WRITE sets the read/write flag.
 * SHEAP_DYNSIZE allows the heap to allocate/free pages when proper.
 * SHEAP_DPPANIC (DP for Dead Pointer) turns MM operations on a dead pointer
 *   into kernel panics. */

#define SHEAP_SUPER (1 << 7)
#define SHEAP_WRITE (1 << 6)
#define SHEAP_DYNSIZE (1 << 5)
#define SHEAP_DPPANIC (1 << 4)

struct sheap_t {
	uaddr_t root;
	size_t size;
	
	uaddr_t start_addr;
	uaddr_t end_addr;
	
	u8 pmerge;
	u8 flags;
};

struct sheap_t *sheap_create(uaddr_t start, uaddr_t end, u8 flags);
void *sheap_alloc(size_t size, u8 align, struct sheap_t *heap);
void sheap_free(void *ptr, struct sheap_t *heap);
size_t sheap_resize(size_t size, struct sheap_t *heap);

#endif /* SHEAP_H */
