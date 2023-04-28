#include <asm/asm.h>
#include <stackframe.h>

.section .exc_gen_entry
exc_gen_entry:
	SAVE_ALL
	csrr t0,scause
	andi t0,	0x7c 
	lw 	 t0, exception_handlers(t0)
	jr   t0
