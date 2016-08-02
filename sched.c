#include "common.h"
#include "types.h"
#include "sched.h"
#include "string.h"
#include "mm.h"
#include "vga.h"
#include "page.h"
#include "common.h"

volatile struct task_t *current_task;
static volatile struct task_t *task_queue;
static pid_t next_pid;
static struct task_t *root_task;

/*
extern u32 kstack;
extern pdir_t *kern_pdir;
void kinit_sched() {
	current_task = (struct task_t *)kmalloc(sizeof *current_task, 0);
	current_task->id = next_pid++;
	current_task->next = NULL;
	current_task->eflags = 0x200;
	current_task->pdir = kern_pdir;
	
	//asm volatile("mov %%ebp, %0" : "=r"(current_task->ebp));
	asm volatile("mov %%esp, %0" : "=r"(current_task->esp));
	
	kernel_set_stack(&kstack, STACK_SIZE);
	current_task->ebp = &kstack;
	//current_task->esp = 
	
	vga_printf("root thread: 0x%p\n", current_task);
	
	task_queue = NULL;
}
*/

extern u32 kstack;
extern pdir_t *kern_pdir;
void kinit_sched() {
	root_task = (struct task_t *)kmalloc(sizeof *root_task, 0);
	root_task->id = next_pid++;
	root_task->next = NULL;
	root_task->pdir = kern_pdir;
	
	asm volatile("mov %%ebp, %0" : "=r"(root_task->ebp));
	asm volatile("mov %%esp, %0" : "=r"(root_task->esp));
	
	current_task = root_task;
	vga_printf("root thread: 0x%p\n", current_task);
	
	task_queue = NULL;
}


void print_queue() {
	vga_printf("current: 0x%p\n", current_task);
	struct task_t *tmp = task_queue;
	vga_printf("queue: ");
	while(tmp) {
		vga_printf("0x%p ", tmp);
		tmp = tmp->next;
	}
	vga_printf("\n\n");
}

pid_t getpid() {
	return current_task->id;
}

extern void switch_stub(struct task_t *prev, struct task_t *next);
static void task_exit() {
	register u32 val asm("eax");
	vga_printf("task exit 0x%p 0x%d\n", current_task, val);
	
	task_queue = task_queue->next;
	
	while(1);
}

pid_t clone(int (*fn)(void *), void *arg, int flags) {
	/* create the task structure */
	struct task_t *task = kmalloc(sizeof *task, 0);
	memset(task, 0, sizeof *task);
	task->id = next_pid++;
	task->next = NULL;
	
	task->ebp = task->esp = kmalloc(STACK_SIZE, MM_ALIGN) + STACK_SIZE;

	/* push necessary arguments */
	*(--task->esp) = (uaddr_t)arg;
	*(--task->esp) = (uaddr_t)task_exit;
	*(--task->esp) = (uaddr_t)fn;
		
	task->pdir = clone_pdir(cur_pdir);
	
	/* add the task to the queue */
	/* TODO: use locks here */
	cli();
	if(task_queue) {
		struct task_t *tmp = task_queue;
		while(tmp->next) {
			tmp = tmp->next;
		}
		tmp->next = task;
	} else {
		task_queue = task;
	}
	sti();
	
	vga_printf("new task 0x%p\n", task);
	return task->id;
}

void task_switch(struct regs_t *reg) {
	/* do nothing if scheduling isn't enabled or there is no next task */
	if(!task_queue) {
		return;
	}
	
	/* append the current task to the end of the queue */
	struct task_t *tmp = task_queue;
	while(tmp->next) {
		tmp = tmp->next;
	};
	tmp->next = current_task;
	current_task->next = NULL;
	
	/* select the new task */
	struct task_t *prev_task = current_task;
	current_task = task_queue;
	task_queue = task_queue->next;
	
	/* set the tss */
	kernel_set_stack(current_task->esp, STACK_SIZE);
	
	/* set the current page directory */
	cur_pdir = current_task->pdir;
	
	/* switch tasks */
	switch_stub(prev_task, current_task);
}
