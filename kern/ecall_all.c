#include <sbi.h>
#include <env.h>
#include <mmu.h>
#include <pmap.h>
#include <printk.h>
#include <sched.h>
#include <ecall.h>

extern struct Env* curenv;

void e_putchar(int c) {
	printcharc((char)c);
	return;
}

int e_print_cons(const void* s,u_int num) {

	for (i = 0; i < num; ++i) {
		printcharc(((char*)s)[i]);
	}
	return 0;
}

u_int e_getenvid(void) {
	return curenv->env_id;
}

void __attribute__((noreturn)) e_yield(void) {
	schedule(1);
}


int e_env_destroy(u_int envid) {
	struct Env* e;
	try(envid2env(envid,&e,1));
	printk("[%08x] destroying %08x\n",curenv->env_id,e->env_id);
	env_destroy(e);
	return 0;
}

static inline int is_illegal_va(u_long va) {

}

int e_mem_alloc(u_int  envid, u_int va, u_int perm) {
	struct Env* env;
	struct Page* pp;
	if (is_illegal_va(va)) {
		return -E_INVAL;
	}
	try(envid2env(envid,&env,1));
	try(page_alloc(&pp));
	return page_insert(env->env_pgdir,env->env_asid,pp,va,perm);
}

int e_mem_map(u_int srcid, u_int srcva, u_int dstid, u_int dstva,u_int perm) {
	struct Env* srcenv;
	struct Env* dstenv;
	struct Page* pp;
	if (is_illegal_va(srcva) || is_illegal_va(dstva)) {
		return -E_INVAL;
	}
	if (envid2env(srcid,&srcenv,1) != 0 || envid2env(dstid,&dstenv,1) != 0) {
		return -E_BAD_ENV;
	}
	Pte* t;
	pp = page_lookup(srcenv->env_pgdir,srcva,&t);
	if (pp == NULL) {
		return -E_INVAL;
	}
	return page_insert(dstenv->env_pgdir, dstenv->env_asid,pp,dstva,perm);

}

int e_mem_unmap(u_int envid, u_int va) {
	struct Env* e;
	if (is_illegal_va(va)) {
		return -E_INVAL;
	}
	try(envid2env(envid,&e,1));
	page_remove(e->env_pgdir,e->env_asid,va);
	return 0;
}

int e_exofork(void) {
	struct Env* e;
	try(env_alloc(&e,curenv->env_id));
	e->env_tf = ;
	e->env_tf.regs[1] = 0;
	e->env_status = ENV_NOT_RUNNABLE;
	e->env_pri = curenv->env_pri;
	return e->env_id;
}

int e_set_env_status(u_int envid, u_int status) {
	struct Env* env;
	if (status  != ENV_NOT_RUNNABLE && status != ENV_RUNNABLE) {
		return -E_INVAL;
	}
	try(envid2env(envid,&env,1));
	if (env->env_status == ENV_RUNNABLE && status == ENV_NOT_RUNNABLE) {
		TAILQ_REMOVE(&env_sched_list,env,env_sched_link);
	}
	else if (env->env_status == ENV_NOT_RUNNABLE && status == ENV_RUNNABLE) {
		TAILQ_INSERT_TAIL(&env_sched_list,env,env_sched_link);
	}
	env->env_status = status;
	return 0;
}

int e_set_trapframe(u_int envid, struct TrapFrame* tf) {
	if (is_illegal_va_range((u_long)tf,sizeof(*tf))) {
		return -E_INVAL;
	}	
	struct Env* env;
	try(envid2env(envid,&env,1));
	if (env == curenv) {
	
	} else {
		env->env_tf = ;
		return 0;
	}

}

int e_ipc_recv(u_int dstva) {


}

int e_ipc_try_send(u_int envid, u_int value, u_int srcva, u_int perm) {



}

int e_write_dev(u_int va, u_int pa, u_int len) {


}

int e_read_dev(u_int va,u_int pa, u_int len) {


}



void e_panic(char* msg) {
	panic("%s",TRUP(msg));
}




int e_cgetc(void) {
	int ch;
	while ((ch = scancharc()) == 0);
	return ch;
}

void* ecall_table[MAX_ENO] = {
	[ECALL_putchar] = e_putchar,
	[ECALL_print_cons] = e_print_cons,
	[ECALL_getenvid] = e_getenvid,
	[ECALL_yield] = e_yield,
	[ECALL_env_destroy] = e_env_destroy,
	
	[ECALL_mem_alloc] = e_mem_alloc,
	[ECALL_mem_map] = e_mem_map,
	[ECALL_mem_unmap] = e_mem_unmap,
	[ECALL_exofork] = e_exofork,
	[ECALL_set_env_status] = e_set_env_status,
	[ECALL_set_trapframe] = e_set_trapframe,
	[ECALL_panic] = e_panic,
	[ECALL_ipc_try_send] = e_ipc_try_send,
	[ECALL_ipc_recv] = e_ipc_recv,
	[ECALL_cgetc] = e_cgetc,
	[ECALL_write_dev] = e_write_dev,
	[ECALL_read_dev] = e_read_dev,

};


void do_ecall(struct TrapFrame* tf) {
	int (*func)(u_int, u_int, u_int, u_int, u_int);
	int eno = tf->regs[10];
    if (eno < 0 || eno >= MAX_ENO) {
   		tf->regs[1] = -E_NO_E;
    	return;
	}	  
	tf->sepc += 4;
	func = ecall_table[eno];
	u_int arg1 = tf->reg[11];
	u_int arg2 = tf->reg[12];
	u_int arg3 = tf->reg[13];
	u_int arg4 = tf->reg[14];
	u_int arg5 = tf->reg[15];
	tf->regs[1] = func(arg1,arg2,arg3,arg4,arg5);
}
