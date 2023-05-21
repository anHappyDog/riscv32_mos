#include <lib.h>

static char* file1 = "/newmotd";

int main() {
	char str[100];		
	char buf[1024];
	int r = open(file1,O_RDWR);
	if (r < 0) {
		user_panic("failed to open %s, return value: %d\n",file1,r);
	}
	int len = strlen(str);
	fprintf(r,str,len);
	close(r);
	r = open(file1,O_RDWR);
	if (r < 0) {
		user_panic("sad\n");
	}
	int l = read(r,buf,len);
	if (l != len) {
		user_panic("sda1\n");
	}
	buf[len] = 0;
	if (strcmp(str,buf) != 0) {
		user_panic("read failed!\n");
	}
	close(r);
	//debugf("getchar test:\n");
	/*char a[550];
	int x = 0,cnt = 0;
	for (int i = 0; i  < 100 ; ++i) {
		a[i] = 'a';
	}
	ecall_write_disk(0,0,a,1);
	for (int i = 0; i  < 100 ; ++i) {
		a[i] = 0;
	}
	ecall_read_disk(0,0,a,1);
	debugf(":::%s:::\n",a);*/
	//x = ecall_getchar();
	/*while((x = ecall_getchar()) != '\r') {
		if (x > 0) {
			a[cnt++] = x;
		}
	}*/
	/*a[cnt] = 0;
	debugf(":::%s\n",a);
	debugf("----------------\n");
	debugf("user_device test:\n");
	int *t =  ((int*)0x10008000);
	debugf("virtio magic is %08x\n",*t);
	debugf("virtio version is %08x\n",*(t + 1));
	debugf("virtio id is %08x\n",*(t + 2));
	*/
	return 0;
}
