//
// Created by Flager on 2022/4/21.
//

#ifndef BLFS_DISK_H
#define BLFS_DISK_H

#include <string>


#ifndef LINUX

#define __bitwise __attribute__((bitwise))

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

#endif //BLFS_DISK_H
