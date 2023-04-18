#ifndef __ASM_MIPS_REGDEF_H
#define __ASM_MIPS_REGDEF_H

/*
 * Symbolic register names for 32 bit ABI
 */
#define zero x0 /* wired zero */
#define ra x1	/* assembler temp  - uppercase because of ".set at" */
#define sp x2	/* return value */
#define gp x3
#define tp x4 /* argument registers */
#define t0 x5
#define t1 x6
#define t2 x7
#define fp x8 /* caller saved */
#define s0 x8
#define s1 x9
#define a0 x10
#define a1 x11
#define a2 x12
#define a3 x13
#define a4 x14
#define a5 x15
#define a6 x16 /* callee saved */
#define a7 x17
#define s2 x18
#define s3 x19
#define s4 x20
#define s5 x21
#define s6 x22
#define s7 x23
#define s8 x24 /* caller saved */
#define s9 x25
#define s10 x26 /* kernel scratch */
#define s11 x27
#define t3 x28 /* global pointer */
#define t4 x29 /* stack pointer */
#define t5 x30 /* frame pointer */
#define t6 x31 /* same like fp! */

#endif /* __ASM_MIPS_REGDEF_H */
