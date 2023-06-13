#include "serv.h"
#include <mmu.h>
#include <drivers/virtio_disk.h>


volatile Pde* fs_pgdir;

struct Super* super;
uint32_t* bitmap;

void file_flush(struct File*);
int block_is_free(u_int);

void* diskaddr(u_int blockno) {
	uint32_t t = DISKMAP + BY2BLK * blockno;
	if (t > DISKMAX + DISKMAP) {
		user_panic("the blockno is beyond disk's capacity!\n");
	}
	return (void*)t;
}

int va_is_mapped(void* va) {
	//	return () ;
	return ecall_check_address(va,0,0) == 0 ? 1 : 0;
}

void* block_is_mapped(u_int blockno) {
	void* va = diskaddr(blockno);
	if (va_is_mapped(va)) {
		return va;
	}
	return NULL;
}

int va_is_dirty(void *va) {
	// is there anything wrong that PTE_DIRTY?
	return GET_VPT(fs_pgdir,va) & PTE_DIRTY;
}

int read_block(u_int blockno, void** blk, u_int *isnew) {
	if (super && blockno >= super->s_nblocks) {
		user_panic("reading non-existent block %08x\n",blockno);
	}
	if (bitmap && block_is_free(blockno)) {
		user_panic("reading free block %08x\n",blockno);
	}
	void *va = diskaddr(blockno);
	if (block_is_mapped(blockno)) {
		if (isnew) {
			*isnew = 0;
		}
	}
	else {
		if (isnew) {
			*isnew = 1;
		}
		ecall_mem_alloc(0,va,PTE_D | PTE_R | PTE_W | PTE_U);
		ecall_read_disk(blockno*SECT2BLK,0,va,SECT2BLK);
	}
	if (blk) {
		*blk = va;
	}
	return 0;
}

int map_block(u_int blockno) {
	if (block_is_mapped(blockno) != NULL) {
		return 0;
	}
	return ecall_mem_alloc(ecall_getenvid(),(void*)diskaddr(blockno),PTE_D | PTE_R | PTE_W | PTE_U);
}

int block_is_dirty(u_int blockno) {
	void* va = diskaddr(blockno);
	return va_is_mapped(va) && va_is_dirty(va);
}

void write_block(u_int blockno) {
	if (!block_is_mapped(blockno)) {
		user_panic("write unmapped block %08x\n",blockno);
	}
	void* va = diskaddr(blockno);
//	debugf("block * SECT2BLK is %d\n",blockno * SECT2BLK);
//	debugf("write back va is ;;%s;;\n",(char*)va);
//	char buf[4099];
//	ecall_read_disk(blockno * SECT2BLK,0,buf,SECT2BLK);
//	debugf("before write va is ;;;%s;;;\n",buf);
	ecall_write_disk(blockno * SECT2BLK,0,va,SECT2BLK);
}

void unmap_block(u_int blockno) {
	void* va;
	va = block_is_mapped(blockno);
	if (!block_is_free(blockno) && block_is_dirty(blockno)) {
		write_block(blockno);
	}
	ecall_mem_unmap(ecall_getenvid(),va);
	user_assert(!block_is_mapped(blockno));

}

int dirty_block(u_int blockno) {
	void* va = diskaddr(blockno);
	if (!va_is_mapped(va)) {
		return -E_NOT_FOUND;
	}
	if (va_is_dirty(va)) {
		return 0;
	}
 	return ecall_mem_map(0,va,0,va,PTE_D | PTE_DIRTY | PTE_R | PTE_W | PTE_U);
}

int block_is_free(u_int blockno) {
	if (super == 0 || blockno >= super->s_nblocks) {
		return 0;
	}
	if (bitmap[blockno / 32 ] & (1 << (blockno % 32))) {
		return 1;
	}
	return 0;
}

void free_block(u_int blockno) {
	if (blockno == 0 || blockno >= super->s_nblocks) {
		return;
	}
	bitmap[blockno / 32] |= (1 << (blockno % 32));
}

int alloc_block_num(void) {
	int blockno;
	for (blockno = 3; blockno < super->s_nblocks; ++blockno) {
		if (block_is_free(blockno)) {
			bitmap[blockno / 32] &= ~(1 << (blockno % 32));
			write_block(blockno / BIT2BLK + 2);
			return blockno;
		}
	}
	return -E_NO_DISK;
}

int alloc_block(void) {
	int r,bno;
	if ((r = alloc_block_num()) < 0) {
		return r;
	}
	bno = r;
	if ((r = map_block(bno)) < 0) {
		free_block(bno);
		return r;
	}
	return bno;
}

