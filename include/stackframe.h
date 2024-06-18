#include <asm/asm.h>
#include <mmu.h>
#include <trap.h>

// clang-format off
.macro SAVE_ALL
	csrrw sp, sscratch ,sp
	bnez  sp, trap_from_user
trap_from_kernel:
		csrr sp, sscratch
trap_from_user:
		addi sp,sp, -TF_SIZE
		sw zero,    REG_ZERO(sp)
		sw ra,      REG_RA(sp)
		sw gp,      REG_GP(sp)
   		sw tp, 		REG_TP(sp)
		sw t0,		REG_T0(sp)
		sw t1, 		REG_T1(sp)
		sw t2,		REG_T2(sp)
		sw fp,		REG_FP(sp)
		sw s1,		REG_S1(sp)
		sw a0,		REG_A0(sp)
		sw a1,		REG_A1(sp)
		sw a2,		REG_A2(sp)
		sw a3, 		REG_A3(sp)
		sw a4,		REG_A4(sp)
		sw a5,		REG_A5(sp)
		sw a6,		REG_A6(sp)
		sw a7,		REG_A7(sp)
		sw s2,		REG_S2(sp)
		sw s3,		REG_S3(sp)
		sw s4, 		REG_S4(sp)
		sw s5, 		REG_S5(sp)
		sw s6,		REG_S6(sp)
		sw s7,		REG_S7(sp)
		sw s8,		REG_S8(sp)
		sw s9,		REG_S9(sp)
		sw s10,		REG_S10(sp)
		sw s11,		REG_S11(sp)
		sw t3,		REG_T3(sp)
		sw t4,		REG_T4(sp)
		sw t5,		REG_T5(sp)
		sw t6,		REG_T6(sp)
		
	//	csrrw s0,sscratch sp
		csrrw s0,sscratch, sp
		csrr  s1, sstatus
		csrr  s2, sepc
		csrr  s3, stval
		csrr  s4, scause
		sw  s0,     REG_SP(sp)
		sw  s1,	    REG_STATUS(sp)
		sw  s2,	    REG_SEPC(sp)
		sw  s3,     REG_STVAL(sp)
		sw  s4,		REG_SCAUSE(sp)
		
.endm


.macro RESTORE_ALL
	lw  s1, REG_STATUS(sp)
	lw  s2, REG_SEPC(sp)
	andi s0, s1, 1<<8
	bnez s0, _to_kernel
_to_user:
	addi s0,sp, TF_SIZE
	csrw sscratch,s0
_to_kernel:
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
.endm




