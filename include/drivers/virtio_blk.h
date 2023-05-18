#ifndef TESTMACHINE_DISK_H
#define TESTMACHINE_DISK_H
#include <asm/barrier.h>
#include <drivers/virtio_types.h>
#include <drivers/virtio.h>


#define RESET 0
#define ACKNOWLEDGE 1
#define DRIVER 2
#define FAILED 128
#define FEATHERS_OK 8
#define DRIVER_OK 4
#define DRIVER_NEEDS_RESET 64



#define DEV_DISK_REGADDRESS 0x10008000
#define DEV_DISK_MAGICVALUE 0x0
#define DEV_DISK_VERSION	0x4
#define DEV_DISK_DEVICEID 0x8
#define DEV_DISK_VENDORID 0xc
#define DEV_DISK_DEVICEFEATHERS 0x10
#define DEV_DISK_DEVICEFEATHER_SEL 0x14
#define DEV_DISK_DRIVERFEATHERS  0x20
#define DEV_DISK_DRIVERFEATHERS_SEL 0x24
#define DEV_DISK_QUEUE_SEL		0x30
#define DEV_DISK_QUEUENUM_MAX	0x34
#define DEV_DISK_QUEUENUM 		0x38
#define DEV_DISK_QUEUEREADY 	0x44
#define DEV_DISK_QUEUENOTIFY	0x50
#define DEV_DISK_INTERRUPTSTATUS 0x60
#define DEV_DISK_INTERRUPTACK	0x64
#define DEV_DISK_STATUS 0x70
#define DEV_DISK_QUEUEDESC_LOW	0x80
#define DEV_DISK_QUEUEDESC_HIGH 0x84
#define DEV_DISK_QUEUEAVAIL_LOW 0x90
#define DEV_DISK_QUEUEAVAIL_HIGH 0x94
#define DEV_DISK_QUEUEUSED_LOW 0xa0
#define DEV_DISK_QUEUEUSED_HIGH 0xa4
#define DEV_CONFIG_GENERATION	0xfc
#define DEV_DISK_CONFIG			0x100

#define VIRTIO_BLK_F_SIZE_MAX 		1
#define VIRTIO_BLK_F_SEG_MAX  		2
#define VIRTIO_BLK_F_GEOMETRY 		4
#define VIRTIO_BLK_F_RO		  		5
#define VIRTIO_BLK_F_BLK_SIZE 		6
#define VIRTIO_BLK_F_FLUSH	  		9
#define VIRTIO_BLK_F_TOPOLOGY 		10
#define VIRTIO_BLK_F_CONFIG_WCE		11

//legacy interface feather bits
#define VIRTIO_BLK_F_BARRIER 		0
#define VIRTIO_BLK_F_SCSI			7


#define SEC_SIZE 512

#define DEV_ADDR(base,offset)  *(int*)(base + offset)
#define virt_mb()		__smp_mb()
#define virt_rmb()		__smp_rmb()
#define virt_wmb()		__smp_wmb()


static inline void virtio_mb() {
		virt_mb();
}

static inline void virtio_rmb() {
		virt_rmb();
}

static inline void virtio_wmb() {
		virt_wmb();
}

//type code
#define VIRTIO_BLK_T_IN 		0
#define VIRTIO_BLK_T_OUT 		1
#define VIRTIO_BLK_T_FLUSH  	4

//status code
#define VIRTIO_BLK_S_OK			0
#define VIRTIO_BLK_S_IOERR  	1
#define VIRTIO_BLK_S_UNSPP  	2


struct virtq_blk_req {
	_u32 type;
	_u32 reserved;
	_u64  sector;
	//1 sector`
	//_u8 *data;
	//_u8 status;
};

struct virtio_blk_config {
	_u64 capacity;
	_u32 size_max;
	_u32 seg_max;
	struct virtio_blk_geometry {
		_u16 cylinders;
		_u8 heads;
		_u8 sectors;
	} geometry;
	_u32 blk_size;
	struct virtio_blk_topology {
		_u8 physical_block_exp;
		_u8 alignment_offset;
		_u16 min_io_size;
		_u32 opt_io_size;
	} topology;
	_u8 writeback;
};

struct virtio_blk {
	//struct virtio_device dev;
	struct virtio_blk_config config;
	struct virtq_desc * desc;
	struct virtq_avail* avail;
	struct virtq_used* used;
	struct virtq_blk_req req[QUEUE_SIZE];
	_u8 free[QUEUE_SIZE / 8];

};


void disk_rw(_u32 sector, _u32 write,void* adhr);
void disk_init(void);
void disk_test();










#endif /*  TESTMACHINE_DISK_H  */