void read_super(void) {
	int r;
	void* blk;
	if ((r = read_block(1,&blk,0)) < 0) {
		user_panic("cannot read superblock: %e", r);
	}
	super = blk;
	if (super->s_magic != FS_MAGIC) {
		user_panic("bad file system magic number %08x\n",super->s_magic);
	}
	if (super->s_nblocks > DISKMAX / BY2BLK) {
		user_panic("file system is too large\n");
	}
	debugf("superblock is good\n");
}

void read_bitmap(void) {
	u_int i;
	void* blk = NULL;
	u_int nbit_map = super->s_nblocks / BIT2BLK + 1;
	for (i = 0; i < nbit_map;++i) {
		read_block(i + 2,blk,0);
	}
	bitmap = diskaddr(2);
	user_assert(!block_is_free(0));
	user_assert(!block_is_free(1));
	for (i = 0; i < nbit_map; ++i) {
		user_assert(!block_is_free(i + 2));
	}
	debugf("read bit map is good\n");


}

void check_write_block(void) {
	super = 0;
	read_block(0,0,0);
	memcpy((char*)diskaddr(0),(char*)diskaddr(1),BY2PG);
	strcpy((char*)diskaddr(1),"OOPS!\n");
	write_block(1);
	user_assert(block_is_mapped(1));

	ecall_mem_unmap(0,diskaddr(1));
	user_assert(!block_is_mapped(1));
	read_block(1,0,0);
	user_assert(strcmp((char*)diskaddr(1),"OOPS!\n") == 0);

	memcpy((char*)diskaddr(1),(char*)diskaddr(0),BY2PG);
	write_block(1);
	super = (struct Super*)diskaddr(1);
}



void fs_init(void) {
	if(ecall_get_pgdir((Pde**)&fs_pgdir) != 0) {
		user_panic("fs_init get fs_pgdir failed!\n");
	}
	read_super();
	check_write_block();
	read_bitmap();
}

int file_block_walk(struct File* f,u_int filebno, uint32_t **ppdiskbno, u_int alloc) {
	int r;
	uint32_t *ptr;
	uint32_t *blk;
	if (filebno < NDIRECT) {
		ptr = &f->f_direct[filebno];
	} else if (filebno < NINDIRECT) {
		if (f->f_indirect == 0) {
			if (alloc == 0) {
				return -E_NOT_FOUND;
			}
			if ((r = alloc_block()) < 0) {
				return r;
			}
			f->f_indirect = r;
		}
		if ((r = read_block(f->f_indirect,(void**)&blk,0)) < 0) {
			return r;
		
		}
	   ptr = blk + filebno;	
	} else {
		return -E_INVAL;
	}
	*ppdiskbno = ptr;
	return 0;
}

int file_map_block(struct File* f, u_int filebno, u_int* diskbno, u_int alloc) {
	int r;
	uint32_t *ptr;
	if ((r = file_block_walk(f,filebno,&ptr,alloc)) < 0) {
		return r;
	}
	//debugf("ptr is %08x\n, *ptr is %d\n",ptr,*ptr);
	if (*ptr == 0) {
		if (alloc == 0) {
			return -E_NOT_FOUND;
		}
		if ((r = alloc_block()) < 0) {
			return r;
		}
		*ptr = r;
	}
	*diskbno = *ptr;
	return 0;

}

int file_clear_block(struct File *f,u_int filebno) {
	int r;
	uint32_t *ptr;
	if ((r = file_block_walk(f,filebno,&ptr,0)) < 0) {
		return r;
	}
	if (*ptr) {
		free_block(*ptr);
		*ptr = 0;
	}
	return 0;
}

int file_get_block(struct File* f,u_int filebno,void**blk) {
	int r;
	u_int diskbno;
	u_int isnew;
	if ((r = file_map_block(f,filebno,&diskbno,1)) < 0) {
		return r;
	}
	if ((r = read_block(diskbno,blk,&isnew)) < 0) {
		return r;
	}
	return 0;
}

int file_dirty(struct File* f, u_int offset) {
	int r;
	u_int diskbno;
	if ((r = file_map_block(f,offset / BY2BLK,&diskbno,0)) < 0) {
		return r;
	}
	return dirty_block(diskbno);
}


int dir_lookup(struct File* dir, char* name, struct File** file) {
	u_int nblock;
	nblock = dir->f_size / BY2BLK;
	for (int i = 0; i < nblock; ++i) {
		void* blk;
		try(file_get_block(dir,i,&blk));
		struct File *files = (struct File*)blk;
		for (struct File* f = files; f < files + FILE2BLK; ++f) {
			if (strcmp(name,f->f_name) == 0) {
				*file = f;
				f->f_dir = dir;
				return 0;
			}
		}
	}
	return -E_NOT_FOUND;
}


