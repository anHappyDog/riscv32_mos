#ifndef _TRAP_H_
#define _TRAP_H_

#ifndef __ASSEMBLER__

#include <types.h>

struct Trapframe {

	unsigned long regs[32];

	/* Saved special registers. */
	unsigned long sstatus;
	unsigned long sepc;
	unsigned long stval;
	unsigned long scause;
	unsigned long sscratch;
};

void print_tf(struct Trapframe *tf);

#endif 

#define REG_ZERO 0
#define REG_RA (REG_ZERO + 4)
#define REG_SP (REG_RA + 4)
#define REG_GP (REG_SP + 4)
#define REG_TP (REG_GP + 4)
#define REG_T0 (REG_TP + 4)
#define REG_T1 (REG_T0 + 4)
#define REG_T2 (REG_T1 + 4)
#define REG_FP (REG_T2 + 4)
#define REG_S1 (REG_FP + 4)
#define REG_A0 (REG_S1 + 4)
#define REG_A1 (REG_A0 + 4)
#define REG_A2 (REG_A1 + 4)
#define REG_A3 (REG_A2 + 4)
#define REG_A4 (REG_A3 + 4)
#define REG_A5 (REG_A4 + 4)
#define REG_A6 (REG_A5 + 4)
#define REG_A7 (REG_A6 + 4)
#define REG_S2 (REG_A7 + 4)
#define REG_S3 (REG_S2 + 4)
#define REG_S4 (REG_S3 + 4)
#define REG_S5 (REG_S4 + 4)
#define REG_S6 (REG_S5 + 4)
#define REG_S7 (REG_S6 + 4)
#define REG_S8 (REG_S7 + 4)
#define REG_S9 (REG_S8 + 4)
#define REG_S10 (REG_S9 + 4)
#define REG_S11 (REG_S10 + 4)
#define REG_T3 (REG_S11 + 4)
#define REG_T4 (REG_T3 + 4)
#define REG_T5 (REG_T4 + 4)
#define REG_T6 (REG_T5 + 4)


#define REG_STATUS (REG_T6 + 4)
#define REG_SEPC (REG_STATUS + 4)
#define REG_STVAL (REG_SEPC + 4)
#define REG_SCAUSE (REG_STVAL + 4)
#define REG_SSCRATCH (REG_SCAUSE + 4)
#define TF_SIZE (REG_SSCRATCH + 4)












#endif
