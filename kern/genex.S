#include <asm/asm.h>
#include <stackframe.h>

.macro BUILD_HANDLER exception handler
NESTED(handle_\exception, TF_SIZE + 8, zero) 
	mv      a0, sp
	addi    sp,sp,-8
	jal 	\handler
	addi    sp,sp,8
	j       ret_from_exception
END(handle_\exception)
.endm


FEXPORT(ret_from_exception) 
	RESTORE_ALL
	/*addi sp,sp,-4
	sw   t0, (sp)
	csrr t0, sepc
	addi t0,t0,4
	csrw sepc, t0
	lw  t0,(sp)
	addi sp,sp,4*/
	sret


BUILD_HANDLER load_page do_load_page
BUILD_HANDLER ecall_from_u do_ecall_from_u
BUILD_HANDLER reserved do_reserved
BUILD_HANDLER instruction_page do_instruction_page
BUILD_HANDLER store_page do_store_page
BUILD_HANDLER software do_software_int
BUILD_HANDLER timer do_timer_int
