#ifndef __VIRTIO_H_
#define __VIRTIO_H_
#include <drivers/virtio_types.h>

#define RESET 0x0
#define ACKNOWLEDGE 0x01
#define DRIVER 0x02
#define FAILED 0x80
#define FEATHERS_OK 0x8
#define DRIVER_OK 0x4
#define DEVICE_NEEDS_RESET 0x40

//reserved feather bits for device and driver feathers

#define VIRTIO_F_ANY_LAYOUT 27
#define VIRTIO_F_INDIRECT_DESC 28
#define VIRTIO_F_EVENT_IDX 29
#define VIRTIO_F_VERSION_1 32
#define VIRTIO_F_ACCESS_PLATFORM 33
#define VIRTIO_F_RING_PACKED 34
#define VIRTIO_F_IN_ORDER 35
#define VIRTIO_F_SR_IOV 37
#define VIRTIO_F_NOTIFICATION_DATA 38
#define VIRTIO_F_NOTIF_CONFIG_DATA 39
#define VIRTIO_F_RING_RESET 40



#define GET(bs,addr) *(_u32*)(bs + addr)

#define QUEUE_SIZE 128


//for virtq_desc flags;
#define VIRTQ_DESC_F_NEXT 1 
#define VIRTQ_DESC_F_WRITE 2
#define VIRTQ_DESC_F_INDIRECT 4


struct virtq_desc {
	_u64 addr;
	_u32 len;
	_u16 flags;
	_u16 next;
};

//for avail flags;
#define VIRTQ_AVAIL_F_INTERRUPT 1
struct virtq_avail {
	_u16 flags;
	_u16 idx;
	_u16 ring[QUEUE_SIZE];
	_u16 used_event;
};

struct virtq_used_elem {
	_u32 id;
	_u32 len;
};


struct virtq_used {
	_u16 flags;
	_u16 idx;
	struct virtq_used_elem ring[QUEUE_SIZE];
	_u16 avail_event;



};






#endif
