#include<pmap.h>
#include<printk.h>
#include<mmu.h>

static u_long memsize;
u_long npage;

struct Page* pages;
static u_long freemem;

struct Page_list page_free_list;

void mips_detect_memory() {
//	memsize = csr_read(CSR_HGATP); 
	memsize =  64 * 1024 * 1024;
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

void mips_vm_init() {
	pages = (struct Page*)alloc(npage * sizeof(struct Page),BY2PG,1);
	printk("to memory %x for struct Pages.\n",freemem);
	printk("pmap.c:\t mips vm init success\n");
}

void page_init() {
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
}

int page_alloc(struct Page** new) {
	struct Page* pp;
	if (LIST_EMPTY(&page_free_list)) {
		return -E_NO_MEM;
	}
	pp = LIST_FIRST(&page_free_list);
	LIST_REMOVE(pp,pp_link);
	memset((void*)page2addr(pp),0,BY2PG);
	*new = pp;
	return 0;
}

void page_free(struct Page* pp) {
	assert(pp->pp_ref == 0);
	LIST_INSERT_HEAD(&page_free_list,pp,pp_link);
}
static int pgdir_init_fill(Pde* pgdir,u_long va,struct Page* p) {
	Pde* pgdir_entryp;
	Pte* pte;
	struct Page* pp;
	pgdir_entryp = pgdir + PDX(va);
	if (!((*pgdir_entryp) & PTE_V)) {
		try(page_alloc(&pp));
		*pgdir_entryp = page2ptx(pp) | PTE_V;
		pp->pp_ref += 1;
	}
	
	pte = (Pte*)PADDR(PTE_ADDR(*pgdir_entryp)) + PTX(va);
	*pte = page2ptx(p) | PTE_V | PTE_R | PTE_W | PTE_X;
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
			*pgdir_entryp = page2ptx(pp) | PTE_V;
			pp->pp_ref += 1;
		}
		else {
			*ppte = 0;
			return 0;
		}
	}
	//printk("0x%08x\n",PTE_ADDR(*pgdir_entryp));
	//printk("0x%08x\n",VADDR(PTE_ADDR(*pgdir_entryp)));
	*ppte = (Pte*)PADDR(PTE_ADDR(*pgdir_entryp)) + PTX(va);
	return 0;
}

int pgdir_init() {
	Pde* pgdir;
	struct Page *pp0;
    try(page_alloc(&pp0));	
	pgdir = (Pde*)page2addr(pp0);
	pp0->pp_ref = 1;
	int cnt = 0;
	for (int i = 0; page2addr(&pages[i]) < KERNEND; ++i) {
	//printk("the address is 0x%08x : 0x%08x\n",PPN2VA(i),page2addr(&pages[i]));
		try(pgdir_init_fill(pgdir,page2addr(&pages[i]),&pages[i]));
		++cnt;
	}
	//try(pgdir_init_fill(pgdir,PPN2VA(page2ppn(pp0)),pp0));
	
	asm ("csrw satp, %0" : : "r"(((((unsigned long)pgdir) >> 12) | (1 << 31))));
	asm ("sfence.vma");
	printk("pgdir address is 0x%08x\n",pgdir);
	printk("pgdir_init :   %d pages of kernel has been filled into the pd!\n",cnt);	
	/*
	Pte* sd = (int*)(0x83ffe000);
	for (int i = 0; i < 1024; ++i) {
		printk(" ---   0x%08x \n", ((*(sd + i) >> 10 ) << 12));
	}
	*/
/*	
	Pde* sd = (int*)(0x83fff000);
	for (int i = 0; *(sd + i) != 0; ++i) {
		printk("----------- 0x%08x -------\n",*(sd + i));
		Pte* ttt = ((*(sd + i) >> 10 ) << 12);
		printk("address : 0x%08x\n",ttt);
		for (int j = 0;*(ttt + j) != 0; ++j) {
			printk("0x%08x\n",((*(ttt + j) >> 10 ) << 12));
		}
	}
/*	
	/*
	Pte* pte2;
	struct Page* pp1;
	try(page_alloc(&pp1));
	try(pgdir_init_fill(pgdir,PPN2VA(page2ppn(pp1)),pp1));
	int* t1 = (int*)page2addr(pp1);
	*t1 = 100;
	*(t1 + 1) = 200;
	pgdir_walk(pgdir,PPN2VA(page2ppn(pp1)),0,&pte2);	
	int * t2 =(int*)(((*pte2) >> 10 ) << 12);
	for (int i = 0; *(t2 + i) != 0; ++i) {
		printk("%d\n",*(t2 + i));
	}
	*/
	
	//return 0;	
	/*
		Pte* pte,*pte1;
	unsigned long* tt = (unsigned long*)pgdir;
    for (int i = 0; *(tt + i) != 0; ++i) {
		printk("0x%08x   %10b        %08x\n",*(tt + i),*(tt + i),((*(tt + i) >> 10) << 12));
	}
	printk("--------------\n");
	
	pgdir_walk(pgdir,0,0,&pte);
	pgdir_walk(pgdir,(1 << 22),0,&pte1);
	for (int i = 0; *(pte + i) != 0 && i < 1024 ; ++i) {
		printk("%d    :    0x%08x",i,((*(pte +i) >> 10) << 12));
		if (*(pte1 + i) != 0) {
			printk("            : 0x%08x",((*(pte1 + i) >> 10) << 12));
		}
		printk("\n");
	}
*/	
	return 0;
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
