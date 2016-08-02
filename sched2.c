#include "common.h"
#include "types.h"
#include "sched.h"
#include "string.h"
#include "mm.h"
#include "vga.h"
#include "page.h"

volatile struct task_t *current_task;
static volatile struct task_t *task_queue;
static pid_t next_pid;
static struct task_t *root_task;

extern pdir_t *kern_pdir;
void kinit_sched() {
	root_task = (struct task_t *)kmalloc(sizeof *root_task, NULL, 0);
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

static void task_exit() {
	register u32 val asm("eax");
	vga_printf("task exit 0x%p 0x%d\n", current_task, val);
	//current_task = task_queue;
	//task_queue = task_queue->next;
	while(1);
}

pid_t clone(int (*fn)(void *), void *arg) {
	/* create the task structure */
	struct task_t *task = kmalloc(sizeof *task, NULL, 0);
	memset(task, 0, sizeof *task);
	task->id = next_pid++;
	task->next = NULL;
	task->ebp = task->esp = kmalloc(STACK_SIZE, NULL, MM_ALIGN) + STACK_SIZE;
	task->pdir = cur_pdir;
	
	/* push necessary arguments */
	*(--task->esp) = (uaddr_t)arg;
	*(--task->esp) = (uaddr_t)task_exit;
	*(--task->esp) = (uaddr_t)fn;
	
	/* add the task to the queue */
	if(task_queue) {
		struct task_t *tmp = task_queue;
		while(tmp->next) {
			tmp = tmp->next;
		}
		tmp->next = task;
	} else {
		task_queue = task;
	}
	vga_printf("created task 0x%p\n", task);
	return task->id;
}

void task_save(struct regs_t *regs) {
	current_task->esp = regs->esp;
	current_task->ss = regs->ss;
	current_task->eip = regs->eip;
}

struct task_t *task_sched() {
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
	
	return current_task;
}

void task_switch() {
	kernel_set_stack(current_task->esp, STACK_SIZE);
	asm volatile("mov %0,%%esp" : : "r"(current_task->esp));
	asm volatile("mov %0,%%ecx; mov %%ecx,%%ss" : : "r"(current_task->ss));
	asm volatile("pushf; pop %ecx; or $0x200,%ecx; push %ecx; popf");
	asm volatile("mov %0,%%ecx; mov %%ecx,%%cr3" : : "r"(VIRT_TO_PHYS(current_task->pdir)));
	asm volatile("mov %0,%%ecx; jmp *%%ecx" : : "r"(current_task->eip));
}
