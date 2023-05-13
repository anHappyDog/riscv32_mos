#include <env.h>
#include <lib.h>
#include <mmu.h>
extern Pde* curenv_pgdir;

static void __attribute__((noreturn)) cow_entry(struct Trapframe* tf) {
	u_int va = tf->stval;
	u_int perm;
	Pde* pd; 
	if (ecall_get_pgdir(&pd) != 0) {
		user_panic("pgdir is NULL !\n");
	}
	//debugf("va is %08x\n",va);
	//debugf("stval is %08x\n",tf->stval);
	Pte* pt = (Pte*)PADDR(PTE_ADDR(pd[PDX(va)])) + PTX(va);
	perm = *pt & 0xc00003ff;
	//debugf("perm is %08x\n",perm);
	if ((perm & PTE_COW) == 0) {
		user_panic("PERM doesn't have PTE_COW\n");
	}
	perm = (perm & ~PTE_COW) | PTE_W;
	if (ecall_mem_alloc(0,(void*)UCOW,perm) < 0) {
		user_panic("failed to alloc a new page at 'UCOW'");
	}
	memcpy((void*)UCOW,(void*)ROUNDDOWN(va,BY2PG),BY2PG);
	if (ecall_mem_map(0,(void*)UCOW,0,(void*)va,perm) < 0 ) {
		user_panic("failed to mem the map from UCOW to va!\n");
	}
	if (ecall_mem_unmap(0,(void*)UCOW) < 0) {
		user_panic("failed to mem unmap UCOW!");
	}
	int r = ecall_set_trapframe(0,(struct Trapframe*)UXSTACKTOP - 1);
	user_panic("ecall_set_trapframe returned %d",r);
}


static void duppage(u_int envid, u_int vpn, u_int perm) {
	u_int addr;
	addr = vpn << PGSHIFT;

	if ((perm & PTE_W) == 0 || (perm & PTE_LIBRARY) == PTE_LIBRARY || (perm & PTE_COW) == PTE_COW) {
		ecall_mem_map(0,(void*)addr,envid,(void*)addr,perm);
	}	
	else {
		perm |= PTE_COW;
		perm &= ~PTE_W;
		ecall_mem_map(0,(void*)addr,envid,(void*)addr,perm);
		ecall_mem_map(0,(void*)addr,0,(void*)addr,perm);
	}
}

int fork(void) {
	u_int child;
	int i;
	extern volatile struct Env* env;
	if (env->env_cow_entry != (u_int)cow_entry) {
		try(ecall_set_env_cow_entry(ecall_getenvid(),(u_int)cow_entry));
	}
	child = ecall_exofork();
	if (child == 0) {
		env = envs + ENVX(ecall_getenvid());
		curenv_pgdir = env->env_pgdir;
		return 0;
	}
	Pde* pg;
	Pte* pt;
	ecall_get_pgdir(&pg);
	for (i = VPN(USTACKTOP) - 1; i >= 0;--i) {
		if (pg[i >> 10] & PTE_V) {
			pt =(Pte*)PADDR(PTE_ADDR(pg[i >> 10])) + (i % 1024);

			//debugf("-----%d --- %08x  pt : %08x\n",i, (i << 12),*pt);
			if (*pt & PTE_V) {
				duppage(child,i,(*pt & 0xc00003ff));
			}
		}
	}
	ecall_set_env_cow_entry(child,(u_int)cow_entry);
	ecall_set_env_status(child,ENV_RUNNABLE);

	return child;
}

