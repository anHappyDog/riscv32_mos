#include <env.h>
#include <lib.h>
#include <mmu.h>

void exit(void) {
	ecall_env_destroy(0);
	user_panic("unreachable code");
}

volatile struct Env* env;
extern int main(int, char**);

void libmain(int argc, char **argv) {
	env = &envs[ENVX(ecall_getenvid())];
	main(argc,argv);
	exit();
}

