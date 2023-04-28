#include <asm/cp0regdef.h>
#include <asm/embdasm.h>
#include <elf.h>
#include <env.h>
#include <mmu.h>
#include <pmap.h>
#include <printk.h>
#include <sched.h>
#include <sbi.h>
#define NASID 512


struct Env envs[NENV] __attribute__((aligned(BY2PG)));
struct Env* curenv = NULL;
static struct Env_list env_free_list;

struct Env_sched_list env_sched_list;
static Pde* base_pgdir;


//
static uint32_t asid_bitmap[NASID / 32] = {0};



// not ok need fixxed
static int asid_alloc(u_int *asid) {
	for (int i = 0; i < NASID; ++i) {
		int index = i >> 5;
		int inner = i & 31;
		if ((asid_bitmap[index] & (1 << inner)) == 0) {
			asid_bitmap[index] |= 1 << inner;
			*asid = i;
			return 0;
		}
	}
	return -E_NO_FREE_ENV;
}

static void asid_free(u_int i) {
	int index = i >> 5;
	int inner = i & 31;
	asid_bitmap[index] &= ~(1 << inner);

}

static void map_segment(Pde* pgdir, u_int asid, u_long pa, u_long va, u_int size, u_int perm) {
	assert(pa % BY2PG == 0);
	assert(va % BY2PG == 0);
	assert(size % BY2PG == 0);
	
	for (int i = 0; i < size; i += BY2PG) {
		struct Page* pp = addr2page(pa + i);
		page_insert(pgdir,asid,pp,va +i,perm);
	}


}

u_int mkenvid(struct Env* e) {
	static u_int i = 0;
	return ((++i) << (1 + LOG2NENV)) | (e - envs);
}

int envid2env(u_int envid, struct Env** penv, int checkperm) {
	struct Env* e;
	if (envid == 0) {
		*penv = curenv;
		return 0;
	}
	else {
		e = &envs[ENVX(envid)];
	}
	if (checkperm) {
		if (e != curenv && e->env_parent_id != curenv->env_id) {
			return -E_BAD_ENV;
		}
	
	}
	*penv = e;
	return 0;
}


void env_init(void) {
	SET_SSTATUS(1);
	SET_STVEC(0x80200000,0);
	SET_SIE(0,1,0);
	SBI_TIMER(20000 + RD_TIME());
	printk("env_init : interrupt entry set at 0x%08x, mode is %d\n",0x80200000,0);
	printk("env_init : timer interrupt in on !\n");
	//return;
	
	printk("sie is %d\n",((RD_SIE() >> 5) & 1));
	printk("sip is %d\n",((RD_SIP() >> 5) & 1));
	printk("sstatus is %d\n",((RD_SSTATUS() >> 1) & 1));
	LIST_INIT(&env_free_list);
	TAILQ_INIT(&env_sched_list);
	for (int i = NENV - 1; i >= 0; --i) {
		envs[i].env_status = ENV_FREE;
		LIST_INSERT_HEAD(&env_free_list,&envs[i],env_link);
	}
	struct Page* p;
	panic_on(page_alloc(&p));	
	//page_insert((Pde*)(0x83fff000),0,p,page2addr(p),PTE_R | PTE_W);
	p->pp_ref += 1;
	base_pgdir = (Pde*)page2addr(p);
	map_segment(base_pgdir,0,pages,UPAGES,ROUND(npage * sizeof(struct Page),BY2PG),PTE_G | PTE_R);
	map_segment(base_pgdir,0,envs,UENVS,ROUND(NENV * sizeof(struct Env),BY2PG),PTE_G | PTE_R);
	printk("env_init : envs int finished !\n");
}

static int env_setup_vm(struct Env* e) {
	/*
	struct Page* p;
	try(page_alloc(&p));
	e->env_pgdir = (Pte*)page2addr(p);
	memcpy(e->env_pgdir + PDX(UTOP),base_pgdir + PDX(UTOP),
			sizeof(Pde) * (PDX(UVPT) - PDX(UTOP)));
	e->env_pgdir[PDX(UVPT)] =  PTE_V;
	*/
	return 0;

}

