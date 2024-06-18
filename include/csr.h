#define CSR_HGATP 0X680



#define csr_read(csr)						\
	({								\
	 	register unsigned long __v;				\
			__asm__ __volatile__ ("csrr %0, " __ASM_STR(csr)	\
								      : "=r" (__v) :			\
		    							 : "memory");			\
								__v;							\
})
