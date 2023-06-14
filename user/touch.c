#include <lib.h>


void touch(char* path) {
	int fd = open(path,O_CREAT | O_RDWR),r;
	if (fd < 0) {
		user_panic("touch failed! err: %3d\n",fd);
	}
	if ((r = close(fd)) < 0) {
		user_panic("touch failed! err: %3d\n",fd);
	}

}

void usage() {
	printf("format wrong!\n usage: touch [file1] [file2] ...\n");
}

int main(int argc, char**argv) {
	if (argc < 2) {
		usage();
	} else {
		for (int i = 1; i < argc; ++i) {
			touch(argv[i]);
		}
	}
	return 0;
}

