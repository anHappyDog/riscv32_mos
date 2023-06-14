#include <lib.h>

int main(int argc, char**argv) {
	for (int i = 0; i < argc; ++i) {
		debugf("''''''''''''%s;;;;;;;;;\n",argv[i]);
	}
	return 0;
}
