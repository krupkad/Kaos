#ifndef SCHED_H
#define SCHED_H

#include "page.h"
#include "isr.h"

#define CLONE_STACK (1 << 0)

struct task_t {
	pid_t id;
	uaddr_t *ebp, *esp;
	pdir_t *pdir;
	struct task_t *next;
};

void kinit_sched();
pid_t getpid();
pid_t clone(int (*fn)(void *), void *arg, int flags);
pid_t fork();

#endif /* SCHED_H */
