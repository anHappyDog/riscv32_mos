#include <sbi.h>
void printcharc(char ch) {
	SBI_PUTCHAR(ch);
}

char scancharc(void) {
	return 0;
}

void halt(void) {
	SBI_SHUTDOWN();
}
