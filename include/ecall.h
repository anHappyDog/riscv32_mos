#ifndef ECALL_H
#define ECALL_H

#ifndef __ASSEMBLER__

enum {
	ECALL_putchar,
	ECALL_print_cons,
	ECALL_getenvid,
	ECALL_yield,
	ECALL_env_destroy,
	ECALL_mem_alloc,
	ECALL_mem_map,
	ECALL_mem_unmap,
	ECALL_exofork,
	ECALL_set_env_status,
	ECALL_set_trapframe,
	ECALL_panic,
	ECALL_ipc_try_send,
	ECALL_ipc_recv,
	ECALL_cgetc,
	ECALL_write_dev,
	ECALL_read_dev,
	MAX_ENO,	

};

#endif

#endif

