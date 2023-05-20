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
	disk_init();
	env_init();
	
	ENV_CREATE(fs_t1);
/*	char buf[550];
	for (int i = 0; i < 512;++i) {
		buf[i] = 'c';
	}
	e_write_disk(0,0,buf,1);
	memset(buf,0,sizeof(buf));
	e_read_disk(0,0,buf,1);
	printk(":::\n%s\n:::\n",buf);*/

	SBI_TIMER(200000 + RD_TIME());
	while(1);
	SBI_SHUTDOWN();
}

#endif
