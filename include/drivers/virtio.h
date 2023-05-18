#ifndef __VIRTIO_H__
#define __VIRTIO_H__

#include <pmap.h>

/*  */
// mark buffer as continuing via the next field
#define VIRTQ_DESC_F_NEXT		1
// make buffer as write-only ohterwise read-only
#define VIRTQ_DESC_F_WRITE		1
// buffer contains a list of buffer descriptors
#define VIRTQ_DESC_F_INDIRECT 	1

// these are used for used->flags to advise the driver: dont't kick me when you add a buffer??????? not reliable
#define VIRTQ_USED_F_NO_NOTIFY  1
// this is used for avail->flags to advise the device : : don't interrupt me when you consume a buffer. not reliable
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1

//support for indirect descriptors
#define VIRTIO_F_INDIRECT_DESC   28
//support for avail_event and used_event fields
#define VIRTIO_F_EVENT_IDX		29
//arbitrary descriptor layouts	
#define VIRTIO_F_ANY_LAYOUT		27






#define QUEUE_SIZE 8


//virtqueue descriptor,the size is 16 bytes, wht use u64 may be 
//for padding. These desc can chain together via next.
struct virtq_desc {
	//physical in guest
	uint64_t addr;
	uint32_t len;
	//mentioned above.
	uint16_t flags;
	//next field if flags is NEXT
	uint16_t next;
};




struct virtq_avail {

	uint16_t flags;
	uint16_t idx;		//
	uint16_t ring[QUEUE_SIZE];		// queue size ?
	uint16_t used_event;//only if virto_f_event_idx
};

struct virtq_used_elem {
//the index of start of used descriptor chain
	uint32_t id;
//the total length of the descriptor chain
	uint32_t len;
// we use u32 because of padding
};

struct virtq_used {

	uint16_t flags;
	uint16_t idx;
	struct virtq_used_elem ring[QUEUE_SIZE];
};

struct virtq {
	unsigned int num;
	struct virtq_desc *desc;
	struct virtq_avail *avail;
	struct virtq_used *used;
};

static inline int virtq_need_event(uint16_t event_idx,uint16_t new_idx, uint16_t old_idx) {
	return (uint16_t)(new_idx - event_idx - 1) < (uint16_t)(new_idx - old_idx);
}

static inline uint16_t* virtq_avail_event(struct virtq * va) {
	return (uint16_t*) &va->used->ring[va->num];
}



int read_sector(u_int secno,void* dst);
int write_sector(u_int secno,void* src);






#endif
