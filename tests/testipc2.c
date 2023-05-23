#include <lib.h>

int main() {
	debugf("%d\n",ipc_recv(0,0x20000000,0));
	char * t = (char*)0x20000000;
	debugf(";;;%s:::\n",t);

	return 0;
}
