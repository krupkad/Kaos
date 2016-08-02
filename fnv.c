#ifndef FNV_H
#define FNV_H

#include <stdlib.h>
#include <stdio.h>

typedef unsigned long long u64;

#define FNVP64 ((u64)0x00000100000001b3)
#define FNVO64 ((u64)0xcbf29ce484222325)

u64 fnv64c(u64 input) {
	unsigned int i;
	u64 hash = FNVO64;
	for(i = 0; i < 8; i++) {
		hash ^= input & 0xff;
		hash *= FNVP64;
		input >>= 8;
	}
	
	return hash;
}

extern u64 fnv64(u64 input);

int main() {
	u64 i;
	for(i = 0; i < 1000; i++) {
		u64 code = fnv64c(i);
		u64 ass = fnv64(i);
		
		if(code != ass) {
			printf("code %#.16llx != assembly %#.16llx for input %#.16llx\n",
					code, ass, i);
		}
	}
	return 0;
}

#endif
