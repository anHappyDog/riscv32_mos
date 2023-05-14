#include <env.h>
#include <pmap.h>
#include <printk.h>
#include <trap.h>
#include <asm/embdasm.h>
#include <sbi.h>
#include <types.h>
#include <ecall.h>

extern void handle_int(void);
extern void handle_tlb(void);
extern void handle_sys(void);
extern void handle_mod(void);
extern void handle_reserved(void);
extern void handle_software(void);
extern void handle_timer(void);
extern void handle_instruction_page(void);
extern void handle_store_page(void);
extern void handle_ecall_from_u(void);
extern void handle_load_page(void);


extern void schedule(int yield);
extern void* ecall_table[MAX_ENO];


void (*exception_handlers[32])(void) = {
	[0 ... 31] = handle_reserved,
	[17] = handle_software,
	[21] = handle_timer,
	[3] = handle_software,
	[8] = handle_ecall_from_u,
	[12] = handle_instruction_page,
	[13] = handle_load_page,
	[15] = handle_store_page,
	/*	[0] = handle_int,
	[1] = handle_tlb,
	[2 ... 3] = handle_tlb,
	[8] = handle_sys,*/
};

void do_load_page(struct Trapframe* tf) {
	struct Page* pp = addr2page(tf->stval);
	if (pp == NULL && page_alloc(&pp) != 0) {
		panic("load page : alloc failed!\n");
	}
	/*if (page_alloc(&pp) != 0) {
		panic("alloc page failed!\n");
	}*/
	if (page_insert(curenv->env_pgdir,curenv->env_asid,pp,ROUNDDOWN(tf->stval,BY2PG),PTE_R | PTE_W | PTE_U) != 0) {
		panic("insert page failed!\n");
	}
//	printk("load page process well!\n");
}


void do_software_int(struct Trapframe* tf) {
	printk("software_int is  ok!\n");
	printk("ExcCode is %08x\n",tf->scause);

}

void do_ecall_from_u(struct Trapframe* tf) {
	int (*func)(u_int, u_int, u_int, u_int, u_int);
	int eno = tf->regs[10];
    if (eno < 0 || eno >= MAX_ENO) {
   		tf->regs[1] = -E_NO_E;
    	return;
	}	  
	tf->sepc += 4;
	func = ecall_table[eno];
	u_int arg1 = tf->regs[11];
	u_int arg2 = tf->regs[12];
	u_int arg3 = tf->regs[13];
	u_int arg4 = tf->regs[14];
	u_int arg5 = tf->regs[15];
	tf->regs[10] = func(arg1,arg2,arg3,arg4,arg5);
}


void do_store_page(struct Trapframe* tf) {
	Pte* pte1;
	page_lookup(cur_pgdir,tf->stval,&pte1);
	if ((*pte1 & PTE_COW) == PTE_COW) {
		*((struct Trapframe*)UXSTACKTOP - 1) = *tf;

		if (tf->regs[2] < USTACKTOP || tf->regs[2] >= UXSTACKTOP) {
			tf->regs[2] = UXSTACKTOP;
		}
		tf->regs[2] -= 2 * sizeof(struct Trapframe);
	    *(struct Trapframe*)tf->regs[2] = *tf;
	
		if(curenv->env_cow_entry) {
			tf->regs[10] = tf->regs[2];
			//tf->regs[2] -= sizeof(tf->regs[10]);	
			tf->sepc = curenv->env_cow_entry;	
		//	env_pop_tf(tf);
		}
		else {
			panic("COW doesn't have cow_entry!\n");
		}
	}
	else {
		print_tf(tf);
		panic("do_store_page doesn't have pte_cow\n");
	}
	/*
	if (page_alloc(&pp) != 0) {
		panic("alloc page failed!\n");
	}
	if (page_insert(curenv->env_pgdir,curenv->env_asid,pp,ROUNDDOWN(tf->stval,BY2PG),PTE_R | PTE_W | PTE_U) != 0) {
		panic("insert page failed!\n");
	}
	*/
//	printk("store page fault process well!\n");
}

void do_instruction_page(struct Trapframe* tf) {
	print_tf(tf);
	panic("Instruction Page Fault!\n");


}


void do_timer_int(struct Trapframe* tf) {
	//static int kick = 0;
	//printk("ok!\n");	
	//printk("ExcCode is %08x\n",tf->scause);
	/*printk("go to schedule ....\n");
	++kick;
	if (kick == 20) {
		panic("tick finish, tick is %d!\n",kick);
		
	}*/
//	printk("---%d\n",RD_TIME());	
	SBI_TIMER(200000 + RD_TIME());
	//printk("%d : ",kick);
	schedule(0);

}

void do_reserved(struct Trapframe* tf) {
	print_tf(tf);
	panic("Unknown ExcCode %08x",tf->scause);
	// scause is not yet determined !!!!
	//panic("Unknown ExcCode %2d",(tf->scause >> 2) &0x1f);
	//printk("sip is %d\n",((RD_SIP() >> 5) & 1));
}
