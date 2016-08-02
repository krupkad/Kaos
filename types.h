#ifndef TYPES_H
#define TYPES_H

#if !defined(__ASSEMBLY__)

typedef unsigned long long u64;
typedef unsigned long u32;
typedef long s32;
typedef unsigned short u16;
typedef short s16;
typedef unsigned char u8;
typedef char s8;

typedef u32 uaddr_t; /* to be used for ALL physical addresses */
typedef u32 size_t;
typedef s32 pid_t;

#endif /* !defined(__ASSEMBLY__) */

#endif /* TYPES_H */
