#include <lib.h>

const char file1[100] = "/motd";

int main() {
	char buf[512];
	char buf1[512];
	debugf("hello, file system!\n");
	int fd = open(file1,O_RDONLY);
	read(fd,buf,512);
	close(fd);
	debugf("buf is ;\n%s;\n",buf);
	fd = open(file1,O_RDWR);
	for (int i = 0; i < 512; ++i) {
		buf1[i] = 0;
	}
	write(fd,buf1,512);
	close(fd);
	fd = open(file1,O_RDONLY);
	read(fd,buf,512);
	close(fd);
	debugf("buf is ;\n%s;\n",buf);
	return 0;
}
