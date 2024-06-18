void riscv32_init() {
	printk("init.c:\triscv32_init() is called!\n");
	riscv32_detect_memory();
	page_init();
	pgdir_init();
	disk_init();
	env_init();
	env_check();
	halt();
}
