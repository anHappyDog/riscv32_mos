#include <lib.h>

const char file1[100] = "/newmotd";

int main() {
	int fk = fork();
	if (fk == 0) {
	char buf[512];
		int fd = open("/motd",O_RDONLY);
		if (fd < 0) {
			debugf("child, fd : %d\n",fd);
		}
		read(fd,buf,512);
		close(fd);
		debugf("child:\n;\n%s\n;\n",buf);
	} else {
	char buf1[512];
		int fd = open("/motd",O_RDONLY);
		if (fd < 0) {
			debugf("father,fd : %d\n",fd);
		}	
		read(fd,buf1,512);
		close(fd);
		debugf("father:\n;\n%s\n;\n",buf1);
	}
	
	return 0;
}
