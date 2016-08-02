#include "mtwist.h"
#include "types.h"

/* TODO: we *really* should have a mutex on this */

static u32 tstate[624];
static u32 tindex = 0;

void mt_seed(u32 seed) {
	tstate[0] = seed;
	u32 i;
	for(i = 1; i < 624; i++) {
		tstate[i] = (0x6c078965 * (tstate[i - 1] ^ (tstate[i - 1] >> 30)) + i);
	}
}

static inline void mt_gen() {
	u32 i;
	for(i = 0; i < 624; i++) {
		u32 y = (tstate[i] >> 31) + (tstate[(i+1) % 624] ^ (1 << 31));
		tstate[i] = tstate[(i+397) % 624] ^ (y >> 1);
		if(y & 1) {
			tstate[i] ^= 0x9908b0df;
		}
	}
}

u32 mt_rand() {
	if(tindex == 0) {
		mt_gen();
	}
	
	u32 y = tstate[tindex];
	y ^= y >> 11;
	y ^= (y << 7) & 0x9d2c5680;
	y ^= (y << 15) & 0xefc60000;
	y ^= y >> 18;
	
	tindex = (tindex + 1) % 624;
	return y;
}
