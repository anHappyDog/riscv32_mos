#include <print.h>
#include <printk.h>
#include <trap.h>
#include <sbi.h>

void outputk(void *data, const char *buf, size_t len) {
	for (int i = 0; i < len; i++) {
		SBI_PUTCHAR(buf[i]);
	}
}

void printk(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vprintfmt(outputk, NULL, fmt, ap);
	va_end(ap);
}

void print_tf(struct Trapframe *tf) {
	for (int i = 0; i < sizeof(tf->regs) / sizeof(tf->regs[0]); i++) {
		printk("$%2d = %08x\n", i, tf->regs[i]);
	}
	printk("sstatus   = %08x\n",tf->sstatus);
	printk("sepc 	  = %08x\n",tf->sepc);
	printk("stval 	  = %08x\n",tf->stval);
	printk("scause    = %08x\n",tf->scause);

}
