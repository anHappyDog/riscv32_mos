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

void serve_init(void) {
	int i;
	u_int va;
	va = FILEVA;
	for (i = 0; i < MAXOPEN; ++i) {
		opentab[i].o_fileid = i;
		opentab[i].o_ff = (struct Filefd*)va;
		va += BY2PG;
	}
}



int open_alloc(u_int envid,struct Open** o) {
	int i,r;
	for (i = 0; i < MAXOPEN; ++i) {
		switch(pageref(opentab[i].o_ff)) {
			case 0:
				if ((r = ecall_mem_alloc(0,opentab[i].o_ff,PTE_G | PTE_R | PTE_W | PTE_U | PTE_D | PTE_LIBRARY)) < 0) {
					return r;
				}
			default:
				if ((r = ecall_mem_map(0,opentab[i].o_ff,envid,opentab[i].o_ff,PTE_G | PTE_R | PTE_W | PTE_U | PTE_D | PTE_LIBRARY) < 0)) {
					return r;		
				}
				opentab[i].o_fileid += MAXOPEN;
				*o = &opentab[i];
				memset((void*)opentab[i].o_ff,0,BY2PG);
				return (*o)->o_fileid;
		}
	}
	return -E_MAX_OPEN;
}

int open_lookup(u_int envid, u_int fileid, struct Open** po) {
	struct Open *o;
	o = &opentab[fileid % MAXOPEN];
	if (pageref(o->o_ff) == 1 || o->o_fileid != fileid) {
		return -E_INVAL;
	}
	*po = o;
	return 0;

}

void serve_open(u_int envid, struct Fsreq_open* rq) {
	struct File* f;
	struct Filefd* ff;
	int r;
	struct Open *o;
	if ((r = open_alloc(envid,&o)) < 0) {
		ipc_send(envid,r,0,0);
	}

	if ((r = file_open(rq->req_path,&f)) < 0) {
		ipc_send(envid,r,0,0);
		return;
	}
	o->o_file = f;
	ff = (struct Filefd*)o->o_ff;
	ff->f_file = *f;
	ff->f_fileid = o->o_fileid;
	o->o_mode = rq->req_omode;
	ff->f_fd.fd_omode = o->o_mode;
	ff->f_fd.fd_dev_id = devfile.dev_id;
	ipc_send(envid,0,o->o_ff,PTE_R | PTE_W | PTE_U | PTE_G | PTE_D | PTE_LIBRARY);
}

void serve_map(u_int envid, struct Fsreq_map* rq) {
	struct Open* popen;
	u_int filebno;
	void* blk;
	int r;
	if ((r = open_lookup(envid,rq->req_fileid,&popen)) < 0) {
		ipc_send(envid,r,0,0);
		return;
	}
	filebno = rq->req_offset / BY2BLK;
	if ((r = file_get_block(popen->o_file,filebno,&blk)) < 0) {
		ipc_send(envid,r,0,0);
		return;
	}
	ipc_send(envid,0,blk,PTE_R | PTE_W | PTE_U | PTE_G | PTE_D | PTE_LIBRARY);
}

void serve_set_size(u_int envid, struct Fsreq_set_size* rq) {
	struct Open* popen;
	int r;
	if ((r = open_lookup(envid,rq->req_fileid,&popen)) < 0) {
		ipc_send(envid,r,0,0);
		return;
	}
	if ((r = file_set_size(popen->o_file,rq->req_size)) < 0) {
		ipc_send(envid,r,0,0);
		return;
	}
	ipc_send(envid,0,0,0);

}


void serve_close(u_int envid,struct Fsreq_close* rq) {
	struct Open *popen;
	int r;
	if ((r = open_lookup(envid,rq->req_fileid,&popen)) < 0) {
		ipc_send(envid,r,0,0);
		return;
	}
	file_close(popen->o_file);
	ipc_send(envid,0,0,0);
}

void serve_remove(u_int envid, struct Fsreq_remove* rq) {
	int r;
	r = file_remove(rq->req_path);
	ipc_send(envid,r,0,0);

}

void serve_dirty(u_int envid, struct Fsreq_dirty* rq) {
	struct Open* popen;
	int r;
	if ((r = open_lookup(envid,rq->req_fileid,&popen)) < 0) {
		ipc_send(envid,r,0,0);
		return;
	}
	if ((r = file_dirty(popen->o_file,rq->req_offset)) < 0) {
		ipc_send(envid,r,0,0);
		return;
	}
	ipc_send(envid,0,0,0);

}

void serve_sync(u_int envid) {
	fs_sync();
	ipc_send(envid,0,0,0);
}

void serve(void) {
	u_int req,whom,perm;
	for (;;) {
		perm = 0;
		req = ipc_recv(&whom,(void*)REQVA,&perm);
		if (!(perm & PTE_V)) {
			debugf("Invalid request from %08x: no argument page\n",whom);
			continue;
		}
		switch (req) {
			case FSREQ_OPEN:
				serve_open(whom,(struct Fsreq_open*)REQVA);
				break;
			case FSREQ_MAP:
				serve_map(whom,(struct Fsreq_map*)REQVA);
				break;
			case FSREQ_SET_SIZE:
				serve_set_size(whom,(struct Fsreq_set_size*)REQVA);
				break;
			case FSREQ_CLOSE:
				serve_close(whom,(struct Fsreq_close*)REQVA);
				break;
			case FSREQ_DIRTY:
				serve_dirty(whom,(struct Fsreq_dirty*)REQVA);
				break;
			case FSREQ_REMOVE:
				serve_remove(whom,(struct Fsreq_remove*)REQVA);
				break;
			case FSREQ_SYNC:
				serve_sync(whom);
				break;
			default:
				debugf("Invalid request code %d from %08x\n",whom,req);
				break;	
		}
	}
	ecall_mem_unmap(0,(void*)REQVA);

}




int main() {
	user_assert(sizeof(struct File) == BY2FILE);
	serve_init();
	fs_init();
	serve();
	return 0;
}
