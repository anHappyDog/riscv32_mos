#include <sbi.h>
#include <env.h>
#include <mmu.h>
#include <pmap.h>
#include <printk.h>
#include <sched.h>
#include <syscall.h>

extern struct Env* curenv;

void sys_putchar(int c) {
	printcharc((char)c);
	return;
}

int sys_print_cons(const void* s,u_int num) {

	for (i = 0; i < num; ++i) {
		printcharc(((char*)s)[i]);
	}
	return 0;
}

u_int sys_getenvid(void) {
	return curenv->env_id;
}

