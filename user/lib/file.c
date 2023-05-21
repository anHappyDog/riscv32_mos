#include <fs.h>
#include <lib.h>

#define debug 0

static int file_close(struct Fd* fd);
static int file_read(struct Fd* fd, void* buf,u_int n,u_int offset);
static int file_write(struct Fd* fd, const void* buf, u_int n, u_int offset);
static int file_stat(struct Fd* fd, struct Stat* stat);

struct Dev devfile = {
	.dev_id = 'f',
	.dev_name = "file",
	.dev_read = file_read,
	.dev_write = file_write,
	.dev_close = file_close,
	.dev_stat = file_stat,
};


int open(const char* path, int mode) {
	struct Fd* fd;
	try(fd_alloc(&fd));
	try(fsipc_open(path,mode,fd));
	char *va;
	struct Filefd *ffd;
	u_int size, fileid;
	va = fd2data(fd);
	ffd = (struct Filefd*)fd;
	size = ffd->f_file.f_size;
	fileid = ffd->f_file.f_size;
	for (int i = 0; i < size; i += BY2PG) {
		try(fsipc_map(fileid,i,va + i));
	}

	return fd2num(fd);
}

static int file_close(struct Fd* fd) {
	int r;
	struct Filefd* ffd;
	void* va;
	u_int size, fileid;
	u_int i;
	ffd = (struct Filefd*) fd;
	fileid = ffd->f_fileid;
	size = ffd->f_file.f_size;
	va = fd2data(fd);
	
	for (i = 0; i < size; i += BY2PG) {
		fsipc_dirty(fileid, i);
	}
	if ((r = fsipc_close(fileid)) < 0) {
		debugf("cannot close the file!\n");
		return r;
	}
	if (size == 0) {
		return 0;
	}
	for (i = 0; i < size; i += BY2PG) {
		if ((r = ecall_mem_unmap(0,(void*)(va + i))) < 0) {
			debugf("cannot unmap the file!\n");
			return r;
		}
	}
	return 0;
}

static int file_read(struct Fd* fd, void* buf, u_int n, u_int offset) {
	u_int size;
	struct Filefd* f;
	f = (struct Filefd*)fd;
	size = f->f_file.f_size;
	if (offset > size) {
		return 0;
	}
	if (offset + n > size) {
		n = size - offset;
	}
	memcpy(buf,(char*)fd2data(fd) + offset, n);
	return n;
}

int read_map(int fdnum, u_int offset, void** blk) {
	int r;
	void*va;
	struct Fd* fd;
	if ((r = fd_lookup(fdnum,&fd)) < 0) {
		return r;
	}
	if (fd->fd_dev_id != devfile.dev_id) {
		return -E_INVAL;
	}
	va = fd2data(fd) + offset;

	if (offset >= MAXFILESIZE) {
		return -E_NO_DISK;
	}
	Pte* pte;
	if (ecall_check_address(va,0,&pte) != 0) {
		return -E_NO_DISK;
	}
	*blk = (void*)va;
	return 0;
}

static int file_write(struct Fd* fd, const void* buf, u_int n,u_int offset) {
	int r;
	u_int tot;
	struct Filefd* f;
	f = (struct Filefd*)fd;
	tot = offset + n;
	if (tot > MAXFILESIZE) {
		return -E_NO_DISK;
	}
	if (tot > f->f_file.f_size) {
		if ((r = ftruncate(fd2num(fd),tot)) < 0) {
			return r;
		}
	}
	memcpy((char*)fd2data(fd) + offset,buf,n);
	return n;
}

static int file_stat(struct Fd* fd, struct Stat* st) {
	struct Filefd* f;
	f = (struct Filefd*)fd;
	strcpy(st->st_name,f->f_file.f_name);
	st->st_size = f->f_file.f_size;
	st->st_isdir = f->f_file.f_type == FTYPE_DIR;
	return 0;
}

int ftruncate(int fdnum, u_int size) {
	int i,r;
	struct Fd* fd;
	struct Filefd* f;
	u_int oldsize, fileid;
	if (size > MAXFILESIZE) {
		return -E_NO_DISK;
	}
	if ((r = fd_lookup(fdnum,&fd)) < 0) {
		return r;
	}
	if (fd->fd_dev_id != devfile.dev_id) {
		return -E_INVAL;
	}
	f = (struct Filefd*)fd;
	fileid = f->f_fileid;
	oldsize = f->f_file.f_size;
	f->f_file.f_size = size;
	if ((r = fsipc_set_size(fileid,size)) < 0) {
		return r;
	}
	void* va = fd2data(fd);
	for (i = ROUND(oldsize,BY2PG); i < ROUND(size,BY2PG);i += BY2PG) {
		if ((r = fsipc_map(fileid,i,va + i)) < 0) {
			fsipc_set_size(fileid,oldsize);
			return r;
		}
	}
	for (i = ROUND(size,BY2PG); i < ROUND(oldsize,BY2PG);i += BY2PG) {
		if ((r = ecall_mem_unmap(0,(void*)(va + i))) < 0) {
			user_panic("ftruncate: ecall_mem_unmap failed %08x: %e!\n",va + i,r);
		}
	}
	return 0;
}

int remove(const char* path) {
	return fsipc_remove(path);
}
int sync(void) {
	return fsipc_sync();
}


