#include<asm/embdasm.h>
#include<drivers/virtio_disk.h>
#include<pmap.h>
#include<printk.h>
#include<mmu.h>
#include<env.h>
static u_long memsize;
u_long npage;

struct Page* pages;
static u_long freemem;

Pde* cur_pgdir;
Pde* root_pgdir;

struct Page_list page_free_list;

void riscv32_detect_memory() {
//	memsize = csr_read(CSR_HGATP); 
	memsize =  128 * 1024 * 1024;
	npage = memsize / BY2PG ;
	printk("Memory size: %lu bytes, number of pages: %lu\n",memsize,npage);
}

void* alloc(u_int n ,u_int align, int clear) {
	extern char end[];
	u_long alloced_mem;
	if (freemem == 0) {
		freemem = (u_long)end;
	}
	freemem = ROUND(freemem,align);
	alloced_mem = freemem;
	freemem += n;
	panic_on(freemem >= memsize + 0x80000000);
	if (clear) {
		memset((void*)alloced_mem,0,n);
	}
	return (void*)alloced_mem;
}

void page_init() {

	pages = (struct Page*)alloc(npage * sizeof(struct Page),BY2PG,1);
		
	LIST_INIT(&page_free_list);
	freemem = ROUND(freemem,BY2PG);
	for (int i = 0; i < npage; ++i) {
		if (page2addr(&pages[i]) < freemem) {
			pages[i].pp_ref = 1;
		}
		else {
			pages[i].pp_ref = 0;
			LIST_INSERT_HEAD(&page_free_list,&pages[i],pp_link);
		}
	}
	//printk("------------------------------------------------------------\n");
	//printk("to memory %x for struct Pages.\n",freemem);
	//printk("page_init success\n");


}

// alloc n pages which are sequent.
int page_alloc_sequent(struct Page** new, int n) {
	struct Page*pp,*first = NULL,*prev = NULL;
	if (LIST_EMPTY(&page_free_list)) {
		return -E_NO_MEM;
	}
	int flag = 0,cnt = 0;
	LIST_FOREACH(pp,&page_free_list,pp_link) {
		if (cnt > 0) {
			if (page2addr(pp) != page2addr(prev) - BY2PG) {
				cnt = 0;
				first = pp;
			}
		}
		else {
			first = pp;
		}
		prev = pp;
		++cnt;
		if (cnt == n) {
			flag = 1;
			break;
		}
	}
	if (!flag) {
		return -E_NO_MEM;
	}
	for (struct Page* t = first;t != pp->pp_link.le_next; t = t->pp_link.le_next) {
		LIST_REMOVE(t,pp_link);
		memset((void*)page2addr(t),0,BY2PG);
		t->pp_ref += 1;
	}
	*new = pp;
	return 0;
}


int page_alloc(struct Page** new) {
	struct Page* pp;
	if (LIST_EMPTY(&page_free_list)) {
		return -E_NO_MEM;
	}
	pp = LIST_FIRST(&page_free_list);
	LIST_REMOVE(pp,pp_link);
	//printk("pp->pp_ref is %08x\n",pp->pp_ref);
	//pp->pp_ref = 0;
	memset((void*)page2addr(pp),0,BY2PG);
	*new = pp;
	return 0;
}

void page_free(struct Page* pp) {
	assert(pp->pp_ref == 0);
	LIST_INSERT_HEAD(&page_free_list,pp,pp_link);
}

void page_decref(struct Page* pp) {
	assert(pp->pp_ref > 0);
	if (--pp->pp_ref == 0) {
		page_free(pp);
	}
}


void page_remove(Pde* pgdir, u_int asid, u_long va) {
	Pte* pte;
	struct Page* pp = page_lookup(pgdir,va,&pte);
	if (pp == NULL) {
		return;
	}
	page_decref(pp);
	*pte = 0;
	SET_TLB_FLUSH(va,asid,0);
}


