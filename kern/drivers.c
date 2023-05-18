#include <drivers/virtio_blk.h>
#include <drivers/virtio.h>
#include <printk.h>
#include <pmap.h>

struct virtq_desc* vdescTable;
struct virtq_used* virtqUsed;
struct virtq_avail* virtqAvail;

struct virtio_blk disk;
//u_int descUsed = 0;

static void disk_blk_init(struct virtq_desc* des,struct virtq_used* use,struct virtq_avail* avail) {
	disk.avail = avail;
	disk.used = use;
	disk.desc = des;
	for (int i = 0; i < QUEUE_SIZE / 8; ++i) {
		disk.free[i]  = 0xff;
	}	
}


void disk_init(void) {
	struct Page* p;
	if (DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_DEVICEID) != 2 || 
		DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_MAGICVALUE) != 0x74726976 ||
		DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_VERSION) != 2 || 
		DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_VENDORID) != 0x554d4551) {
		panic("disk init read pre info failed !\n");
	}
	_u32 status = 0;
	DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_STATUS) = status;
	status |= ACKNOWLEDGE;
	DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_STATUS) = status;
	status |= DRIVER;
	DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_STATUS) = status;

	u_int feathers = DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_DEVICEFEATHERS);
	feathers &= ~(1 << VIRTIO_BLK_F_RO);
	feathers &= ~(1 << VIRTIO_BLK_F_SCSI);
	feathers &= ~(1 << VIRTIO_BLK_F_CONFIG_WCE);
	feathers &= ~(1 << VIRTIO_F_ANY_LAYOUT);
	feathers &= ~(1 << VIRTIO_F_EVENT_IDX);
	feathers &= ~(1 << VIRTIO_F_INDIRECT_DESC);	
	DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_DRIVERFEATHERS) = feathers;
	status |= FEATHERS_OK;
	DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_STATUS) = status;
	if(!(DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_STATUS) & FEATHERS_OK)) {
		panic("device is not feathers ok!\n");
	}
	DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_QUEUE_SEL) = 0;
	if (DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_QUEUEREADY)) {
		panic("virtio disk shouldn't be ready before set!\n");
	}
	_u32 maxqs = DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_QUEUENUM_MAX);
	if (maxqs == 0) {
		panic("virtio disk's max queue size is 0!\n");
	}
	if (maxqs < QUEUE_SIZE) {
		panic("virtio disk is too small to hold the QUEUE SIZE!\n");
	}
	if (page_alloc_sequent(&p,4) != 0) {
		panic("not enough sequent memory for desctable!\n");
	}
	vdescTable = (struct virtq_desc*)page2addr(p);
	if (page_alloc(&p) != 0) {
		panic("not enought memory for virtavail!\n");
	}
	p->pp_ref += 1;
	virtqAvail = (struct virtq_avail*)page2addr(p);
	if (page_alloc(&p) != 0) {
		panic("not enough memory for virtused!\n");
	}
	p->pp_ref += 1;
	virtqUsed = (struct virtq_used*)page2addr(p);
	disk_blk_init(vdescTable,virtqUsed,virtqAvail);
	DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_QUEUEDESC_HIGH) = 0;
	DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_QUEUEDESC_LOW) = (_u32)vdescTable;
	DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_QUEUEAVAIL_HIGH) = 0;
	DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_QUEUEAVAIL_LOW) = (_u32)virtqAvail;
	DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_QUEUEUSED_HIGH) = 0;
	DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_QUEUEUSED_LOW) = (_u32)virtqUsed;
	
	DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_QUEUENUM) = QUEUE_SIZE;
	
	DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_QUEUEREADY) = 1;
	status |= DRIVER_OK;
	DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_STATUS) = status;
}
static int is_desc_free(int i) {
	if (i >= QUEUE_SIZE || i < 0) {
		panic("free desc i is illegal!\n");
	}	
	return (disk.free[i / 8] & (1 << (i & 0x7))) == 1;
}

static void free_desc(_u32 i) {
	if (is_desc_free(i)) {
		panic("desc is already free before being freed!\n");
	}
	disk.desc[i].addr = 0;
	disk.desc[i].len = 0;
	disk.desc[i].flags = 0;
	disk.desc[i].next = 0;
	disk.free[i >> 3] |= (1 << (i & 0x7));
}

