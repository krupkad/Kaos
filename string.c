#include "string.h"

void *memcpy(void *dest, const void *src, int count) {
	int i;
	for(i = 0; i < count; i++) {
		((char *)dest)[i] = ((char *)src)[i];
	}
	return dest;
}

void *memset(void *dest, int val, int count) {
	int i;
	for(i = 0; i < count; i++) {
		((unsigned char *)dest)[i] = (unsigned char)val;
	}
	return dest;
}

int strlen(const char *str) {
	int c = 0;
	while(*str) {
		++c;
		++str;
	}
	return c;
}
