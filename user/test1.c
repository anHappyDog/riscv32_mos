#include <lib.h>

int main() {
	debugf("getchar test:\n");
	char a[100];
	int x = 0,cnt = 0;
	//x = ecall_getchar();
	while((x = ecall_getchar()) != '\r') {
		if (x > 0) {
			a[cnt++] = x;
		}
	}
	a[cnt] = 0;
	debugf(":::%s\n",a);
	debugf("----------------\n");
	debugf("user_device test:\n");
	int *t =  ((int*)0x10008000);
	debugf("virtio magic is %08x\n",*t);
	debugf("virtio version is %08x\n",*(t + 1));
	debugf("virtio id is %08x\n",*(t + 2));
	return 0;
}
