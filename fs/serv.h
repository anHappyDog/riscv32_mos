#include <fs.h>
#include <lib.h>
#include <mmu.h>

#define BY2SECT 512
#define SECT2BLK (BY2BLK / BY2SECT)

#define DISKMAP 0x10000000
#define DISKMAX 0x40000000


int file_open(char* path, struct File** pfile);
int file_get_block(struct File* f, u_int blockno,void** pblk);
int file_set_size(struct File* f,u_int newsize);
void file_close(struct File* f);
int file_remove(char* path);
int file_dirty(struct File* f, u_int offset);
void file_flush(struct File*);

void fs_init(void);
void fs_sync(void);
extern uint32_t* bitmap;
int map_block(u_int);
int alloc_block(void);
