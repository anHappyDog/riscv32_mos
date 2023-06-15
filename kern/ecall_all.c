#include <env.h>
#include <mmu.h>
#include <pmap.h>
#include <sched.h>
#include <ecall.h>
#include <asm/embdasm.h>
#include <drivers/console.h>
#include <drivers/virtio_disk.h>

extern struct Env* curenv;



int e_get_pgref(void*v)
{
	return addr2page(v)->pp_ref;

}

void e_read_disk(uint32_t lowsec,uint32_t highsec,void*buf,int nsec) {
	uint64_t sector = lowsec | ((uint64_t)highsec << 32);
	disk_rw(sector,0,buf,nsec);

}
void e_write_disk(uint32_t lowsec,uint32_t highsec,void*buf,int nsec) {
	uint64_t sector = lowsec | ((uint64_t)highsec << 32);
	disk_rw(sector,1,buf,nsec);

}

void e_putchar(int c) {
	printcharc((char)c);
	return;
}

int e_getchar(void) {
	return scancharc();
}

int e_print_cons(const void* s,u_int num) {
	for (int i = 0; i < num; ++i) {
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
	return va < UTEMP || va >= UTOP;
}

static inline int is_illegal_va_range(u_long va, u_int len) {
	if (len == 0) {
		return 0;
	}
	return va + len < va || va < UTEMP || va + len > UTOP;

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
	if (envid2env(srcid,&srcenv,1) != 0 || envid2env(dstid,&dstenv,0) != 0) {
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
	//memcpy(e->env_pgdir,cur_pgdir,BY2PG);	
	//context change KSTACKTOP
	e->env_tf = *((struct Trapframe*)RD_SSCRATCH());	
	e->env_tf.regs[10] = 0;
	e->env_status = ENV_NOT_RUNNABLE;
	e->env_pri = curenv->env_pri;
	//reflect_pgdir(e);
	//Pte* pt;
	//u_int addr,perm;
	/*for (int i = VPN(USTACKTOP) - 1;i >= VPN(USTACKTOP) - 8; --i) {
		pt = (Pte*)PADDR(PTE_ADDR(cur_pgdir[i >> 10])) + (i % 1024);
		addr = i << PGSHIFT;
		perm = (((*pt & 0xc00003ff) | PTE_COW) & ~ PTE_W);
		e_mem_map(0, addr,e->env_id,addr,perm);
		e_mem_map(0, addr,0,addr,perm);
	}*/
	//e_mem_map(curenv->env_id,(u_int)cur_pgdir,e->env_id,(u_int)cur_pgdir,PTE_U | PTE_R | PTE_COW);
	/*for (int i = 0; i < 1024; ++i) {
		if (cur_pgdir[i] & PTE_V) {
			pt = (Pde*)PADDR(PTE_ADDR(cur_pgdir[i]));
			e_mem_map(curenv->env_id,(u_int)pt,e->env_id,(u_int)pt,PTE_U | PTE_R | PTE_COW);
		}
	}*/

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

int e_set_trapframe(u_int envid, struct Trapframe* tf) {
	if (is_illegal_va_range((u_long)tf,sizeof(*tf))) {
		return -E_INVAL;
	}	
	struct Env* env;
	try(envid2env(envid,&env,1));
	if (env == curenv) {
		*((struct Trapframe*)RD_SSCRATCH()) = *tf;	
		return tf->regs[1];
	} else {
		env->env_tf = *tf;
		return 0;
	}

}

int e_ipc_recv(u_int dstva) {
	if (dstva != 0 && is_illegal_va(dstva)) {
		return -E_INVAL;
	}
	curenv->env_ipc_recving = 1;
	curenv->env_ipc_dstva = dstva;
	curenv->env_status = ENV_NOT_RUNNABLE;
	TAILQ_REMOVE(&env_sched_list,curenv,env_sched_link);
	((struct Trapframe*)RD_SSCRATCH())->regs[10] = 0;
	//curenv->env_tf.regs[10] = 0;
	schedule(1);
}

int e_ipc_try_send(u_int envid, u_int value, u_int srcva, u_int perm) {
	struct Env* e;
	struct Page* p;
	if (srcva != 0 && is_illegal_va(srcva)) {
		return -E_INVAL;
	}
	try(envid2env(envid,&e,0));
	if (e->env_ipc_recving == 0) {
		return -E_IPC_NOT_RECV;
	}
	e->env_ipc_value = value;
	e->env_ipc_from = curenv->env_id;
	e->env_ipc_perm = PTE_V | perm;
	e->env_ipc_recving = 0;
	e->env_status = ENV_RUNNABLE;
	TAILQ_INSERT_TAIL(&env_sched_list,e,env_sched_link);
	Pte* t;
	if (srcva != 0) {
		p = page_lookup(curenv->env_pgdir,srcva,&t);
		if (p == NULL) {
			return -E_INVAL;
		}
		page_insert(e->env_pgdir,e->env_asid,p,e->env_ipc_dstva,perm);
	}
	return 0;
}

int e_write_dev(u_int va, u_int pa, u_int len) {
	return 0;

}

int e_read_dev(u_int va,u_int pa, u_int len) {
	return 0;

}



void e_panic(char* msg) {
	panic("%s",TRUP(msg));
}




int e_cgetc(void) {
	int ch;
	while ((ch = scancharc()) == 0);
	return ch;
}

int e_set_env_cow_entry(u_int envid, u_int cow_entry) {
	struct Env* e;
	try(envid2env(envid,&e,1));
	e->env_cow_entry = cow_entry;
	return 0;
}

int e_get_pgdir(Pde** pde) {
	if (curenv == NULL) {
		return -E_INVAL;
	}
	//reflect_pgdir(curenv);		
	*pde = cur_pgdir;
	return 0;
}


void* ecall_table[MAX_ENO] = {
	[ECALL_putchar] = e_putchar,
	[ECALL_getchar] = e_getchar,
	[ECALL_print_cons] = e_print_cons,
	[ECALL_getenvid] = e_getenvid,
	[ECALL_yield] = e_yield,
	[ECALL_env_destroy] = e_env_destroy,
	[ECALL_set_env_cow_entry] = e_set_env_cow_entry,	
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
	[ECALL_get_pgdir] = e_get_pgdir,
	[ECALL_write_dev] = e_write_dev,
	[ECALL_read_dev] = e_read_dev,
	[ECALL_read_disk] = e_read_disk,
	[ECALL_write_disk] = e_write_disk,	
	[ECALL_get_pgref] = e_get_pgref,
};



