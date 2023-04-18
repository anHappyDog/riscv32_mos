#include <asm/asm.h>
#include <mmu.h>

.section .boot
EXPORT(_start)
.option pic
	/* disable interrupts */
	//mtc0    zero, CP0_STATUS
	//csrw mstatus,zero
	/* hint: you can reference the memory layout in include/mmu.h */
	/* set up the kernel stack */
	/* Exercise 1.3: Your code here. (1/2) */
	li sp, 0x80600000
	
	/* jump to mips_init */
	/* Exercise 1.3: Your code here. (2/2) */
	jal mips_init