int pgdir_init_fill(Pde* pgdir,u_long va,u_long pa,u_long perm) {
	Pde* pgdir_entryp;
	Pte* pte;
	struct Page* pp;
	//LIST_REMOVE(p,pp_link);
	pgdir_entryp = pgdir + PDX(va);
	if (!((*pgdir_entryp) & PTE_V)) {
		try(page_alloc(&pp));
		*pgdir_entryp = page2ptx(pp) | PTE_V;
		pp->pp_ref += 1;
		pgdir_init_fill(pgdir,page2addr(pp),page2addr(pp),PTE_R | PTE_W);
	}
	pte = (Pte*)PADDR(PTE_ADDR(*pgdir_entryp)) + PTX(va);
	*pte = ((pa >> 12) << 10) | PTE_V | perm;
	return 0;
}



static int pgdir_walk(Pde* pgdir, u_long va, int create, Pte** ppte) {
	Pde* pgdir_entryp;
	struct Page* pp;
	pgdir_entryp = pgdir + PDX(va);	
	if (!((*pgdir_entryp) & PTE_V)) {
		if (create != 0) {
			if (page_alloc(&pp) == -E_NO_MEM) {
				return -E_NO_MEM;
			}
			*pgdir_entryp = page2ptx(pp) |  PTE_V;
			pp->pp_ref += 1;
			pgdir_init_fill(pgdir,page2addr(pp),page2addr(pp),PTE_W | PTE_R);
			//	page_insert(pgdir,curenv->env_asid,pp,page2addr(pp), PTE_R | PTE_W);
		}
		else {
			*ppte = 0;
			return 0;
		}
	}
	*ppte = (Pte*)PADDR(PTE_ADDR(*pgdir_entryp)) + PTX(va);
	return 0;
}

struct Page* page_lookup(Pde* pgdir, u_long va, Pte** ppte) {
	struct Page* pp;
	Pte* pte;
	pgdir_walk(pgdir, va,0,&pte);
	if (pte == NULL || (*pte & PTE_V) == 0) {
		return NULL;
	}
	pp = addr2page(((*pte) >> 10) << 12 );
	if (ppte) {
		*ppte = pte;
	}
	return pp;
}

int page_insert(Pde* pgdir, u_int asid, struct Page* pp, u_long va, u_int perm) {
	Pte* pte;
	pgdir_walk(pgdir,va,0,&pte);
	if(pte && (*pte & PTE_V)) {
		if (addr2page(PADDR(PTE_ADDR(*pte))) != pp) {
			page_remove(pgdir,asid,va);
		}
		else {
			SET_TLB_FLUSH(va,asid,0);
			*pte = page2ptx(pp) | perm | PTE_V;
			return 0;
		}
	}
	SET_TLB_FLUSH(va,asid,0);
	try(pgdir_walk(pgdir,va,1,&pte));
	*pte = page2ptx(pp) | perm | PTE_V;
	pp->pp_ref += 1;
	return 0;	

}


//only for device
int pgdir_map(Pde* pgdir, u_int asid, u_long pa, u_long va, u_int perm) {
	Pte* pte;
	try(pgdir_walk(pgdir,va,1,&pte));
	*pte = ((pa >> 12) << 10) | perm | PTE_V;
	SET_TLB_FLUSH(va,asid,0);
	return 0;
}


