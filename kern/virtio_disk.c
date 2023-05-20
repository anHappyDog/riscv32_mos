#include <drivers/virtio.h>
#include <drivers/virtio_disk.h>
#include <drivers/virtio_types.h>
#include <pmap.h>
#include <asm/barrier.h>


struct virtqueue* disk;
void disk_init(void) {
	_u32 status = 0;
	if (GET(DISK_ADDRESS,DISK_MAGIC_VALUE) != MAGIC_VALUE  ||
			GET(DISK_ADDRESS,DISK_VERSION) != VERSION || 
			GET(DISK_ADDRESS,DISK_DEVICEID) != DEVICEID || 
			GET(DISK_ADDRESS,DISK_VENDORID) != VENDORID) {
		panic("failed to init the disk due to the check magic value ...\n");
	}
	GET(DISK_ADDRESS,DISK_STATUS) = status;
	status |= ACKNOWLEDGE;
	GET(DISK_ADDRESS,DISK_STATUS) = status;
	status |= DRIVER;
	GET(DISK_ADDRESS,DISK_STATUS) = status;
	GET(DISK_ADDRESS,DISK_DEVICEFEATHERS_SEL) = 0;
	_u32 feathers = GET(DISK_ADDRESS,DISK_DEVICEFEATHERS);
//	printk("feathers in 31 - 0 bit are %08x\n",feathers);
	feathers &= ~(1 << VIRTIO_BLK_F_RO);
	feathers &= ~(1 << VIRTIO_BLK_F_CONFIG_WCE);
	feathers &= ~(1 << VIRTIO_BLK_F_MQ);
	feathers &= ~(1 << VIRTIO_F_ANY_LAYOUT);	
	feathers &= ~(1 << VIRTIO_F_EVENT_IDX);
	feathers &= ~(1 << VIRTIO_F_INDIRECT_DESC);
//	printk("after set feathers in 31 - 0 bit are %08x\n",feathers);
	GET(DISK_ADDRESS,DISK_DRIVERFEATHERS) = feathers;
	GET(DISK_ADDRESS,DISK_DEVICEFEATHERS_SEL) = 1;
	feathers = GET(DISK_ADDRESS,DISK_DEVICEFEATHERS);
//	printk("feathers in 63 -32 bit are %08x\n",feathers);
	GET(DISK_ADDRESS,DISK_DRIVERFEATHERS_SEL) = 1;
	GET(DISK_ADDRESS,DISK_DRIVERFEATHERS) = feathers;
	status |= FEATHERS_OK;
	GET(DISK_ADDRESS,DISK_STATUS) = status;
	if (!(status & FEATHERS_OK)) {
		panic("disk featehrs is not ok after set!\n");
	}
	if (GET(DISK_ADDRESS,DISK_QUEUEREADY)) {
		panic("disk shouldn't be ready before set!\n");
	}
	_u32 maxq = GET(DISK_ADDRESS,DISK_QUEUENUM_MAX);
	if (maxq == 0 || maxq < QUEUE_SIZE) {
		panic("max queuesize is not enough!\n");
	}	
	struct Page* pp;
	if (page_alloc(&pp) != 0) {
		panic("alloc page failed!\n");
	}
	pp->pp_ref += 1;
	disk = (struct virtqueue*)page2addr(pp);
	if (page_alloc(&pp) != 0) {
		panic("alloc page failed!\n");
	}
	pp->pp_ref += 1;
	disk->desc = (struct virtq_desc*)page2addr(pp);
	if (page_alloc(&pp) != 0) {
		panic("alloc page failed!\n");
	}
	pp->pp_ref += 1;
	disk->avail = (struct virtq_avail*)page2addr(pp);
	if (page_alloc(&pp) != 0) {
		panic("alloc page failed!\n");
	}
	pp->pp_ref += 1;
	disk->used = (struct virtq_used*)page2addr(pp);
	disk->free = 0xff;
	GET(DISK_ADDRESS,DISK_QUEUENUM) = QUEUE_SIZE;
	GET(DISK_ADDRESS,DISK_QUEUEDESC_LOW) = (_u32)disk->desc;
	GET(DISK_ADDRESS,DISK_QUEUEDESC_HIGH) = 0;
	GET(DISK_ADDRESS,DISK_AVAIL_LOW) = (_u32)disk->avail;
	GET(DISK_ADDRESS,DISK_AVAIL_HIGH) = 0;
	GET(DISK_ADDRESS,DISK_USED_LOW) = (_u32)disk->used;
	GET(DISK_ADDRESS,DISK_USED_HIGH) = 0;

	GET(DISK_ADDRESS,DISK_QUEUEREADY) = 1;
	status |= DRIVER_OK;
	GET(DISK_ADDRESS,DISK_STATUS) = status;
}

