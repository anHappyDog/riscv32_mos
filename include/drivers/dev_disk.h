#ifndef TESTMACHINE_DISK_H
#define TESTMACHINE_DISK_H

/*
 *  Definitions used by the "disk" device in GXemul.
 *
 *  This file is in the public domain.
 */
#define RESET 0
#define ACKNOWLEDGE 1
#define DRIVER 2
#define FAILED 128
#define FEATHERS_OK 8
#define DRIVER_OK 4
#define DRIVER_NEEDS_RESET 64

#define DEV_DISK_REGADDRESS 0x10008000
#define DEV_DISK_STATUS 0x70
#define DEV_DISK_MAGICVALUE 0x0
#define DEV_DISK_DEVICEID 0x8
#define DEV_DISK_DEVICEFEATHERS 0x10


#define DEV_ADDR(base,offset)  *(int*)(base + offset)

void disk_init(void);


#endif /*  TESTMACHINE_DISK_H  */