int pgdir_init() {
	Pde* pgdir;
	struct Page *pp0;
    try(page_alloc(&pp0));	
	pgdir = (Pde*)page2addr(pp0);
	pp0->pp_ref = 1;
/*	for (int i = 0; i < npage; ++i) {
	//printk("the address is 0x%08x : 0x%08x\n",PPN2VA(i),page2addr(&pages[i]));
		try(pgdir_init_fill(pgdir,page2addr(&pages[i]),&pages[i]));
		++cnt;
	}*/
	extern struct Env envs[NENV];
	for (u_long addr = KERNSTART; addr < ROUNDDOWN((u_long)envs,BY2PG); addr += BY2PG) {
 		try(pgdir_init_fill(pgdir,addr,addr,PTE_R | PTE_X));
	}
	for (u_long addr = ROUNDDOWN((u_long)envs,BY2PG); addr < KERNEND; addr += BY2PG) {
		try(pgdir_init_fill(pgdir,addr,addr,PTE_R | PTE_W));
	}
	for (u_long addr = KERNEND; addr < ROUNDDOWN((u_long)pages,BY2PG); addr += BY2PG) {
		try(pgdir_init_fill(pgdir,addr,addr,PTE_R | PTE_X));
	}
	for (u_long addr = ROUNDDOWN((u_long)pages,BY2PG); addr < 0X88000000; addr += BY2PG) {
		try(pgdir_init_fill(pgdir,addr,addr,PTE_R | PTE_W));
	}
	try(pgdir_map(pgdir,0,DISK_ADDRESS - KERNSTART,DISK_ADDRESS,PTE_R | PTE_W));
	//try(pgdir_init_fill(pgdir,DEV_DISK_REGADDRESS,,PTE_R | PTR_W);
	cur_pgdir = pgdir;	
	root_pgdir = pgdir;
	SET_SATP(1,0,((unsigned long)pgdir));	
	SET_TLB_FLUSH(0,0,1);
	/*printk("------------------------------------------------------------------\n");
	printk("root pgdir address is 0x%08x\n",pgdir);
	printk("pgdir_init :   pgdir_init successfulls!\n");	
	printk("------------------------------------------------------------------\n");
	*/return 0;
}


void physical_memory_manage_check() {
	struct Page* pp,*pp0,*pp1,*pp2;
	struct Page_list f1;
	int *tmp;

	pp0 = pp1 = pp2 = 0;
	assert(page_alloc(&pp0) == 0);
	assert(page_alloc(&pp1) == 0);
	assert(page_alloc(&pp2) == 0);

	assert(pp0);
	assert(pp1 && pp0 != pp1);
	assert(pp2 && pp2 != pp1 && pp2 != pp0);

	f1 =page_free_list;
	LIST_INIT(&page_free_list);
	assert(page_alloc(&pp) == -E_NO_MEM);

	tmp = (int*)page2addr(pp0);
	*tmp = 1000;
	page_free(pp0);

	printk("The number in address tmp is %d\n",*tmp);

	assert(page_alloc(&pp0) == 0);
	assert(pp0);

	assert(tmp == (int*)page2addr(pp0));
	assert(*tmp == 0);

	page_free_list = f1;

	page_free(pp0);
	page_free(pp1);
	page_free(pp2);
	
	struct Page_list test_free;
	struct Page* test_pages;
	test_pages = (struct Page*)alloc(10 * sizeof(struct Page),BY2PG,1);
	LIST_INIT(&test_free);
	int i,j = 0;
	struct Page *p,*q;
	for (i = 9;i>=0;--i) {
		test_pages[i].pp_ref = i;
		LIST_INSERT_HEAD(&test_free,&test_pages[i],pp_link);
	}
	
	p = LIST_FIRST(&test_free);
	int answer1[] = {0,1,2,3,4,5,6,7,8,9};
	assert(p != NULL);
	while (p != NULL) {
		assert(p->pp_ref == answer1[j++]);
		p = LIST_NEXT(p,pp_link);
	}
	
	int answer2[] = {0,1,2,3,4,20,5,6,7,8,9};
	q = (struct Page*) alloc(sizeof(struct Page),BY2PG,1);
    q->pp_ref = 20;	
	
	LIST_INSERT_AFTER(&test_pages[4],q,pp_link);
	p = LIST_FIRST(&test_free);

	j = 0;
	while (p != NULL) {
		assert(p->pp_ref == answer2[j++]);
		p = LIST_NEXT(p,pp_link);
	}

	printk("physical_memory_manage_check() succeeded\n");
}
