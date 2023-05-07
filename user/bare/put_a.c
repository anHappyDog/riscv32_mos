#include <printk.h>
#include <sbi.h>

void _start() {
	//asm volatile ("ecall");
	for (unsigned i = 0; ; ++i) {
		if (((i >>8) & 1) == 0) {
			SBI_PUTCHAR('c');
		}
	}
}
