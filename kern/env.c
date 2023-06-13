#include <drivers/virtio_disk.h>
#include <asm/embdasm.h>
#include <elf.h>
#include <env.h>
#include <mmu.h>
#include <queue.h>
#include <pmap.h>
#include <printk.h>
#include <sched.h>
#include <sbi.h>
#define NASID 512

extern struct virtqueue * disk;
struct Env envs[NENV] __attribute__((aligned(BY2PG)));
struct Env* curenv = NULL;
static struct Env_list env_free_list;

struct Env_sched_list env_sched_list;
static Pde* base_pgdir;

//
static uint32_t asid_bitmap[NASID / 32] = {0};



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

void reflect_pgdir(struct Env* e) {
	u_int addr;	
	map_segment(e->env_pgdir,e->env_asid,(u_long)(e->env_pgdir),(u_long)(e->env_pgdir),BY2PG,PTE_U | PTE_R | PTE_W);
	for (int i = 0; i < 1024; ++i) {
		if (e->env_pgdir[i] & PTE_V) {
			addr = PADDR(PTE_ADDR(e->env_pgdir[i]));
			map_segment(e->env_pgdir,e->env_asid,addr,addr,BY2PG, PTE_U | PTE_R | PTE_W);
		}
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
	SET_SSTATUS(1,1);
	SET_STVEC(KERNEND,0);
	SET_SIE(1,1,1);
	//SBI_TIMER(4000 + RD_TIME());
	//asm("ebreak" :: );
	printk("env_init : interrupt entry set at 0x%08x, mode is %d\n",KERNEND,0);
	printk("env_init : timer interrupt in on !\n");
	printk("---------------------------------------------------------------\n");
	
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
	//panic("-----%08x\n",ROUND((u_long)pages - 0x80200000,BY2PG));
	//panic("------envs : %08x    pages : %08x\n",envs,pages);
	//map_segment(base_pgdir,0,KERNSTART,KERNSTART,ROUND(0x4000000,BY2PG),PTE_G |  PTE_R | PTE_W  |PTE_X);	
	map_segment(base_pgdir,0,KERNSTART,KERNSTART,ROUND((u_long)envs - KERNSTART,BY2PG),PTE_G | PTE_R | PTE_X);
	map_segment(base_pgdir,0,KERNEND,KERNEND,ROUND((u_long)pages - KERNEND,BY2PG),PTE_G | PTE_R | PTE_X);
	map_segment(base_pgdir,0,(u_long)envs,(u_long)envs,ROUND(KERNEND - (u_long)envs,BY2PG),PTE_R | PTE_W);
	map_segment(base_pgdir,0,(u_long)pages,(u_long)pages,ROUND(0x84000000 - (u_long)pages,BY2PG), PTE_R | PTE_W);
	//map_segment(base_pgdir,0,(u_long)pages,(u_long)pages,ROUND(0x84000000 - (u_long)pages,BY2PG),PTE_R | PTE_W);
	map_segment(base_pgdir,0,(u_long)pages,UPAGES,ROUND(npage * sizeof(struct Page),BY2PG),PTE_G |  PTE_R | PTE_U);
	map_segment(base_pgdir,0,(u_long)envs,UENVS,ROUND(NENV * sizeof(struct Env),BY2PG), PTE_G | PTE_R | PTE_U);
	//map_segment(base_pgdir,0,DEV_DISK_REGADDRESS,DEV_DISK_REGADDRESS,BY2PG,PTE_R | PTE_W | PTE_G | PTE_LIBRARY);	
	map_segment(base_pgdir,0,(u_long)disk,(u_long)disk,BY2PG,PTE_G | PTE_R | PTE_W | PTE_LIBRARY);
	map_segment(base_pgdir,0,(u_long)disk->desc,(u_long)disk->desc,BY2PG,PTE_G | PTE_R | PTE_W | PTE_LIBRARY);
	map_segment(base_pgdir,0,(u_long)disk->avail,(u_long)disk->avail,BY2PG,PTE_G | PTE_R | PTE_W | PTE_LIBRARY);
	map_segment(base_pgdir,0,(u_long)disk->used,(u_long)disk->used,BY2PG,PTE_G | PTE_R | PTE_W | PTE_LIBRARY);
	
	pgdir_map(base_pgdir,0,DISK_ADDRESS - KERNSTART ,DISK_ADDRESS,PTE_R | PTE_W | PTE_G | PTE_LIBRARY);
	printk("envs's address is 0x%08x\n",envs);
	printk("env_init : envs int finished !\n");
	printk("---------------------------------------------------------------\n");
}

static int env_setup_vm(struct Env* e) {
	//u_long addr;
	struct Page* p;
	try(page_alloc(&p));
	p->pp_ref += 1;
	e->env_pgdir = (Pte*)page2addr(p);
	memcpy(e->env_pgdir,base_pgdir,BY2PG);
	for (int i = 0; i < 10; ++i) {
		p = NULL;
		try(page_alloc(&p));
		page_insert(e->env_pgdir,e->env_asid,p,UXSTACKTOP - (i + 1) * BY2PG,PTE_U | PTE_W | PTE_R);
	}
	/*
	map_segment(e->env_pgdir,e->env_asid,(u_long)(e->env_pgdir),(u_long)(e->env_pgdir),BY2PG,PTE_U | PTE_R | PTE_W);
	for (int i = 0; i < 1024; ++i) {
		if (e->env_pgdir[i] & PTE_V) {
			addr = PADDR(PTE_ADDR(e->env_pgdir[i]));
			map_segment(e->env_pgdir,e->env_asid,addr,addr,BY2PG, PTE_U | PTE_R | PTE_W);
		}
	}*/
	return 0;

}

int env_alloc(struct Env** new, u_int parent_id) {
	struct Env* e;
	e = LIST_FIRST(&env_free_list);
	if (e == NULL) {
		return -E_NO_FREE_ENV;
	}
	try(env_setup_vm(e));
	e->env_cow_entry = 0;
	e->env_runs = 0;
	
	e->env_id = mkenvid(e);
	try(asid_alloc(&(e->env_asid)));
	e->env_parent_id = parent_id;
	//turn on the interrupt
	e->env_tf.sstatus = ((1 << 1)|(1 << 18));
	//give space for argc and argv
	e->env_tf.regs[2] = USTACKTOP - sizeof(int) - sizeof(char**); 
		
	LIST_REMOVE(e,env_link);
	*new = e;
	return 0;

}

static int load_icode_mapper(void* data, u_long va, size_t offset, u_int perm, const void* src, size_t len) {

	struct Env* env = (struct Env* )data;
	struct Page* p;

	try(page_alloc(&p));
	if (src != NULL) {
		memcpy((void*)(page2addr(p) + offset),src,len);
	}
	return page_insert(env->env_pgdir,env->env_asid,p,va,perm);
}

static void load_icode(struct Env* e, const void* binary, size_t size) {
	const Elf32_Ehdr* ehdr = elf_from(binary,size);
	if (!ehdr) {
		panic("bad elf at %x",binary);
	}
	size_t ph_off;
	ELF_FOREACH_PHDR_OFF (ph_off,ehdr) {
		Elf32_Phdr *ph = (Elf32_Phdr*)(binary + ph_off);
		if (ph->p_type == PT_LOAD) {
			panic_on(elf_load_seg(ph,binary + ph->p_offset, load_icode_mapper,e));
		}
	}
	e->env_tf.sepc = ehdr->e_entry;
}

struct Env* env_create(const void* binary, size_t size, int priority) {
	struct Env* e;
	env_alloc(&e,0);
	e->env_pri = priority;
	e->env_status = ENV_RUNNABLE;
	load_icode(e,binary,size);
	reflect_pgdir(e);	
	TAILQ_INSERT_HEAD(&env_sched_list,e,env_sched_link);	
	return e;
}

void env_free(struct Env* e) {
	Pte* pt;
	u_int pdeno,pteno,pa;
	int flag = 0;
	printk("[%08x] free env %08x \n",curenv?curenv->env_id:0,e->env_id);
	for (pdeno = 0; pdeno < PDX(UTOP); ++pdeno) {
		flag = 0;
		if (!(e->env_pgdir[pdeno] & PTE_V)) {
			continue;
		}
		pa = PADDR(PTE_ADDR(e->env_pgdir[pdeno]));
		pt = (Pte*)pa;
		for (pteno = 0; pteno <= PTX(~0);++pteno) {
			if (pt[pteno] & PTE_LIBRARY) {
				flag = 1;
				continue;
			}
			if (pt[pteno] & PTE_V) {
				page_remove(e->env_pgdir,e->env_asid,
						(pdeno <<  (PTSHIFT + PGSHIFT)) | (pteno << PGSHIFT));
			}
		}
		if (flag == 0) {
			e->env_pgdir[pdeno] = 0;
			page_decref(addr2page(pa));
			SET_TLB_FLUSH(UVPT + (pdeno << PGSHIFT),e->env_asid,0);
		}
	}
	
	page_decref(addr2page((u_long)(e->env_pgdir)));	
	asid_free(e->env_asid);
	SET_TLB_FLUSH(UVPT + (PDX(UVPT) << PGSHIFT),e->env_asid,0);
	e->env_status = ENV_FREE;
	LIST_INSERT_HEAD(&env_free_list,e,env_link);
	TAILQ_REMOVE(&env_sched_list,e,env_sched_link);

}

void env_destroy(struct Env* e) {
	env_free(e);
	if (curenv == e) {
		curenv = NULL;
		printk("i am killed ... \n");
		schedule(1);
	}
}

extern void env_pop_tf(struct Trapframe* tf) __attribute__((noreturn));


void env_run(struct Env* e) {
	assert(e->env_status == ENV_RUNNABLE);
	
	if(curenv) {
		curenv->env_tf = *((struct Trapframe*)RD_SSCRATCH());
	}
	curenv = e;
	curenv->env_runs++;
	cur_pgdir = curenv->env_pgdir;
	//printk("%08x  --- \n",curenv->env_id);
	//Pte* t1;
 	//struct Page* ppp1 =  page_lookup(cur_pgdir,0x4000000,&t1);	
	//printk("----%08x\n",*((u_int*)page2addr(ppp1)));
	//panic("test for 0x4000000,----\n");
	SET_TLB_FLUSH(0,curenv->env_asid,1);
	SET_SATP(1,curenv->env_asid,(unsigned long)cur_pgdir);	

	env_pop_tf((&(curenv->env_tf)));
}








void env_check() {
	struct Env* pe, *pe0,*pe1,*pe2;
	struct Env_list f1;
	u_long page_addr;

	pe0 = pe1 = pe2 = 0;
	assert((env_alloc(&pe0,0) == 0));
	assert((env_alloc(&pe1,0) == 0));
	assert((env_alloc(&pe2,0) == 0));
	assert(pe0);
	assert(pe1 && pe1 != pe0);
	assert(pe2 && pe2 != pe0 && pe2 != pe1);
	f1 = env_free_list;
	LIST_INIT(&env_free_list);
	assert((env_alloc(&pe,0) == -E_NO_FREE_ENV));
	env_free_list = f1;
	printk("pe0->env_id %d\n",pe0->env_id);
	printk("pe1->env_id %d\n",pe1->env_id);
	printk("pe2->env_id %d\n",pe2->env_id);
	printk("env_init() works well!\n");
	for (page_addr = 0; page_addr < npage * sizeof(struct Page);page_addr += BY2PG ) {
		assert(va2pa(base_pgdir,UPAGES + page_addr) == (unsigned long)pages + page_addr);	
	}
	for (page_addr = 0; page_addr < NENV * sizeof(struct Env);page_addr += BY2PG ) {
		assert(va2pa(base_pgdir,UENVS + page_addr) == (unsigned long)envs + page_addr);	
	}
	printk("pe1->env_pgdir %x\n",pe1->env_pgdir);
	assert(pe2->env_pgdir[PDX(UTOP)] == base_pgdir[PDX(UTOP)]);
	printk("env_setup_vm passed!\n");
	printk("pe2's sp register %x\n",pe2->env_tf.regs[2]);

	TAILQ_INSERT_TAIL(&env_sched_list,pe0,env_sched_link);
	TAILQ_INSERT_TAIL(&env_sched_list,pe1,env_sched_link);
	TAILQ_INSERT_TAIL(&env_sched_list,pe2,env_sched_link);

	env_free(pe2);
	env_free(pe1);
	env_free(pe0);
	printk("env_check() succeeded!\n");
}

void envid2env_check() {
	struct Env* pe,*pe0,*pe2;
	assert(env_alloc(&pe0,0) == 0);
	assert(env_alloc(&pe2,0) == 0);
	int re;
	pe2->env_status = ENV_FREE;
	re = envid2env(pe2->env_id,&pe,0);
	assert(re == -E_BAD_ENV);
	pe2->env_status = ENV_RUNNABLE;
	re = envid2env(pe2->env_id,&pe,0);
	assert(re == 0 && pe->env_id == pe2->env_id);

	curenv = pe0;
	re = envid2env(pe2->env_id,&pe,1);
	assert(re == -E_BAD_ENV);
	printk("envid2env() works well!\n");

}

