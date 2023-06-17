#include <lib.h>

int main(int argc,char**argv) {
	char c;
	while ((c = ecall_cgetc()) != '\n') {
		debugf("%d\n",c);
	}
	return 0;
}
