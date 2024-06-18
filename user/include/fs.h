#ifndef _FS_H_
#define _FS_H_ 1

#include <stdint.h>

#define BY2BLK BY2PG
#define BIT2BLK (BY2BLK * 8)

#define MAXNAMELEN 128
#define MAXPATHLEN 1024
#define NDIRECT 10
#define NINDIRECT (BY2BLK / 4)
#define MAXFILESIZE (NINDIRECT * BY2BLK)
#define BY2FILE 256

struct File {
	char f_name[MAXNAMELEN];
	uint32_t f_size;
	uint32_t f_type;
	uint32_t f_direct[NDIRECT];
	uint32_t f_indirect;
	struct File* f_dir;
	char f_pad[BY2FILE - MAXNAMELEN - (3 + NDIRECT) * 4 -sizeof(void*)];
} __attribute__((aligned(4),packed));

#define FILE2BLK (BY2BLK / sizeof(struct File))

#define FTYPE_REG 0 
#define FTYPE_DIR 1

#define FS_MAGIC 0x68286097

struct Super {
	uint32_t s_magic;
	uint32_t s_nblocks;
	struct File s_root;

};



#endif
