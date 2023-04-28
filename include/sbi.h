#ifndef _SBI_H_
#define _SBI_H_


#define SBI_SET_TIMER 0
#define SBI_CONSOLE_PUTCHAR 1
#define SBI_SET_SHUTDOWN 8

#define SBI_ECALL(__num, __a0, __a1, __a2)  \
({                                          \
 	register unsigned long a0 asm("a0") = (unsigned long)(__a0); \
	register unsigned long a1 asm("a1") = (unsigned long)(__a1); \
	register unsigned long a2 asm("a2") = (unsigned long)(__a2); \
	register unsigned long a7 asm("a7") = (unsigned long)(__num); \
 	asm volatile("ecall"										\
				: "+r"(a0)										\
				: "r"(a1),"r"(a2),"r"(a7)						\
				: "memory");									\
 	a0;															\
})


#define SBI_ECALL_0(__num) SBI_ECALL(__num,0,0,0);
#define SBI_ECALL_1(__num,__a0) SBI_ECALL(__num,__a0,0,0)
#define SBI_PUTCHAR(__a0) SBI_ECALL_1(SBI_CONSOLE_PUTCHAR, __a0)
#define SBI_TIMER(__a0) SBI_ECALL_1(SBI_SET_TIMER,__a0)
#define SBI_SHUTDOWN() SBI_ECALL_0(SBI_SET_SHUTDOWN)
#endif
