#include <asm/asm.h>
#include <asm/embdasm.h>
#include <env.h>
#include <kclock.h>
#include <pmap.h>
#include <printk.h>
#include <trap.h>
#include <sbi.h>


// When build with 'make test lab=?_?', we will replace your 'mips_init' with a generated one from
// 'tests/lab?_?'.
#ifdef MOS_INIT_OVERRIDDEN
#include <generated/init_override.h>
#else

void riscv32_init() {
	printk("init.c:\triscv32_init() is called\n");
//	printk("%d   %08x\n",1,21231);
//	char s1[100] = "asdsaddsadadad";
//	printk("%s\n",s1);
	// lab2:
	

	
	riscv32_detect_memory();
	page_init();
	pgdir_init();
	//SET_STVEC(0x80200000,0);
	//SET_SSTATUS(1);
	//SET_SIE(0,1,0);
	//SBI_TIMER(1000 + RD_TIME());
	env_init();
	//env_check();
	//asm("ebreak" :: );	
	//printk("ebreak ok !\n");
	//env_check();	
//	ENV_CREATE_PRIORITY(user_bare_test1,3);
//	ENV_CREATE_PRIORITY(user_bare_loop, 1);
//	ENV_CREATE_PRIORITY(user_bare_loop, 2);
//	ENV_CREATE_PRIORITY(user_bare_loop, 3);
//	ENV_CREATE_PRIORITY(user_bare_loop, 4);
//	ENV_CREATE_PRIORITY(user_bare_put_a,2);
	// lab4:
	// ENV_CREATE(user_tltest);
	//ENV_CREATE(user_bare_loop);
//	ENV_CREATE_PRIORITY(user_fktest,10);
//	ENV_CREATE_PRIORITY(user_fktest,10);
//	ENV_CREATE_PRIORITY(user_fktest,10);
	ENV_CREATE(user_ppa);
	ENV_CREATE(user_ppa);

	// ENV_CREATE(user_pingpong);

	// lab6:
	// ENV_CREATE(user_icode);  // This must be the first env!

	// lab5:
	// ENV_CREATE(user_fstest);
	// ENV_CREATE(fs_serv);  // This must be the second env!
	// ENV_CREATE(user_devtst);

	// lab3:
	// kclock_init();
	// enable_irq();
	//	halt();
	SBI_TIMER(200000 + RD_TIME());
	while(1);
	SBI_SHUTDOWN();
}

#endif
