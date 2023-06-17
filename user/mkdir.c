#include <lib.h>

void mkdir(char* path) {
	int fd,r;
	if ((fd = open(path,O_MKDIR)) < 0) {
		printf("mkdir failed!, err is %3d\n",fd);
		exit();
	}
	if ((r = close(fd)) < 0) {
		printf("mkdir failed!, err is %3d\n",r);
		exit();
	}
}

void usage() {
	printf("wrong format!\nusage: mkdir [dir1] [dir2] [dir3] ...\n");
}

int main(int argc, char**argv) {
	if (argc < 2) {
		usage();
	} else {
		for (int i = 1; i < argc; ++i) {
			mkdir(argv[i]);
		}
	}
	return 0;
}
