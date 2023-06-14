#include <lib.h>

int xx = 0;

int main() {
	debugf("child fork starting!\n");
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
				int t4 = fork();
				if (t4 == 0) {
					int t5 = fork();
					if (t5 == 0) {
						int t6 = fork();
						debugf("ccccccccccccccc\n");
					}
				} else {
					debugf("aaaaaaaaaaaaaaaa\n");
				}	
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
