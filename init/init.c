#include <asm/asm.h>
#include <env.h>
#include <kclock.h>
#include <pmap.h>
#include <printk.h>
#include <trap.h>

// When build with 'make test lab=?_?', we will replace your 'mips_init' with a generated one from
// 'tests/lab?_?'.
#ifdef MOS_INIT_OVERRIDDEN
#include <generated/init_override.h>
#else

void mips_init() {
	printk("init.c:\tmips_init() is called\n");
//	printk("%d   %08x\n",1,21231);
//	char s1[100] = "asdsaddsadadad";
//	printk("%s\n",s1);

	// lab2:
	mips_detect_memory();
	mips_vm_init();
	page_init();
	//physical_memory_manage_check();
	pgdir_init();
	/*
	extern struct Page* pages;
	extern u_long npage;

	for (int i = 0; i < 100; ++i) {
		printk("the page address is 0x%08x, the vaddr is 0x%08x \n",&pages[i],page2addr(&pages[i]));
	}
*/
	/*
	struct Page* pp,*pp1;
	page_alloc(&pp);
	assert(pp != NULL);
	printk("--------------------------------------\n");
	printk("the alloced page address is 0x%08x, the vaddr is 0x%08x \n",pp,page2addr(pp));
	unsigned long * t1 = (unsigned long*)page2addr(pp);
	*t1 = 100;
	*(t1 + 1) = 200;
	printk("-----------\n%d : %d\n---------\n",*t1,*(t1 + 1));
	printk("-------------\n0x%08x : 0x%08x\n",t1,(t1 + 1));
	page_alloc(&pp1);	
	assert(pp1 != NULL);
	printk("--------------------\n");
	printk("the alloced page address is 0x%08x, the vaddr is 0x%08x \n",pp1,page2addr(pp1));	
	*/
	// lab3:
	// env_init();
	
	// lab3:
	// ENV_CREATE_PRIORITY(user_bare_loop, 1);
	// ENV_CREATE_PRIORITY(user_bare_loop, 2);

	// lab4:
	// ENV_CREATE(user_tltest);
	// ENV_CREATE(user_fktest);
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
	while(1) {

	}
	//	halt();
}

#endif
