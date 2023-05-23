#include <lib.h>

int main() {

	ecall_mem_alloc(0,0x20000000,PTE_R | PTE_W | PTE_U | PTE_V);
	char* t = (char*)0x20000000;
	for (int i = 0; i < 26; ++i) {
		t[i] = 'a' + i;
	}
	debugf(";;;%s;;;\n",t);
	ipc_send(envs[1].env_id,0,0x20000000,PTE_R | PTE_W | PTE_U | PTE_V);
	
	return 0;
}
