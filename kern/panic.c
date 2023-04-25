#include <print.h>
#include <printk.h>

void outputk(void *data, const char *buf, size_t len);

void _panic(const char *file, int line, const char *func, const char *fmt, ...) {
	//u_long sp, ra, badva, sr, cause, epc;
	u_long sp,ra,mtval,mstatus,mcause,mepc;
	asm("mv %0, sp" : "=r"(sp) :);
	asm("mv %0, ra" : "=r"(ra) :);
	asm("csrr %0, mtval" : "=r"(mtval) :);
	asm("csrr %0, mstatus" : "=r"(mstatus) :);
	asm("csrr %0, mcause" : "=r"(mcause) :);
	asm("csrr %0, mepc" : "=r"(mepc) :);

	printk("panic at %s:%d (%s): ", file, line, func);

	va_list ap;
	va_start(ap, fmt);
	vprintfmt(outputk, NULL, fmt, ap);
	va_end(ap);

	printk("\n"
	       "ra:    %08x  sp:  %08x  mstatus: %08x\n"
	       "mcause: %08x  mepc: %08x  mtval:  %08x\n",
	       ra, sp, mstatus, mcause,mepc, mtval);
}
