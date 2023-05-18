#ifndef _PMAP_H_
#define _PMAP_H_

#include <mmu.h>
#include <printk.h>
#include <queue.h>
#include <types.h>

extern Pde *cur_pgdir;

LIST_HEAD(Page_list, Page);
typedef LIST_ENTRY(Page) Page_LIST_entry_t;

struct Page {
	Page_LIST_entry_t pp_link; /* free list link */
	u_short pp_ref;
};

extern struct Page *pages;
extern struct Page_list page_free_list;

static inline u_long page2ppn(struct Page *pp) {
	return pp - pages;
}

static inline u_long page2sft(struct Page *pp) {
	return page2ppn(pp) << PGSHIFT;
}

static inline struct Page *addr2page(u_long addr) {
	if (PPN(addr) >= npage) {
		panic("pa2page called with invalid ahdress: %x", addr);
	}
	return &pages[PPN(addr)];
}

static inline u_long page2addr(struct Page *pp) {
	return page2sft(pp) + KERNSTART;
}

static inline u_long page2ptx(struct Page *pp) {
	return ((page2addr(pp) >> PGSHIFT) << PTSHIFT);
}

static inline struct Page* pa2page(u_long pa) {
	return addr2page((pa >> PTSHIFT) << PGSHIFT);
}

static inline u_long va2pa(Pde *pgdir, u_long va) {
	Pte *p;

	pgdir = &pgdir[PDX(va)];
	if (!(*pgdir & PTE_V)) {
		return ~0;
	}
	p = (Pte *)PADDR(PTE_ADDR(*pgdir));
	if (!(p[PTX(va)] & PTE_V)) {
		return ~0;
	}
	return PADDR(PTE_ADDR(p[PTX(va)]));
}

void riscv32_detect_memory(void);
void riscv32_init(void);
void page_init(void);
void *alloc(u_int n, u_int align, int clear);

int pgdir_init();

int page_alloc(struct Page **pp);
void page_free(struct Page *pp);
void page_decref(struct Page *pp);
int page_insert(Pde *pgdir, u_int asid, struct Page *pp, u_long va, u_int perm);
struct Page *page_lookup(Pde *pgdir, u_long va, Pte **ppte);
void page_remove(Pde *pgdir, u_int asid, u_long va);
int pgdir_map(Pde* pgdir, u_int asid, u_long pa, u_long va, u_int perm);
int page_alloc_sequent(struct Page** newp, int n);

extern struct Page *pages;

void physical_memory_manage_check(void);
void page_check(void);

#endif /* _PMAP_H_ */
