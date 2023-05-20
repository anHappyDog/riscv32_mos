#include <lib.h>
#include <drivers/virtio_disk.h>

int main() {
	ecall_interrupt_off();	
	disk_init();
	ecall_interrupt_on();
	return 0;
}