int dir_alloc_file(struct File* dir, struct File** file) {
	int r;
	u_int nblock,i,j;
	void* blk;
	struct File* f;
	nblock = dir->f_size / BY2BLK;
	for (i = 0; i < nblock; ++i) {
		if ((r = file_get_block(dir,i,&blk)) < 0) {
			return r;
		}
		f = blk;
		for (j = 0; j < FILE2BLK; ++j) {
			if (f[j].f_name[0] == 0) {
				*file = &f[j];
				return 0;
			}
		}
	}
	dir->f_size += BY2BLK;
	if ((r = file_get_block(dir,i,&blk)) < 0) {
		return r;
	}
	f = blk;
	*file = &f[0];
	return 0;
}


char* skip_slash(char* p) {
	while(*p == '/') {
		++p;
	}
	return p;
}


int walk_path(char* path,struct File** pdir,struct File** pfile,char*lastelem) {
	char* p;
	char name[MAXNAMELEN];
	struct File* dir,*file;
	int r;
	path = skip_slash(path);
	file = &super->s_root;
	dir = 0;
	name[0] = 0;
	if (pdir) {
		*pdir = 0;
	}
	*pfile = 0;
	while (*path != 0) {
		dir = file;
		p = path;
		while (*path != '/' && *path != 0) {
			++path;
		}
		if (path - p >= MAXNAMELEN) {
			return -E_BAD_PATH;
		}
		memcpy(name,p,path-p);
		name[path - p] = 0;
		path = skip_slash(path);
		if (dir->f_type != FTYPE_DIR) {
			return -E_NOT_FOUND;
		}
		//debugf("super->s_root is %s\n",super->s_root.f_name);
		//debugf("dir is %s: name is %s\n",dir->f_name,name);
		if ((r = dir_lookup(dir,name,&file)) < 0) {
			if (r == -E_NOT_FOUND && *path == 0) {
				if (pdir) {
					*pdir = dir;
				}
				if (lastelem) {
					strcpy(lastelem,name);
				}
				*pfile = 0;
			}
			return r;
		}
	}
	if (pdir) {
		*pdir = dir;
	}
	*pfile = file;
	return 0;
}


int file_open(char* path,struct File** file) {
	return walk_path(path,0,file,0);
}

int file_create(char* path,struct File** file) {
	char name[MAXNAMELEN];
	int r;
	struct File* dir,*f;
	if ((r = walk_path(path,&dir,&f,name)) == 0) {
		return -E_FILE_EXISTS;
	}
	if (r != -E_NOT_FOUND || dir == 0) {
		return r;
	}
	if (dir_alloc_file(dir,&f) < 0) {
		return r;
	}
	strcpy(f->f_name,name);
	*file = f;
	return 0;

}


void file_truncate(struct File* f, u_int newsize) {
	u_int bno,old_nblocks,new_nblocks;
	old_nblocks = f->f_size / BY2BLK + 1;
	new_nblocks = newsize / BY2BLK + 1;
	if (newsize == 0) {
		new_nblocks = 0;
	}
	if (new_nblocks <= NDIRECT) {
		for (bno = new_nblocks; bno < old_nblocks; ++bno) {
			file_clear_block(f,bno);
		}
		if (f->f_indirect) {
			free_block(f->f_indirect);
			f->f_indirect = 0;
		}
	} else {
		for (bno = new_nblocks; bno < old_nblocks; ++bno) {
			file_clear_block(f,bno);
		}
	}
	f->f_size = newsize;
}

int file_set_size(struct File* f, u_int newsize) {
	if (f->f_size > newsize) {
		file_truncate(f,newsize);
	}
	f->f_size = newsize;
	if (f->f_dir) {
		file_flush(f->f_dir);
	}
	return 0;
}

void file_flush(struct File* f) {
	u_int nblocks;
	u_int bno;
	u_int diskno;
	int r;
	nblocks = f->f_size / BY2BLK + 1;
	for (bno = 0; bno < nblocks; ++bno) {
		if ((r = file_map_block(f,bno,&diskno,0)) < 0) {
			continue;
		}

		if (block_is_dirty(diskno)) {
			write_block(diskno);
		}
	}
}



void fs_sync(void) {
	int i;
	for (i = 0; i < super->s_nblocks; ++i) {
		if (block_is_dirty(i)) {
			write_block(i);
		}
	}

}


void file_close(struct File* f) {
	file_flush(f);
	if (f->f_dir) {
		file_flush(f->f_dir);
	}
}

int file_remove(char* path) {
	int r;
	struct File* f;
	if ((r = walk_path(path,0,&f,0)) < 0) {
		return r;
	}	
	file_truncate(f,0);
	f->f_name[0] = 0;
	file_flush(f);
	if (f->f_dir) {
		file_flush(f->f_dir);
	}
	return 0;
}


