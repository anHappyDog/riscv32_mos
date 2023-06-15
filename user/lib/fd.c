#include <env.h>
#include <fd.h>
#include <lib.h>
#include <mmu.h>


static struct Dev* devtab[] = {&devcons,&devfile,&devpipe,0};

int dev_lookup(int dev_id, struct Dev** dev) {
	for (int i = 0; devtab[i]; ++i) {
		if (devtab[i]->dev_id == dev_id) {
			*dev = devtab[i];
			return 0;
		}
	}
	debugf("[%08x] unknown device type %d\n",env->env_id,dev_id);
	return -E_INVAL;
}


int fd_alloc(struct Fd** fd) {
	u_int va;
	u_int fdno;
	for (fdno = 0; fdno < MAXFD - 1; ++fdno) {
		va = INDEX2FD(fdno);
		if ((vpd[va / PDMAP] & PTE_V) == 0) {
			*fd = (struct Fd*)va;
			return 0;
		}
		if ((vpt[va / BY2PG] & PTE_V) == 0) {
			*fd = (struct Fd*)va;
			return 0;
		}
	
	}
	return -E_MAX_OPEN;
}

void fd_close(struct Fd* fd) {
	ecall_mem_unmap(0,fd);
}

int fd_lookup(int fdnum,struct Fd** fd) {
	u_int va;
	if (fdnum >= MAXFD) {
		return -E_INVAL;
	}
	va = INDEX2FD(fdnum);
	if ((vpt[va / BY2PG] & PTE_V) != 0) {
		*fd = (struct Fd*)va;
		return 0;
	}	
	return -E_INVAL;
}

void* fd2data(struct Fd* fd) {
	return (void*)INDEX2DATA(fd2num(fd));
}

int fd2num(struct Fd* fd) {
	return ((u_int)fd - FDTABLE) / BY2PG;
}
int num2fd(int fd) {
	return fd * BY2PG + FDTABLE;
}

int close(int fdnum) {
	int r;
	struct Dev* dev = NULL;
	struct Fd* fd;
	if ((r = fd_lookup(fdnum,&fd)) < 0 || (r = dev_lookup(fd->fd_dev_id,&dev)) < 0) {
		return r;
	}
	r = (*dev->dev_close)(fd);
	fd_close(fd);
	return r;
}

void close_all(void) {
	int i;
	for (i = 0; i < MAXFD; ++i) {
		close(i);
	}
}

int dup(int oldfdnum,int newfdnum) {
	int i,r;
	void* ova,*nva;
	u_int pte;
	struct Fd* oldfd,*newfd;
	if ((r = fd_lookup(oldfdnum,&oldfd)) < 0) {
		return r;
	}
	close(newfdnum);
	newfd = (struct Fd*)INDEX2FD(newfdnum);
	ova = fd2data(oldfd);
	nva = fd2data(newfd);
	//NOTICE
	if (vpd[PDX(ova)]) {
		for (i = 0; i < PDMAP; i += BY2PG) {
			pte = vpt[VPN(ova + i)];
			if (pte & PTE_V) {
				if ((r = ecall_mem_map(0,(void*)(ova + i),0,(void*)(nva + i),((pte & (PTE_LIBRARY | PTE_R | PTE_W | PTE_U | PTE_DIRTY))))) < 0) {
				//	debugf("r is %d\n",r);
					//debugf("srcva is %08x,ova + i is %08x\n",ova,ova + i);
					goto err;	
				}
			}
		}

	}
	if ((r = ecall_mem_map(0,oldfd,0,newfd,((vpt[VPN(oldfd)] & (PTE_LIBRARY | PTE_R | PTE_W | PTE_U | PTE_DIRTY))))) < 0) {
		debugf("faaaaaaaaaaaabbb\n");
		goto err;
	}
	/*struct Fd* fddd = (struct Fd*)INDEX2FD(newfdnum);
	debugf("fddd->fd_omode is %08x\n",fddd->fd_omode);*/
	return newfdnum;
err:			
	ecall_mem_unmap(0,newfd);
	for (i = 0; i < PDMAP; i += BY2PG) {
		ecall_mem_unmap(0,(void*)(nva + i));
	}
	return r;
}

int read(int fdnum, void*buf, u_int n) {
	int r;
	struct Dev* dev;
	struct Fd* fd;
	if ((r = fd_lookup(fdnum,&fd)) < 0 || (r = dev_lookup(fd->fd_dev_id,&dev)) < 0) {
		return r;
	}
	if ((fd->fd_omode & O_ACCMODE) == O_WRONLY) {
		return -E_INVAL;
	}
	r = dev->dev_read(fd,buf,n,fd->fd_offset);
	if (r > 0) {
		fd->fd_offset += r;
	}
	return r;
}

int readn(int fdnum, void* buf, u_int n ) {
	int m,tot;
	for (tot = 0; tot < n; tot += m) {
		m = read(fdnum,(char*)buf + tot,n - tot);
		if (m < 0) {
			return m;
		}
		if (m == 0) {
			break;
		}
	}
	return tot;
}

int write(int fdnum, const void* buf, u_int n) {
	int r;
	struct Dev* dev;
	struct Fd* fd;
	if ((r = fd_lookup(fdnum,&fd)) < 0 || (r = dev_lookup(fd->fd_dev_id,&dev)) < 0) {
		return r;
	}
	if ((fd->fd_omode & O_ACCMODE) == O_RDONLY) {
	//	debugf("fd->fd_omode is %08x\n",fd->fd_omode);
		return -E_INVAL;
	}
	r = dev->dev_write(fd,buf,n,fd->fd_offset);
	if (r > 0) {
		fd->fd_offset += r;
	}
	return r;
}

int seek(int fdnum,u_int offset) {
	int r;
	struct Fd* fd;
	if ((r = fd_lookup(fdnum,&fd)) < 0) {
		return r;
	}
	fd->fd_offset = offset;
	return 0;
}

int fstat(int fdnum,struct Stat* stat) {
	int r;
	struct Dev* dev = NULL;
	struct Fd* fd;
	if ((r = fd_lookup(fdnum,&fd)) < 0 || (r = dev_lookup(fd->fd_dev_id,&dev)) < 0) {
		return r;
	}
	stat->st_name[0] = 0;
	stat->st_size = 0;
	stat->st_isdir = 0;
	stat->st_dev = dev;
	return (*dev->dev_stat)(fd,stat);
}

int stat(const char* path, struct Stat* stat) {
	int fd,r;
	if ((fd = open(path,O_RDONLY)) < 0) {
		return fd;
	}
	r = fstat(fd,stat);
	close(fd);
	return r;
}
