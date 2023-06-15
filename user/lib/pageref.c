#include <lib.h>

int pageref(void* v) {
	u_int pte;
	if (!(vpd[PDX(v)] & PTE_V)) {
		return 0;
	}
	pte = vpt[(u_long)(v) >> PGSHIFT];
	if (!(pte & PTE_V)) {
		return 0;
	}
	//debugf("pte is %08x,the real pageref is %08x,pageref is %08x\n",PADDR(PTE_ADDR(pte)),ecall_get_pgref(PADDR(PTE_ADDR(pte))),pages[PPN(PADDR(PTE_ADDR(pte)))].pp_ref);
	return pages[PPN(PADDR(PTE_ADDR(pte)))].pp_ref;
	
}
