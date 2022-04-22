//
// Created by Flager on 2022/4/21.
//

#ifndef BLFS_DISK_H
#define BLFS_DISK_H

#ifndef LINUX

#ifdef __CHECKER__
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif
#define __bitwise __bitwise__

typedef         uint8_t         __u8;
typedef         uint16_t        __u16;
typedef         uint32_t        __u32;

#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
typedef         uint64_t        __u64;
#endif

typedef __u16 __bitwise __le16;
typedef __u16 __bitwise __be16;
typedef __u32 __bitwise __le32;
typedef __u32 __bitwise __be32;

#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
typedef __u64 __bitwise __le64;
typedef __u64 __bitwise __be64;
#endif
#else
#include "linux/types.h"
#endif

const char DISK_PATH[] = "/tmp/disk";

#endif //BLFS_DISK_H
