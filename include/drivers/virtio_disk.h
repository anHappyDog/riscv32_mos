#ifndef _VIRTIO_DISK_H_
#define _VIRTIO_DISK_H_
#include <drivers/virtio_types.h>
#include <drivers/virtio.h>

#define MAGIC_VALUE 0x74726976
#define VERSION 0x2
#define DEVICEID 0x2
#define VENDORID 0x554d4551

#define DISK_ADDRESS 0x10008000
#define DISK_LENGTH 0x1000
#define DISK_MAGIC_VALUE 0x000
#define DISK_VERSION 0x004
#define DISK_DEVICEID 0x008
#define DISK_VENDORID 0x00c
#define DISK_DEVICEFEATHERS 0x010
#define DISK_DEVICEFEATHERS_SEL 0x014
#define DISK_DRIVERFEATHERS 0x020
#define DISK_DRIVERFEATHERS_SEL 0x024
#define DISK_QUEUE_SEL 0x030
#define DISK_QUEUENUM_MAX 0x034
#define DISK_QUEUENUM 	0x038
#define DISK_QUEUEREADY 0x044
#define DISK_QUEUENOTIFY 0x050
#define DISK_INTERRUPT_STATUS 0x060
#define DISK_INTERRUPT_ACK 0x064
#define DISK_STATUS 0x070
#define DISK_QUEUEDESC_LOW 0x080
#define DISK_QUEUEDESC_HIGH 0x084
#define DISK_AVAIL_LOW 0x090
#define DISK_AVAIL_HIGH 0x094
#define DISK_USED_LOW 0x0a0
#define DISK_USED_HIGH 0x0a4
#define DISK_CONFIG_GENERATION 0x0fc
#define DISK_CONFIG 0x100


//Feather bits
#define VIRTIO_BLK_F_SIZE_MAX 1
#define VIRTIO_BLK_F_SEG_MAX 2
#define VIRTIO_BLK_F_GEOMETRY 4
#define VIRTIO_BLK_F_RO	5
#define VIRTIO_BLK_F_BLK_SIZE 6
#define VIRTIO_BLK_F_FLUSH 9
#define VIRTIO_BLK_F_TOPOLOGY 10
#define VIRTIO_BLK_F_CONFIG_WCE 11
#define VIRTIO_BLK_F_MQ 12
#define VIRTIO_BLK_F_DISCARD 13
#define VIRTIO_BLK_F_WRITE_ZEROES 14
#define VIRTIO_BLK_F_LIFETIME 15
#define VIRTIO_BLK_F_SECURE_ERASE 16



//config
struct virtio_blk_config {
	_u64 capacity;
	_u32 size_max;
	_u32 seg_max;
	struct virtio_blk_geometry {
		_u16 cylinders;
		_u8 heads;
		_u8 sectors;
	}geometry;
	_u32 blk_size;
	struct virtio_blk_topology {
		_u8 physical_block_exp;
		_u8 alignment_offset;
		_u16 min_io_size;
		_u32 opt_io_size;
	} topology;
	_u8 writeback;
};

// type for virtio_blk_req
#define VIRTIO_BLK_T_IN    0
#define VIRTIO_BLK_T_OUT   1
#define VIRTIO_BLK_T_FLUSH 4

// status for virtio_blk_req
#define VIRTIO_BLK_S_OK    0
#define VIRTIO_BLK_S_IOERR 1
#define VIRTIO_BLK_S_UNSPP 2

struct virtio_blk_req {
	_u32 type;
	_u32 reserved;
	_u64 sector;
	_u8 *data;
	_u8 status;
};


struct virtqueue {
	struct virtq_desc* desc;
	struct virtq_avail* avail;
	struct virtq_used* used;
	struct virtio_blk_req reqs[QUEUE_SIZE];
	_u8 last_seen_used;
	_u8 free;

};

void disk_init(void);
void disk_rw(_u64 sector,int wirte,void*buf);


#endif
