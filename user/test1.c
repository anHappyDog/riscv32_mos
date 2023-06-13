#include <lib.h>

int doOepn(int f, char*buf) {
	int t;
	while ((t = readn(f,buf,512)) != 0) {
		debugf("%s",buf);
	} 
	close(f);
	return 0;	
}

int main() {
	int r;
	struct Stat st;
	if ((r = stat("/motd",&st)) < 0) {
		user_panic("ffff\n");
	}
	if (st.st_isdir) {
	
	} else {
		debugf("ccccccccccc\n");
	}
	/*debugf("cnmmm!\n");
	char buf[512];
	int fd = open("/tt",O_RDONLY);
	doOepn(fd,buf);*/
	return 0;
}
