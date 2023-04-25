void mips_init() {
	printk("init.c:\tmips_init() is called\n");
	mips_detect_memory();
	mips_vm_init();
	page_init();

	physical_memory_manage_check();
	physical_memory_manage_strong_check();
	halt();
}
