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
	return IS_VAL(fs_pgdir,va);
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
	return GET_VPT(fs_pgdir,va) & PTE_D;
}

int dirty_block(u_int blockno) {
	void* va = diskaddr(blockno);
	if (!va_is_mapped(va)) {
		return -E_NOT_FOUND;
	}

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
		ecall_mem_alloc(0,va,PTE_D);
		disk_rw(0,blockno * SECT2BLK,va,SECT2BLK);
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
	return ecall_mem_alloc(ecall_getenvid(),(void*)diskaddr(blockno),PTE_D);
}

int block_is_dirty(u_int blockno) {
	void* va = diskaddr(blockno);
	return va_is_mapped(va) && va_is_dirty(va);
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
	return ecall_mem_map(0,va,0,va,PTE_D | PTE_DIRTY);
}

void write_block(u_int blockno) {
	if (!block_is_mapped(blockno)) {
		user_panic("write unmapped block %08x\n",blockno);
	}
	void* va = diskaddr(blockno);
	disk_rw(1,blockno * SECT2BLK,va,SECT2BLK);
}

int block_is_free(u_int blockno) {
	if (super == 0 || blockno >= super->s_nblocks) {
		return 0;
	}
	if (bitmap[blockno >> 3] & (1 << (blockno % 0x1f))) {
		return 1;
	}
	return 0;
}

void free_block(u_int blockno) {
	if (blockno == 0 || blockno >= super->s_nblocks) {
		return;
	}
	bitmap[blockno >> 3] |= (1 << (blockno & 0x1f));
}

int alloc_block_num(void) {
	int blockno;
	for (blockno = 3; blockno < super->s_nblocks; ++blockno) {
		if (block_is_free(blockno)) {
			bitmap[blockno >> 3] &= ~(1 << (blockno & 0x1f));
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
	u_int nbit_map = super->s_nblocks / BITBL2 + 1;
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


void fs_init(void) {
	fs_pgdir = ecall_get_pgdir();
}
