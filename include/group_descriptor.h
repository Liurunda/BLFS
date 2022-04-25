//
// Created by lighthouse on 4/25/22.
//

#ifndef BLFS_GROUP_DESCRIPTOR_H
#define BLFS_GROUP_DESCRIPTOR_H

class GroupDescriptor {
public:
    GroupDescriptor();
    ~GroupDescriptor();

    __le32 bg_block_bitmap_lo;              // Lower 32-bits of location of block bitmap.
    __le32 bg_inode_bitmap_lo;              // Lower 32-bits of location of inode bitmap.
    __le32 bg_inode_table_lo;               // Lower 32-bits of location of inode table.
    __le16 bg_free_blocks_count_lo;         // Lower 16-bits of free block count.
    __le16 bg_free_inodes_count_lo;         // Lower 16-bits of free inode count.
    __le16 bg_used_dirs_count_lo;           // Lower 16-bits of directory count.
    __le16 bg_flags;                        // Block group flags.
    __le32 bg_exclude_bitmap_lo;            // Lower 32-bits of location of snapshot exclusion bitmap.
    __le16 bg_block_bitmap_csum_lo;         // Lower 16-bits of the block bitmap checksum.
    __le16 bg_inode_bitmap_csum_lo;         // Lower 16-bits of the inode bitmap checksum.
    __le16 bg_itable_unused_lo;             // Lower 16-bits of unused inode count. If set, we needn’t scan past the (sb.s_inodes_per_group - gdt.bg_itable_unused)th entry in the inode table for this group.
    __le16 bg_checksum;                     // Group descriptor checksum; crc16(sb_uuid+group_num+bg_desc) if the RO_COMPAT_GDT_CSUM feature is set, or crc32c(sb_uuid+group_num+bg_desc) & 0xFFFF if the RO_COMPAT_METADATA_CSUM feature is set. The bg_checksum field in bg_desc is skipped when calculating crc16 checksum, and set to zero if crc32c checksum is used.
    // These fields only exist if the 64bit feature is enabled and s_desc_size > 32.
    __le32 bg_block_bitmap_hi;              // Upper 32-bits of location of block bitmap.
    __le32 bg_inode_bitmap_hi;              // Upper 32-bits of location of inodes bitmap.
    __le32 bg_inode_table_hi;               // Upper 32-bits of location of inodes table.
    __le16 bg_free_blocks_count_hi;         // Upper 16-bits of free block count.
    __le16 bg_free_inodes_count_hi;         // Upper 16-bits of free inode count.
    __le16 bg_used_dirs_count_hi;           // Upper 16-bits of directory count.
    __le16 bg_itable_unused_hi;             // Upper 16-bits of unused inode count.
    __le32 bg_exclude_bitmap_hi;            // Upper 32-bits of location of snapshot exclusion bitmap.
    __le16 bg_block_bitmap_csum_hi;         // Upper 16-bits of the block bitmap checksum.
    __le16 bg_inode_bitmap_csum_hi;         // Upper 16-bits of the inode bitmap checksum.
    __u32 bg_reserved;                      // Padding to 64 bytes.
};

#endif //BLFS_GROUP_DESCRIPTOR_H
