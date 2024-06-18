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
/*	
	char buf[550];
	disk_rw(32,0,buf,1);
	printk(":::%s:::\n",buf);
	disk_rw(0,1,buf,1);
*/
//	ENV_CREATE(user_ppa);
//	ENV_CREATE(user_ppa);

//	ENV_CREATE(user_testipc1);
//	ENV_CREATE(user_testipc2);
	ENV_CREATE(user_icode);
//	ENV_CREATE(user_test1);
//	ENV_CREATE(user_test2);
	ENV_CREATE(fs_serv);
//	ENV_CREATE(user_test1);
//	ENV_CREATE(user_test1);
	SBI_TIMER(200000 + RD_TIME());
	while(1);
	
	SBI_SHUTDOWN();
}

#endif
