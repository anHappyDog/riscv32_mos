#ifndef __BARRIER_H_
#define __BARRIER_H_

#define RISCV_FENCE(p,s)			\
		__asm__ __volatile__ ("fence " #p "," #s ::: "memory")

#define mb() 			RISCV_FENCE(iorw, iorw)
#define rmb()			RISCV_FENCE(ir, ir)
#define wmb() 			RISCV_FENCE(ow,ow)

#define __smp_mb() 		RISCV_FENCE(rw,rw)
#define __smp_rmb()		RISCV_FENCE(r,r)
#define __smp_wmb() 	RISCV_FENCE(w,w)


#define smp_mb__after_spinlock()		RISCV_FENCE(rw,rw)

#endif
