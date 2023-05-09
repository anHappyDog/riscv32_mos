#include <asm/asm.h>

LEAF(mecall)
	addi sp,sp,-4
	sw   ra,(sp)
	ecall
	lw   ra,(sp)
	addi sp,sp,4
	ret	
END(mecall)
	
