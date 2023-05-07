#include <asm/asm.h>
#include <mmu.h>
#include <trap.h>

.text 
LEAF(env_pop_tf)
.option pic
	csrw sscratch,sp
	move sp,a0
	#j ret_from_exception
	lw  s1, REG_STATUS(sp)
	lw  s2, REG_SEPC(sp)
	csrw sstatus, s1
	csrw sepc   , s2
	lw ra, REG_RA(sp)
	lw gp, REG_GP(sp)
	lw tp, REG_TP(sp)
	lw t0, REG_T0(sp)
	lw t1, REG_T1(sp)
	lw t2, REG_T2(sp)
	lw fp, REG_FP(sp)
	lw s1, REG_S1(sp)
	lw a0, REG_A0(sp)
	lw a1, REG_A1(sp)
	lw a2, REG_A2(sp)
	lw a3, REG_A3(sp)
	lw a4, REG_A4(sp)
	lw a5, REG_A5(sp)
	lw a6, REG_A6(sp)
	lw a7, REG_A7(sp)
	lw s2, REG_S2(sp)
	lw s3, REG_S3(sp)
	lw s4, REG_S4(sp)
	lw s5, REG_S5(sp)
	lw s6, REG_S6(sp)
	lw s7, REG_S7(sp)
	lw s8, REG_S8(sp)
	lw s9, REG_S9(sp)
	lw s10, REG_S10(sp)
	lw s11, REG_S11(sp)
	lw t3, REG_T3(sp)
	lw t4, REG_T4(sp)
	lw t5, REG_T5(sp)
	lw t6, REG_T6(sp)

	lw sp, REG_SP(sp)

	sret

END(env_pop_tf)
