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
	char buf[550];
	for (int i = 0; i < 100; ++i) {
		buf[i] = 'c';
	}
	//	buf[0] = 'c';
	buf[100] = 0;
	disk_rw(1,1,buf);
	//disk_rw(1,1,buf);
	//disk_rw(1,1,buf);
	//disk_rw(1,1,buf);
	memset(buf,0,550);
	//printk("before::%s\n",buf);
	disk_rw(1,0,buf);
	printk(":::\n%s\n:::\n",buf);
//	SBI_TIMER(200000 + RD_TIME());
	//while(1);
	
	SBI_SHUTDOWN();
}

#endif
