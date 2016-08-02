
.global scent_handler
scent_handler:
	mov 4(%esp), %edx /* get the regs_t */
	
	mov 32(%edx), %eax /* get eax (syscall number) */
	cmp scent_table_size, %eax /* chech the syscall is valid */
	jb 1f
	ret
1:
	mov 20(%edx), %ebx /* get ebx, (argument list) */
	mov $scent_begin, %edx
	mov (%edx,%eax,8), %ecx /* get number of arguments */
	lea (,%ecx,4), %edi /* size of args list, using lea multiply trick */
2:
	test %ecx, %ecx
	jz 3f
	dec %ecx
	push (%ebx,%ecx,4)
	jnz 2b
3:
	mov 4(%edx,%eax,8),%edx
	mov %edi,%ebx /* cdecl preserves ebx, so put the arg list size there */
	call *%edx
	add %ebx,%esp /* pop the arg list */
	
	mov 4(%esp), %edx
	mov %eax, 32(%edx)
	
	ret

.macro syscall_entry fn argc
scent_\fn:
	.byte \argc
	.fill 3,1,0
	.long \fn
.endm

.macro syscall_func fn
scent_num_\fn:
	.long (scent_\fn - scent_begin) / (scent_dummy_end - scent_dummy_begin)

.global sys_\fn
sys_\fn:
	mov scent_num_\fn, %eax
	lea 4(%esp),%ebx
	int $0x80
	ret
.endm

scent_dummy_begin:
syscall_entry 0 0
scent_dummy_end:

scent_begin:
#include "syscall.h"
scent_end:

syscall_func test_func
syscall_func clone
syscall_func fork
syscall_func getpid

scent_table_size:
	.long (scent_end - scent_begin)/(scent_dummy_end - scent_dummy_begin)
