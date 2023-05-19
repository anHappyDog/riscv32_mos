#include <lib.h>


int main() {
//	debugf("cnmd db!!!\n");
	/*int a = 0;
	int id = 0;
	if ((id = fork()) == 0) {
		debugf("this is child\n");
	}
	else {
		debugf("this is father\n");
		ecall_yield();
	}*/
	debugf("==----\n");
	int c;
	while ((c = ecall_getchar()) != '\r') {
		if (c == '\b') {
			ecall_putchar('\b');
			ecall_putchar(' ');
			ecall_putchar('\b');
		} else {
			ecall_putchar(c);
		}
	}
	ecall_putchar('\n');
/*	int a = 0,id;
	debugf("fktest on ostest!\n");
	if ((id = fork()) == 0) {
		if ((id = fork()) == 0) {
			a += 5;
			int iiiii;
			for (iiiii = 0; iiiii < 10; ++iiiii) {
				debugf("\t\t\t\t@this is child2 : a : %d\n",a);
				ecall_yield();
			}
			return 0;
		}
		a += 4;
		int iiii;
		for (iiii = 0; iiii < 10; ++iiii) {
			debugf("\t\t\t@this is child1 : a : %d\n",a);
			ecall_yield();	
		}
		return 0;
	}
	a += 3;
	int i;
	for (i = 0; i < 10; ++i) {
		debugf("\t\t@this is father : a : %d\n",a);
		ecall_yield();	
	}*/	
	return 0;
}
