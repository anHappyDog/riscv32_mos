#include <printk.h>
#include <sbi.h>

void _start() {
	//asm volatile ("ecall");
	SBI_PUTCHAR('c');
}
