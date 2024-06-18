#include <print.h>
#include <printk.h>
#include <sbi.h>
void outputk(void *data, const char *buf, size_t len);

void _panic(const char *file, int line, const char *func, const char *fmt, ...) {
	//u_long sp, ra, badva, sr, cause, epc;
	u_long sp,ra,stval,sstatus,scause,sepc;
	asm("mv %0, sp" : "=r"(sp) :);
	asm("mv %0, ra" : "=r"(ra) :);
	asm("csrr %0, stval" : "=r"(stval) :);
	asm("csrr %0, sstatus" : "=r"(sstatus) :);
	asm("csrr %0, scause" : "=r"(scause) :);
	asm("csrr %0, sepc" : "=r"(sepc) :);

	printk("panic at %s:%d (%s): ", file, line, func);

	va_list ap;
	va_start(ap, fmt);
	vprintfmt(outputk, NULL, fmt, ap);
	va_end(ap);

	printk("\n"
	       "ra:    %08x  sp:  %08x  sstatus: %08x\n"
	       "scause: %08x  sepc: %08x  stval:  %08x\n",
	       ra, sp, sstatus, scause,sepc, stval);
	SBI_SHUTDOWN();
}
