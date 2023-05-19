#include <drivers/virtio_blk.h>
#include <asm/asm.h>
#include <asm/embdasm.h>
#include <env.h>
#include <kclock.h>
#include <pmap.h>
#include <printk.h>
#include <trap.h>
#include <sbi.h>
#include <drivers/virtio.h>

#ifdef MOS_INIT_OVERRIDDEN
#include <generated/init_override.h>
#else

void riscv32_init() {
	printk("init.c:\triscv32_init() is called\n");
	riscv32_detect_memory();
	page_init();
	pgdir_init();
	env_init();

//	SBI_TIMER(200000 + RD_TIME());
	while(1);
	SBI_SHUTDOWN();
}

#endif
