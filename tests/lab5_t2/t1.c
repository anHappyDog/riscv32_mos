#include <lib.h>

const char file1[100] = "/newmotd";

int main() {
	char buf[512];
	int fd = open(file1,O_RDONLY);
	read(fd,buf,512);
	close(fd);
	debugf(buf);
	return 0;
}