static int alloc_desc(void) {
	for (int i = 0; i < QUEUE_SIZE; ++i) {
		if (disk.free[i >> 3] & (1 << (i & 0x7))) {
			disk.free[i >> 3] &= ~(1 << (i & 0x7));
			return i;
		}
	}
	return -1;
}	

static void free_chain(_u32 i) {
	_u32 flag;
	_u32 next;
	while (1) {
		flag = disk.desc[i].flags;
		next = disk.desc[i].next;
		free_desc(i);
		if (flag & VIRTQ_DESC_F_NEXT) {
			i = next;
		}
		else {
			break;
		}
	}
}

static int alloc3_desc(int *idx) {
	for (int i = 0; i < 3; ++i) {
		idx[i] = alloc_desc();
		if (idx[i] < 0) {
			for (int j = 0; j < i; ++j) {
				free_desc(idx[j]);
				return -1;
			}
		}
	}
	return 0;
}

void disk_rw(_u32 sector, _u32 write,void* adhr) {
	printk(":::%s:::\n",(char*)adhr);
	_u32 status = 0;
	int idx[3];
	if (alloc3_desc(idx) != 0) {
		panic("disk alloc idx failed!\n");
	}
	struct virtq_blk_req* buf0 = &disk.req[idx[0]];
	if (write) {
		buf0->type = VIRTIO_BLK_T_OUT;
	}
	else {
		buf0->type = VIRTIO_BLK_T_IN;
	}
	//buf0->data = (_u8*)adhr;
	buf0->reserved = 0;
	buf0->sector = sector;
	disk.desc[idx[0]].addr = (_u64)buf0;
	disk.desc[idx[0]].len = sizeof(struct virtq_blk_req);
	disk.desc[idx[0]].flags = VIRTQ_DESC_F_NEXT;
	disk.desc[idx[0]].next = idx[1];
	/*	if (write) {
	for (int i = 0; i < 100; ++i) {
		buf0->data[i] = 0xff;
	}
	}
	else {
		for (int i = 0; i < 100; ++i) {
			buf0->data[i] = 0x0;
		}
	}*/
	disk.desc[idx[1]].addr = (_u64)adhr;

	disk.desc[idx[1]].len = SEC_SIZE;
	if (write) {
		disk.desc[idx[1]].flags = 0;
	}
	else {
		disk.desc[idx[1]].flags = VIRTQ_DESC_F_WRITE;
	}
	disk.desc[idx[1]].flags |= VIRTQ_DESC_F_NEXT;
	disk.desc[idx[1]].next = idx[2];
	status = 0x3;
	//buf0->status = 0xff;
	//disk.desc[idx[2]].addr = (_u32)&buf0->status;
	disk.desc[idx[2]].addr = (_u64)&status;
	disk.desc[idx[2]].len = 1;
	disk.desc[idx[2]].flags = VIRTQ_DESC_F_WRITE & ~VIRTQ_DESC_F_NEXT;
	disk.avail->ring[disk.avail->idx % QUEUE_SIZE] = idx[0];
	disk.desc[idx[2]].next = 0;
	virtio_wmb();
	disk.avail->idx += 1;
	virtio_rmb();
	
	DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_QUEUENOTIFY) = 0;
	
	//while (buf0->status == 1);
	while (status != 0x3);
	printk("the result status is %08x\n",status);
	for (int i = 1 ;i <= 512; ++i) {
		char* t = (char*)adhr + i;
		printk("%02x ",*t);
		if (i % 16 == 0 && i != 0) {
			printk("\n");
		}
	}
	free_chain(idx[0]);
	printk("interrupt is %08x\n",DEV_ADDR(DEV_DISK_REGADDRESS,DEV_DISK_INTERRUPTSTATUS));
	//FREE;
}



void disk_test() {
	_u32 t = 0;
	for (int i = 0; i < 10; ++i) {
		t = DEV_ADDR(DEV_DISK_REGADDRESS,0x100 + i);
		printk("%2d :::%08x\n",i,t);	
	}
}















