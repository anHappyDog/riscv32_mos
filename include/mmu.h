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
#define PTE_ADDR(pte) ((pte) & ~0x3ff)

#define PPN(addr) ((addr - KERNSTART) >> PGSHIFT)

/* Page Table/Directory Entry flags */
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
		((pte) >> PTSHIFT) << PGSHIFT;		\
	})

#endif //!__ASSEMBLER__
#endif // !_MMU_H_
