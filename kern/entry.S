#include <asm/asm.h>
#include <stackframe.h>

.section .text.exc_gen_entry
exc_gen_entry:
	SAVE_ALL
	csrr t0,scause
	srl  t1,t0,31
	andi t1,t1,0x1
	beqz t1,not_interrupt
is_interrupt:
	sll  t0,t0,1
	srl  t0,t0,1
	addi t0,t0,16
not_interrupt:
	mv t1,t0
	la 	 t0, exception_handlers
	sll  t1,t1,2
	add t0,t1,t0
	lw  t0, (t0)	
	jr   t0
