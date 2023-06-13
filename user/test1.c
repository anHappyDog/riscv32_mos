#include <lib.h>

static char* file1 = "/motd";

int main() {
	char buf[1024];
	debugf("cc\n");
	int fd1 = open(file1,O_RDONLY);
	if (fd1 < 0) {
		user_panic("can't open the file!\n");
	}
	debugf("open arrived!\n");
	if (read(fd1,buf,100) < 0) {
		user_panic("can't read the file\n");
	}
	buf[101] = 0;
	debugf(";;;;%s;;;;\n",buf);
	close(fd1);	
	return 0;
}
