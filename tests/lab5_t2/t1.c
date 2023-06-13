#include <lib.h>

const char file1[100] = "/motd";

int main() {
	int r;
	struct Stat st;
	r = stat(file1,&st);

	return 0;
}
