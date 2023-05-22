#include <env.h>
#include <fsreq.h>
#include <lib.h>

#define debug 0
u_char fsipcbuf[BY2PG] __attribute__((aligned(BY2PG)));


static int fsipc(u_int type, void* fsreq, void* dstva, u_int *perm) {
	u_int whom;
	ipc_send(envs[1].env_id,type,fsreq,PTE_D | PTE_R | PTE_W | PTE_U | PTE_LIBRARY);
	return ipc_recv(&whom,dstva,perm);
}

int fsipc_open(const char* path,u_int omode,struct Fd* fd) {
	u_int perm;
	struct Fsreq_open* req;
	req = (struct Fsreq_open*)fsipcbuf;
	if (strlen(path) >= MAXNAMELEN) {
		return -E_BAD_PATH;
	}
	strcpy((char*)req->req_path,path);
	req->req_omode = omode;
	return fsipc(FSREQ_OPEN,req,fd,&perm);
}

int fsipc_map(u_int fileid, u_int offset, void*dstva) {
	int r;
	u_int perm;
	struct Fsreq_map* req;
	req = (struct Fsreq_map*)fsipcbuf;
	req->req_fileid = fileid;
	req->req_offset = offset;
	if ((r = fsipc(FSREQ_MAP,req,dstva,&perm)) < 0) {
		return r;
	}
	if ((perm & (~PTE_D | PTE_LIBRARY)) & PTE_V != PTE_V) {
		user_panic("fsipc_map: unexpected permissions %08x for dstva %08x",perm,dstva);
	}
	return 0;
}

int fsipc_set_size(u_int fileid,u_int size) {
	struct Fsreq_set_size* req;
	req = (struct Fsreq_set_size*)fsipcbuf;
	req->req_fileid = fileid;
	req->req_size = size;
	return fsipc(FSREQ_SET_SIZE,req,0,0);
}

int fsipc_close(u_int fileid) {
	struct Fsreq_close *req;
	req = (struct Fsreq_close*)fsipcbuf;
	req->req_fileid = fileid;
	return fsipc(FSREQ_CLOSE,req,0,0);
}

int fsipc_dirty(u_int fileid, u_int offset) {
	struct Fsreq_dirty* req;
	req = (struct Fsreq_dirty*)fsipcbuf;
	req->req_fileid = fileid;
	req->req_offset = offset;
	return fsipc(FSREQ_DIRTY,req,0,0);
}

int fsipc_remove(const char* path) {
	if (strlen(path) == 0 || strlen(path) > MAXNAMELEN) {
	return -E_BAD_PATH;
	}
	struct Fsreq_remove* req = (struct Fsreq_remove*)fsipcbuf;
	strcpy(req->req_path,path);
	return fsipc(FSREQ_REMOVE,req,NULL,0);
}

int fsipc_sync(void) {
	return fsipc(FSREQ_SYNC,fsipcbuf,0,0);
}
