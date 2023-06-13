#include <lib.h>

int xx = 0;

int main() {
	int t = fork();
	if (t == 0) {
		xx = 1;
		debugf("\tchild 1 ,%d\n",xx);
		int t1 = fork();
		if (t1 == 0) {
			xx= 100;
			debugf("\t\tchild 2 ,%d\n",xx);
			int t3 = fork();
			if (t3 == 0) {
				xx = -222222;
				debugf("\t\t\tchild 3 , %d\n",xx);
			} else {
				xx = -100;
				debugf("\t\tchild 2 finished, %d\n",xx);
			
			}
		} else {
			xx = 1000;
			debugf("\t child 1 finished!, %d\n",xx);
		}
	} else {
		xx = -1;
		debugf("father!, %d\n",xx);
	}
	return 0;
}
