#include <sbi.h>
void printcharc(char ch) {
	SBI_PUTCHAR(ch);
}

int scancharc(void) {
	int x = 0;
	while((x = SBI_GETCHAR()) == -1);
	return x;
}

void halt(void) {
	SBI_SHUTDOWN();
}
