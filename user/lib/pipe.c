#include <env.h>
#include <lib.h>
#include <mmu.h>

static int pipe_close(struct Fd*);
static int pipe_read(struct Fd* fd, void* buf, u_int n,u_int offset);
static int pipe_stat(struct Fd* fd, struct Stat*);
static int pipe_write(struct Fd* fd, const void* buf, u_int n, u_int offset);

struct Dev devpipe = {
	.dev_id = 'p',
	.dev_name = "pipe",
	.dev_read = pipe_read,
	.dev_write = pipe_write,
	.dev_stat = pipe_stat,
	.dev_close = pipe_close,
};

#define BY2PIPE 32

struct Pipe {
	u_int p_rpos;
	u_int p_wpos;
	u_char p_buf[BY2PIPE];
};

int pipe(int pfd[2]) {
	int r;
	void* va;
	struct Fd* fd0,* fd1;
	//debugf("fd0ref is %08x,fd1ref is %08x,pipe ref is %08x\n",pageref(fd0),pageref(fd1),pageref(fd2data(fd0)));
	if ((r = fd_alloc(&fd0)) < 0 || (r = ecall_mem_alloc(0,fd0,PTE_R | PTE_W | PTE_U | PTE_LIBRARY)) < 0) {
		goto err;
	}
	if ((r = fd_alloc(&fd1)) < 0 || (r = ecall_mem_alloc(0,fd1, PTE_R | PTE_W | PTE_U | PTE_LIBRARY)) < 0) {
		goto err1;
	}
	//debugf("fd0ref is %08x,fd1ref is %08x,pipe ref is %08x\n",pageref(fd0),pageref(fd1),pageref(fd2data(fd0)));
	va = fd2data(fd0);
	if ((r = ecall_mem_alloc(0,(void*)va, PTE_R | PTE_W | PTE_U | PTE_LIBRARY) < 0)) {
		goto err2;		
	}
	if ((r = ecall_mem_map(0,va,0,(void*)fd2data(fd1), PTE_R | PTE_W | PTE_U | PTE_LIBRARY)) < 0) {
		goto err3;
	}
	//debugf("fd0ref is %08x,fd1ref is %08x,pipe ref is %08x\n",pageref(fd0),pageref(fd1),pageref(fd2data(fd0)));
	fd0->fd_dev_id = devpipe.dev_id;
	fd0->fd_omode = O_RDONLY;
	fd1->fd_dev_id = devpipe.dev_id;
	fd1->fd_omode = O_WRONLY;
	debugf("[%08x] pipecreate \n",env->env_id);
	pfd[0] = fd2num(fd0);
	pfd[1] = fd2num(fd1);
	return 0;
err3:
	ecall_mem_unmap(0,(void*)va);
err2:
	ecall_mem_unmap(0,fd1);
err1:
	ecall_mem_unmap(0,fd0);
err:
	return r;
}

static int _pipe_is_closed(struct Fd* fd, struct Pipe* p) {
	int fd_ref, pipe_ref, runs;
	do {
		runs = env->env_runs;
		fd_ref = pageref(fd);
		pipe_ref = pageref(p);
	} while(runs != env->env_runs);
	return (fd_ref == pipe_ref);
}

static int pipe_read(struct Fd* fd, void* vbuf, u_int n, u_int offset) {
	int i;
	
	//debugf("read,fd is %08x,,fd ref is %08x,pipe ref is %08x\n",fd2num(fd),pageref(fd),pageref(fd2data(fd)));
	struct Pipe* p = (struct Pipe*)fd2data(fd);
	char* rbuf = (char*)vbuf;
	for (i = 0; i < n;) {
		if (p->p_rpos == p->p_wpos) {
			if (i > 0 || _pipe_is_closed(fd,p) == 1) {
				return i;
			} else {
	//			debugf("read yield!,the buf is %d\n",rbuf[i]);
				ecall_yield();
			}
		} else {
			rbuf[i] = p->p_buf[(p->p_rpos) % BY2PIPE];
			++i;
			p->p_rpos += 1;
	//debugf("rbuf is %s\n",rbuf);
		}
	}
	return n;
}

static int pipe_write(struct Fd* fd, const void* vbuf, u_int n, u_int offset) {
	int i;
	char* wbuf = (char*)vbuf;
	struct Pipe* p =  (struct Pipe*)fd2data(fd);
	for (i = 0; i < n;) {
		if (p->p_wpos - p->p_rpos == BY2PIPE) {
			if (_pipe_is_closed(fd,p) == 1) {
				return i;
			} else {
				ecall_yield();
			}
		} else {
			p->p_buf[(p->p_wpos) % BY2PIPE] = wbuf[i];
			++i;
			p->p_wpos += 1;
		}
	}
	return n;
}

int pipe_is_closed(int fdnum) {
	struct Fd* fd;
	struct Pipe* p;
	int r;
	if ((r = fd_lookup(fdnum,&fd)) < 0) {
		return r;
	}
	p = (struct Pipe*)fd2data(fd);
	return _pipe_is_closed(fd,p);
}

static int pipe_close(struct Fd* fd) {
	ecall_mem_unmap(0,fd);
	ecall_mem_unmap(0,(void*)fd2data(fd));
	return 0;
}

static int pipe_stat(struct Fd* fd, struct Stat* stat) {
	return 0;
}











