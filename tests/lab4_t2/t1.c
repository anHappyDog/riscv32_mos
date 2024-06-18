#include <lib.h>

char buf[100];
int main() {
	int c;
	debugf("start!\n");
	for (int i = 0;; ++i) {
		while((c = ecall_cgetc()) == 0) {
			ecall_yield();
		}
		if (c == '\r') {
			buf[i] = 0;
			break;
		}	else {
			buf[i] = c;
		}
	}
	debugf(";;;;;\n%s\n;;;;\n",buf);
	return 0;
}
