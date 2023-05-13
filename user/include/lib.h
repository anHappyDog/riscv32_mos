#ifndef LIB_H
#define LIB_H

#include <args.h>
#include <env.h>
#include <fd.h>
#include <mmu.h>
#include <pmap.h>
#include <ecall.h>
#include <trap.h>


#define envs ((volatile struct Env*)UENVS)
#define pages ((volatile struct Page*)UPAGES)

void exit(void) __attribute__((noreturn));

extern volatile struct Env* env;

#define USED(x) (void)(x)

void debugf(const char* fmt, ... );
void _user_panic(const char*,int, const char*, ...) __attribute__ ((noreturn));
void _user_halt(const char*,int, const char*, ...) __attribute__ ((noreturn));

#define user_panic(...) _user_panic(__FILE__,__LINE__,__VA_ARGS__)
#define user_halt(...) _user_halt(__FILE__,__LINE__,__VA_ARGS__)

#undef panic_on
#define panic_on(expr)								\
	do {											\
		int r = (expr);								\
		if (r != 0) {								\
			user_panic("'" #expr "' returned %d",r);\
		}											\
    } while (0)


int spawn(char* prog, char** argv);
int spawn1(char* prot, char* args, ...);
int fork();


extern int mecall(int,...);

void ecall_putchar(int ch);
int ecall_print_cons(const void* str, u_int num);
u_int ecall_getenvid(void);
void ecall_yield(void);
int ecall_env_destroy(u_int envid);
int ecall_mem_alloc(u_int envid, void* va, u_int perm);
int ecall_mem_map(u_int srcid, void* srcva,u_int dstid, void* dstva, u_int perm);
int ecall_mem_unmap(u_int envid, void* va);

__attribute__((always_inline)) inline static int ecall_exofork(void) {
	return mecall(ECALL_exofork,0,0,0,0,0);
}
int ecall_set_env_cow_entry(u_int envid,u_int cow_entry);
int ecall_set_env_status(u_int envid, u_int status);
int ecall_set_trapframe(u_int envid, struct Trapframe* tf);
void ecall_panic(const char* msg) __attribute__((noreturn));
int ecall_ipc_try_send(u_int envid, u_int value, const void* srcva, u_int perm);
int ecall_ipc_recv(void* dstva);
int ecall_cgetc();
int ecall_write_dev(void*,u_int,u_int);
int ecall_read_dev(void*,u_int,u_int);

void ipc_send(u_int whom,u_int val, const void* srcva, u_int perm);
u_int ipc_recv(u_int* whom, void* dstva, u_int* perm);

void wait(u_int envid);

// not finished!!!!
//
//
//

#define user_assert(x)								\
	do {											\
		if (!(x))									\
			user_panic("assert failed: %s",#x);		\
	} while(0)

#define O_RDONLY 	0x0000
#define O_WRONLY 	0x0001
#define O_RDWR   	0x0002
#define O_ACCMODE 	0x0003

#define O_CREAT     0x0100
#define O_TRUNC 	0x0200
#define O_EXCL		0x0400
#define O_MKDIR		0x0800

#endif































