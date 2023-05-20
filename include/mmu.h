#ifndef _MMU_H_
#define _MMU_H_

#include <error.h>

#define BY2PG 4096		// bytes to a page
#define PDMAP (4 * 1024 * 1024) // bytes mapped by a page directory entry
#define PGSHIFT 12
#define PDSHIFT 20 
#define PTSHIFT 10
#define PDX(va) ((((u_long)(va)) >> 22) & 0x03FF)
#define PTX(va) ((((u_long)(va)) >> 12) & 0x03FF)
#define PPN2VA(n) (((n >> PTSHIFT) << (PGSHIFT + PTSHIFT)) | ((n % 1024)<< PGSHIFT))
#define PTE_ADDR(pte) (((pte) & ~0x3ff) & ~(0x3 << 30))

#define PPN(addr) ((addr - KERNSTART) >> PGSHIFT)
#define VPN(va)  (((u_long)(va)) >> PGSHIFT)

/* Page Table/Directory Entry flags */

#define PTE_DIRTY (1 << 30)
#define PTE_LIBRARY (1 << 9) //1 if the PTE is shared by father and child's process
#define PTE_COW (1 << 8) //1 if the PTE need to be copied when write
#define PTE_D (1 << 7) //1 if the PTE is written after D is reset
#define PTE_A (1 << 6) //1 if the PTE is accessed after A is reset
#define PTE_G (1 << 5) //1 if the PTE is valuable to the whole address space
#define PTE_U (1 << 4) //1 if the PTE is a user page
#define PTE_X (1 << 3) //1 if the PTE can be executed
#define PTE_W (1 << 2) //1 if the PTE can be written
#define PTE_R (1 << 1) //1 if the PTE can be read
#define PTE_V (1 << 0) //1 if the PTE is valuable



#define KERNSTART 0x80000000
#define KERNBASE 0x80200000
#define KERNEND 0x80600000


#define KSTACKTOP 0x80600000
#define ULIM 0x80000000
#define UVPT 0x7fc00000
#define UPAGES 0x7f800000
#define UENVS 0x7f400000
#define UTOP 0x7f400000
#define UXSTACKTOP UTOP
#define USTACKTOP (UTOP - 2 * BY2PG)
#define UTEXT PDMAP
#define UCOW (UTEXT - BY2PG)
#define UTEMP (UCOW - BY2PG)


#define GET_VPT(pgdir,va) 				\
({										\
	Pde* entryp = (Pde*)pgdir + PDX(va);\
	user_assert(entryp != 0);			\
	Pte* pte = (Pte*)((*entryp >> 10) << 12) + PTX(va);	\
	user_assert(pte != 0);				\
	*pte;								\
})

#define IS_VAL(pgdir,va)				\
({										\
	Pte pte = GET_VPT(pgdir,va);	    \
	pte & PTE_V; 						\
})


#ifndef __ASSEMBLER__

#include <string.h>
#include <types.h>

extern u_long npage;

typedef u_long Pde;
typedef u_long Pte;

// turn address between virtual and physical
//need to do

#define assert(x)                                                                                  \
	do {                                                                                       \
		if (!(x)) {                                                                        \
			panic("assertion failed: %s", #x);                                         \
		}                                                                                  \
	} while (0)

#define PADDR(pte) 			\
	({						\
	 	u_long tt = (u_long)(pte);	\
		((tt) >> PTSHIFT) << PGSHIFT;		\
	})

#define TRUP(_p) 											\
	({														\
	 	typeof((_p)) __m_p = (_p);							\
		(u_int) __m_p > ULIM ? (typeof(_p))ULIM : __m_p;	\
	})


#endif //!__ASSEMBLER__
#endif // !_MMU_H_
