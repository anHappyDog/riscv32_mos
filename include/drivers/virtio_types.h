#ifndef __VIRTIO_TYPES_H_
#define __VIRTIO_TYPES_H_
#include<stdint.h>

//check is little endian
/*
#ifdef __CHECKER__
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif
#define __bitwise __bitwise__
*/
typedef uint8_t _u8;
typedef uint16_t _u16;
typedef uint32_t _u32;
typedef uint64_t _u64;
/*
typedef _u16 __bitwise _le16;
typedef _u16 __bitwise _be16;
typedef _u32 __bitwise _le32;
typedef _u32 __bitwise _be32;
typedef _u64 __bitwise _le64;
typedef _u64 __bitwise _be64;
*/




#endif
