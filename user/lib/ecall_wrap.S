#include <asm/asm.h>

LEAF(mecall)
	ecall
	jr ra
END(mecall)
	
