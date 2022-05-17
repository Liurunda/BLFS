//
// Created by Flager on 2022/4/21.
//

#ifndef BLFS_INODE_H
#define BLFS_INODE_H

#include <linux/types.h>

/*
 * i_mode value is a combination of the following flags
 */

#ifndef S_IXOTH
#define S_IXOTH 0x1         // Others may execute
#endif
#ifndef S_IWOTH
#define S_IWOTH 0x2         // Others may write
#endif
#ifndef S_IROTH
#define S_IROTH 0x3         // Others may read
#endif
#ifndef S_IXGRP
#define S_IXGRP 0x8         // Group members may execute
#endif
#ifndef S_IWGRP
#define S_IWGRP 0x10        // Group members may write
#endif
#ifndef S_IRGRP
#define S_IRGRP 0x20        // Group members may read
#endif
#ifndef S_IXUSR
#define S_IXUSR 0x40        // Owner may execute
#endif
#ifndef S_IWUSR
#define S_IWUSR 0x80        // Owner may write
#endif
#ifndef S_IRUSR
#define S_IRUSR 0x100       // Owner may read
#endif
#ifndef S_ISVTX
#define S_ISVTX 0x200       // Sticky bit
#endif
#ifndef S_ISGID
#define S_ISGID 0x400       // Set GID
#endif
#ifndef S_ISUID
#define S_ISUID 0x800       // Set UID
#endif
// These are mutually-exclusive file types
#ifndef S_IFIFO
#define S_IFIFO 0x1000      // FIFO
#endif
#ifndef S_IFCHR
#define S_IFCHR 0x2000      // Character device
#endif
#ifndef S_IFDIR
#define S_IFDIR 0x4000      // Directory
#endif
#ifndef S_IFBLK
#define S_IFBLK 0x6000      // Block device
#endif
#ifndef S_IFREG
#define S_IFREG 0x8000      // Regular file
#endif
#ifndef S_IFLNK
#define S_IFLNK 0xA000      // Symbolic link
#endif
#ifndef S_IFSOCK
#define S_IFSOCK 0xC000     // Socket
#endif


#define EXT4_N_BLOCKS 15

// 目录长度
#define DIRECTORY_LENGTH 256

struct Directory{
    int inode_id;
    char name[DIRECTORY_LENGTH-4];
};


class Inode {
public:
    Inode();

    ~Inode();

    void traverse_data_to_settings(void *);

    void traverse_settings_to_data(void *);

    static const int INODE_SIZE = 0x100;

    __le16 i_mode;                          // File mode.
    __le16 i_uid;                           // Lower 16-bits of Owner UID.
    __le32 i_size_lo;                       // Lower 32-bits of size in bytes.
    __le32 i_atime;                         // Last access time, in seconds since the epoch. However, if the EA_INODE inode flag is set, this inode stores an extended attribute value and this field contains the checksum of the value.
    __le32 i_ctime;                         // Last inode change time, in seconds since the epoch. However, if the EA_INODE inode flag is set, this inode stores an extended attribute value and this field contains the lower 32 bits of the attribute value’s reference count.
    __le32 i_mtime;                         // Last data modification time, in seconds since the epoch. However, if the EA_INODE inode flag is set, this inode stores an extended attribute value and this field contains the number of the inode that owns the extended attribute.
    __le32 i_dtime;                         // Deletion Time, in seconds since the epoch.
    __le16 i_gid;                           // Lower 16-bits of GID.
    __le16 i_links_count;                   // Hard link count. Normally, ext4 does not permit an inode to have more than 65,000 hard links. This applies to files as well as directories, which means that there cannot be more than 64,998 subdirectories in a directory (each subdirectory’s ‘..’ entry counts as a hard link, as does the ‘.’ entry in the directory itself). With the DIR_NLINK feature enabled, ext4 supports more than 64,998 subdirectories by setting this field to 1 to indicate that the number of hard links is not known.
    __le32 i_blocks_lo;                     // Lower 32-bits of “block” count. If the huge_file feature flag is not set on the filesystem, the file consumes i_blocks_lo 512-byte blocks on disk. If huge_file is set and EXT4_HUGE_FILE_FL is NOT set in inode.i_flags, then the file consumes i_blocks_lo + (i_blocks_hi << 32) 512-byte blocks on disk. If huge_file is set and EXT4_HUGE_FILE_FL IS set in inode.i_flags, then this file consumes (i_blocks_lo + i_blocks_hi << 32) filesystem blocks on disk.
    __le32 i_flags;                         // Inode flags.
    struct {
        __le32 l_i_version;                 // Inode version. However, if the EA_INODE inode flag is set, this inode stores an extended attribute value and this field contains the upper 32 bits of the attribute value’s reference count.
    } i_osd1;
    __le32 i_block[EXT4_N_BLOCKS];          // Block map or extent tree. See the section “The Contents of inode.i_block”.
    __le32 i_generation;                    // File version (for NFS).
    __le32 i_file_acl_lo;                   // Lower 32-bits of extended attribute block. ACLs are of course one of many possible extended attributes; I think the name of this field is a result of the first use of extended attributes being for ACLs.
    __le32 i_size_high;                     // Upper 32-bits of file/directory size. In ext2/3 this field was named i_dir_acl, though it was usually set to zero and never used.
    __le32 i_obso_faddr;                    // (Obsolete) fragment address.
    struct {
        __le16 l_i_blocks_high;             // Upper 16-bits of the block count. Please see the note attached to i_blocks_lo.
        __le16 l_i_file_acl_high;           // Upper 16-bits of the extended attribute block (historically, the file ACL location). See the Extended Attributes section below.
        __le16 l_i_uid_high;                // Upper 16-bits of the Owner UID.
        __le16 l_i_gid_high;                // Upper 16-bits of the GID.
        __le16 l_i_checksum_lo;             // Lower 16-bits of the inode checksum.
        __le16 l_i_reserved;                // Unused.
    } i_osd2;
    __le16 i_extra_isize;                   // Size of this inode - 128. Alternately, the size of the extended inode fields beyond the original ext2 inode, including this field.
    __le16 i_checksum_hi;                   // Upper 16-bits of the inode checksum.
    __le32 i_ctime_extra;                   // Extra change time bits. This provides sub-second precision. See Inode Timestamps section.
    __le32 i_mtime_extra;                   // Extra modification time bits. This provides sub-second precision.
    __le32 i_atime_extra;                   // Extra access time bits. This provides sub-second precision.
    __le32 i_crtime;                        // File creation time, in seconds since the epoch.
    __le32 i_crtime_extra;                  // Extra file creation time bits. This provides sub-second precision.
    __le32 i_version_hi;                    // Upper 32-bits for version number.
    __le32 i_projid;                        // Project ID.
};

#endif //BLFS_INODE_H
