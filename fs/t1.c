#include <lib.h>

int main() {
	char buf[550];
	for (int i = 0; i < 512; ++i) {
		buf[i] = 'c';
	}
	ecall_write_disk(0,buf,1);
	for (int i = 0; i < 512; ++i) {
		buf[i] = 0;
	}
	ecall_read_disk(0,buf,1);
	debugf(":::\n%s\n:::\n",buf);
	return 0;
}
