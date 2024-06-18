#include <lib.h>

const char file1[100] = "/motd";

int main() {
	int t = 0;
	int fk1 = fork();
	if (fk1 == 0) {
		int fk11 = fork();
		if (fk11 == 0) {
			t = 3;
			debugf("\t\tchild's child!,  t = %d\n",t);
		}
		else {
			t = 2;
			debugf("\t\t\tchild !, t= %d!\n",t);
		}
	
	} else {
		int fk111 = fork();
		if (fk111 == 0) {
			t = 4;
			debugf("\tfathers child ,t = %d\n",t);
		} else {
			t = 5;
			debugf("father, t = %d\n",t);
		}
	}
	return 0;
}
