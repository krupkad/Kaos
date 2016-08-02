#ifndef SYSTABLE_H
#define SYSTABLE_H

#ifdef __ASSEMBLY__
#define SYSDECL(argc, fn, ...) syscall_entry fn, argc
#else
#define SYSDECL(argc, fn, ...) int sys_##fn(__VA_ARGS__)
#endif

SYSDECL(3, clone, int (*fn)(void *), void *, int);
SYSDECL(0, getpid);
SYSDECL(2, test_func);
SYSDECL(0, fork);

#endif /* SCENT_H */