static int alloc_desc() {
	for (int i = 0; i < QUEUE_SIZE; ++i) {
		if (disk->free & (1 << i)) {
			disk->free &= ~(1 << i);
			return i;
		}
	}
	return -1;
}


static void free_desc(_u32 i) {
	disk->free |= (1 << i);

}

static int alloc_3desc(_u32* idx) {
	for (int i = 0; i < 3; ++i) {
		if ((idx[i] = alloc_desc()) == -1) {
			for (int j = 0; j < i; ++j) {
				free_desc(idx[j]);
			}
			return -1;
		}	
	}
	return 0;
}

static void free_chain(_u32* idx) {
	for (int i = 0; i < 3; ++i) {
		free_desc(idx[i]);
	}
}


static void disk_rw_sector(_u64 sector,int write,void*buf) {
	_u32 idx[3];
	if (alloc_3desc(idx) == -1) {
		panic("disk_rw alloc 3 idx failed!\n");
	}
	struct virtio_blk_req * buf0 = &disk->reqs[idx[0]];
	if (write) {
		buf0->type = VIRTIO_BLK_T_OUT;
	}
	else {
		buf0->type = VIRTIO_BLK_T_IN;
	}
	buf0->data = buf;
	buf0->reserved = 0;
	buf0->sector = sector;
	buf0->status = 0x03;

	disk->desc[idx[0]].addr = (_u32)buf0;
	disk->desc[idx[0]].len = 16;
	disk->desc[idx[0]].flags = VIRTQ_DESC_F_NEXT;
	disk->desc[idx[0]].next = idx[1];
	disk->desc[idx[1]].addr = (_u32)buf0->data;
	disk->desc[idx[1]].len = 512;
	if (write) {
		disk->desc[idx[1]].flags = 0;
	}
	else {
		disk->desc[idx[1]].flags = VIRTQ_DESC_F_WRITE;
	}
	disk->desc[idx[1]].flags |= VIRTQ_DESC_F_NEXT;
	disk->desc[idx[1]].next = idx[2];

	disk->desc[idx[2]].addr = (_u32)&buf0->status;
	disk->desc[idx[2]].len = 1;
	disk->desc[idx[2]].flags = VIRTQ_DESC_F_WRITE;
	disk->desc[idx[2]].next = 0;

	disk->avail->ring[disk->avail->idx % QUEUE_SIZE] = idx[0];
	__smp_wmb();
	disk->avail->idx += 1;
	disk->used->idx += 1;
	__smp_rmb();
	GET(DISK_ADDRESS,DISK_QUEUENOTIFY) = 0;

	while (1) {
//		printk("INTERRUPT_STATUS %08x\n",GET(DISK_ADDRESS,DISK_INTERRUPT_STATUS));
		GET(DISK_ADDRESS,DISK_INTERRUPT_ACK) = GET(DISK_ADDRESS,DISK_INTERRUPT_STATUS) & 0x03;
		if (buf0->status != 0x03) {
			break;
		}
	}

//	printk("status is ::::%d::::\n",buf0->status);
//	printk("::::%s::::\n",buf0->data);
//	printk("used elem id %08x\n",disk->used->ring[disk->used->idx - 1].id);
//	printk("used_elem %08x\n",disk->used->ring[disk->used->idx - 1].len);

	free_chain(idx);
}

void disk_rw(_u64 sector,int write,void*buf,int nsecs) {
	char tbuf[512];
	for (int i = 0; i < nsecs; ++i) {
		if (write) {
//			memcpy((buf + i * SECTOR_SIZE),tbuf,SECTOR_SIZE);
			for (int j = 0; j < SECTOR_SIZE;++j) {
				 tbuf[j] = ((char*)(buf + i * SECTOR_SIZE))[j];
			}
		}
		disk_rw_sector(sector + i,write,tbuf);
		if (write == 0) {
			char *t = (char*)(buf + i * SECTOR_SIZE);
			for (int j = 0; j < SECTOR_SIZE;++j) {
				t[j] = tbuf[j];
			}
			//			memcpy(t,tbuf,SECTOR_SIZE);
		}
	}

}








//

