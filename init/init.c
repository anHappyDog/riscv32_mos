#include <drivers/virtio_disk.h>
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
	
	disk_init();
	/*char buf[550];
	disk_rw(32,0,buf,1);
	printk(":::%s:::\n",buf);*/
	ENV_CREATE(user_test1);
	SBI_TIMER(200000 + RD_TIME());
	while(1);
	
	SBI_SHUTDOWN();
}

#endif
