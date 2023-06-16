#include <env.h>
#include <lib.h>
#include <mmu.h>

void exit(void) {
	close_all();
	ecall_env_destroy(0);
	user_panic("unreachable code");
}
volatile Pde* curenv_pgdir;
volatile struct Env* env;
extern int main(int, char**);

void libmain(int argc, char **argv) {
	env = &envs[ENVX(ecall_getenvid())];
	curenv_pgdir = env->env_pgdir;
	//debugf("env is %08x\n",env);
	//debugf("curenv_pgdir is %08x\n",curenv_pgdir);
	main(argc,argv);
	exit();
}

