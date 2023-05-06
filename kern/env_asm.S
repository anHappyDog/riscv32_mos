#include <asm/asm.h>
#include <mmu.h>
#include <trap.h>

.text 
LEAF(env_pop_tf)
.option pic
	move sp,a0
	j ret_from_exception
END(env_pop_tf)
