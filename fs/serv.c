#include "serv.h"
#include <fd.h>
#include <fsreq.h>
#include <lib.h>
#include <mmu.h>


struct Open {
	struct File* o_file;
	u_int o_fileid;
	int o_mode;
	struct Filefd *o_ff;
};

#define MAXOPEN 1024
#define FILEVA 0x60000000

struct Open opentab[MAXOPEN] = {{0,0,1}};

#define REQVA 0x0ffff000

void serv_init(void) {
	int i;
	u_int va;
	va = FILEVA;
	for (i = 0; i < MAXOPEN; ++i) {
		opentab[i].o_fileid = i;
		opentab[i].o_ff = (struct Filefd*)va;
		va += BY2PG;
	}
}

int open_alloc(struct Open**o) {
	int i;
	for (i = 0; i < MAXOPEN; ++i) {
		//switch ()
	}
	return 0;
}

int main() {


	return 0;
}
