//
// Created by Flager on 2022/4/21.
//

#ifndef BLFS_INODE_H
#define BLFS_INODE_H

#include "disk.h"


/*
 * i_mode value is a combination of the following flags
 */

#define S_IXOTH 0x1         // Others may execute
#define S_IWOTH 0x2         // Others may write
#define S_IROTH 0x3         // Others may read
#define S_IXGRP 0x8         // Group members may execute
#define S_IWGRP 0x10        // Group members may write
#define S_IRGRP 0x20        // Group members may read
#define S_IXUSR 0x40        // Owner may execute
#define S_IWUSR 0x80        // Owner may write
#define S_IRUSR 0x100       // Owner may read
#define S_ISVTX 0x200       // Sticky bit
#define S_ISGID 0x400       // Set GID
#define S_ISUID 0x800       // Set UID
// These are mutually-exclusive file types
#define S_IFIFO 0x1000      // FIFO
#define S_IFCHR 0x2000      // Character device
#define S_IFDIR 0x4000      // Directory
#define S_IFBLK 0x6000      // Block device
#define S_IFREG 0x8000      // Regular file
#define S_IFLNK 0xA000      // Symbolic link
#define S_IFSOCK 0xC000     // Socket

class byte_array {
public:
    explicit byte_array(int length);
    ~byte_array();
    bool set_bytes(unsigned char* new_bytes, size_t len);
    bool get_bytes(unsigned char* new_bytes, size_t len);
    const int length;
private:
    unsigned char* bytes;
};

class Inode {
public:
    Inode();
private:
    Descriptor16 i_mode{0x0, 0};
    Descriptor16 i_uid{0x2, 0};
    Descriptor32 i_size_lo{0x4, 0};
    Descriptor32 i_atime{0x8, 0};
    Descriptor32 i_ctime{0xC, 0};
    Descriptor32 i_mtime{0x10, 0};
    Descriptor32 i_dtime{0x14, 0};
    Descriptor16 i_gid{0x18, 0};
    Descriptor16 i_links_count{0x1A, 0};
    Descriptor32 i_blocks_lo{0x1C, 0};
    Descriptor32 i_flags{0x20, 0};
    Descriptor<byte_array> i_osd1{0x24, byte_array(4)};
    Descriptor<byte_array> i_block{0x28, byte_array(60)};
    Descriptor32 i_generation{0x64, 0};
    Descriptor32 i_file_acl_lo{0x68, 0};
    Descriptor32 i_size_high{0x6C, 0};
    Descriptor32 i_obso_faddr{0x70, 0};
    Descriptor<byte_array> i_osd2{0x74, byte_array(12)};
    Descriptor16 i_extra_isize{0x80, 0};
    Descriptor16 i_checksum_hi{0x82, 0};
    Descriptor32 i_ctime_extra{0x84, 0};
    Descriptor32 i_mtime_extra{0x88, 0};
    Descriptor32 i_atime_extra{0x8C, 0};
    Descriptor32 i_crtime{0x90, 0};
    Descriptor32 i_crtime_extra{0x94, 0};
    Descriptor32 i_version_hi{0x98, 0};
    Descriptor32 i_projid{0x9C, 0};
};

#endif //BLFS_INODE_H
