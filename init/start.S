#include <asm/asm.h>
#include <mmu.h>

.section .boot
EXPORT(_start)
.option pic
	//csrw mstatus,zero
	li sp, 0x80600000
	
	jal riscv32_init
