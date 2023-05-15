#include<env.h>
#include<lib.h>
#include<mmu.h>
#include<ecall.h>
#include<trap.h>


int ecall_getchar(void) {
	return mecall(ECALL_getchar);
}


void ecall_putchar(int ch) {
	mecall(ECALL_putchar,ch);
}

int ecall_get_pgdir(Pde** pde) {
	return mecall(ECALL_get_pgdir,pde);
}

int ecall_print_cons(const void*str, u_int num) {
	return mecall(ECALL_print_cons,str,num);
}

u_int ecall_getenvid(void) {
	return mecall(ECALL_getenvid);
}

void ecall_yield(void) {
	mecall(ECALL_yield);
}

int ecall_env_destroy(u_int envid) {
	return mecall(ECALL_env_destroy,envid);
}

int ecall_mem_alloc(u_int envid, void*va, u_int perm) {
	return mecall(ECALL_mem_alloc,envid,va,perm);
}

int ecall_mem_map(u_int srcid, void* srcva, u_int dstid, void* dstva, u_int perm) {
	return mecall(ECALL_mem_map,srcid,srcva,dstid,dstva,perm);
}

int ecall_mem_unmap(u_int envid, void* va) {
	return mecall(ECALL_mem_unmap, envid, va);
}

int ecall_set_env_status(u_int envid, u_int status) {
	return mecall(ECALL_set_env_status,envid,status);
}

int ecall_set_trapframe(u_int envid, struct Trapframe*tf) {
	return mecall(ECALL_set_trapframe,envid,tf);
}

void ecall_panic(const char* msg) {
	int r = mecall(ECALL_panic,msg);
	user_panic("ECALL_panic returned %d",r);
}

int ecall_ipc_try_send(u_int envid, u_int value, const void* srcva, u_int perm) {
	return mecall(ECALL_ipc_try_send,envid,value,srcva,perm);
}

int ecall_ipc_recv(void* dstva) {
	return mecall(ECALL_ipc_recv,dstva);
}

int ecall_cgetc() {
	return mecall(ECALL_cgetc);
}	

int ecall_set_env_cow_entry(u_int envid, u_int cow_entry) {
	return mecall(ECALL_set_env_cow_entry,envid,cow_entry);
}


int ecall_write_dev(void* va, u_int dev, u_int len) {
	return 0;
	//
}

int ecall_read_dev(void* va, u_int dev, u_int len) {
	return 0;
	//
}









