#include <env.h>
#include <lib.h>
#include <mmu.h>

void ipc_send(u_int whom, u_int val, const void* srcva, u_int perm) {
	int r;
	while ((r = ecall_ipc_try_send(whom,val,srcva,perm)) == -E_IPC_NOT_RECV) {
		ecall_yield();
	}
	user_assert(r == 0);
}

u_int ipc_recv(u_int *whom, void* dstva, u_int* perm) {
	int r = ecall_ipc_recv(dstva);
	if (r != 0) {
		user_panic("ecall_ipc_recv error: %d",r);
	}
	if (whom) {
		*whom = env->env_ipc_from;
	}
	if (perm) {
		*perm = env->env_ipc_perm;
	}
	return env->env_ipc_value;
}
