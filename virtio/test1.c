#include <lib.h>
#include <drivers/virtio_disk.h>

int main() {
	char buf[550];
	disk_rw(32,0,buf,1);
	debugf(";::\n%s\n:::\n",buf);
	return 0;
}
