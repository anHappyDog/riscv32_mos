#ifndef _EMBDASM_H_
#define _EMBDASM_H_
#define SET_SIE(seie,stie,ssie) 				   \
do {												\
	asm volatile ("csrw sie, %0" :: 				 \
		"r"((seie << 9) | (stie << 5) | (ssie << 1)));\
   } while(0)

#define SET_SATP(mode,asid,addr)							\
do {														\
	asm volatile ("csrw satp,%0" ::							\
	"r"((mode << 31) | (asid << 22) | (addr >> 12)));	    \
   } while(0)	

#define SET_TLB_FLUSH(addr,asid,isglobal)					\
do { 														\
	if (isglobal != 0){										\
		asm volatile ("sfence.vma");						\
	}														\
	else {													\
		asm volatile ("sfence.vma %0,%1" ::					\
		"r"(addr),"r"(asid));								\
	}														\
	} while(0)					

#define SET_STVEC(base,mode)								\
	do {													\
		asm volatile ("csrw stvec, %0" ::					\
		"r"((base & ~0x3) | mode));							\
	} while(0)

//TEST
#define SET_SIP(seip,stip,ssip)								\
	do { 													\
		asm volatile ("csrw sip, %0" ::						\
		"r"(((seip << 9) | (stip << 5) | (ssip << 1))));	\
		} while(0)

#define SET_SSTATUS(sum,sie)                                \
	do {													\
		uint32_t x = RD_SSTATUS();							\
		asm volatile ("csrw sstatus, %0" :: 				\
			"r"(x | (sie << 1) | (sum << 18)));				\
		} while(0) 
		
#define RD_SSTATUS() 										\
	({														\
	uint32_t x;												\
	asm volatile ("csrr %0,sstatus" :"=r"(x));				\
	x;														\
	})				

#define RD_SIE() 											\
	({														\
	 uint32_t x;											\
	 asm volatile ("csrr %0,sie" : "=r"(x));				\
	 x;														\
	 })

#define RD_TIME() 											\
	({            											\
	 uint32_t x;											\
	 asm volatile ("csrr %0,time" : "=r"(x));				\
	 x;														\
	 })

#define RD_SIP() 											\
	({														\
	uint32_t x;												\
	asm volatile ("csrr %0, sip" : "=r"(x));				\
	x;														\
	})

#define RD_SSCRATCH()										\
	({														\
	 uint32_t x;											\
	 asm volatile ("csrr %0, sscratch" : "=r"(x));			\
	 x;														\
	 })


#endif
