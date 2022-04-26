//
// Created by lighthouse on 4/26/22.
//

#include <linux/types.h>
#include "group_descriptor.h"
#include "superblock.h"
#include "inode.h"

GroupDescriptor::GroupDescriptor() {

}

GroupDescriptor::~GroupDescriptor() {

}

void GroupDescriptor::traverse_data_to_settings(void *buf) {
    __u8 *u8_buf = reinterpret_cast<__u8 *>(buf);
    auto u8_to_le32 = [&](__le32 &num, int offset) {
        num =
                (__le64) u8_buf[offset] |
                ((__le64) u8_buf[offset + 1] << 8) |
                ((__le64) u8_buf[offset + 2] << 16) |
                ((__le64) u8_buf[offset + 3] << 24);
    };
    auto u8_to_le16 = [&](__le16 &num, int offset) {
        num =
                (__le64) u8_buf[offset] |
                ((__le64) u8_buf[offset + 1] << 8);
    };
    u8_to_le32(bg_block_bitmap_lo, 0x0);
    u8_to_le32(bg_inode_bitmap_lo, 0x4);
    u8_to_le32(bg_inode_table_lo, 0x8);
    u8_to_le16(bg_free_blocks_count_lo, 0xC);
    u8_to_le16(bg_free_inodes_count_lo, 0xE);
    u8_to_le16(bg_used_dirs_count_lo, 0x10);
    u8_to_le16(bg_flags, 0x12);
    u8_to_le32(bg_exclude_bitmap_lo, 0x14);
    u8_to_le16(bg_block_bitmap_csum_lo, 0x18);
    u8_to_le16(bg_inode_bitmap_csum_lo, 0x1A);
    u8_to_le16(bg_itable_unused_lo, 0x1C);
    u8_to_le16(bg_checksum, 0x1E);
    u8_to_le32(bg_block_bitmap_hi, 0x20);
    u8_to_le32(bg_inode_bitmap_hi, 0x24);
    u8_to_le32(bg_inode_table_hi, 0x28);
    u8_to_le16(bg_free_blocks_count_hi, 0x2C);
    u8_to_le16(bg_free_inodes_count_hi, 0x2E);
    u8_to_le16(bg_used_dirs_count_hi, 0x30);
    u8_to_le16(bg_itable_unused_hi, 0x32);
    u8_to_le32(bg_exclude_bitmap_hi, 0x34);
    u8_to_le16(bg_block_bitmap_csum_hi, 0x38);
    u8_to_le16(bg_inode_bitmap_csum_hi, 0x3A);
    u8_to_le32(bg_reserved, 0x3C);
}

void GroupDescriptor::traverse_settings_to_data(void *buf) {
    __u8 *u8_buf = reinterpret_cast<__u8 *>(buf);
    auto le32_to_u8 = [&](__le32 num, int offset) {
        u8_buf[offset] = (__u8) (num & 0xFF);
        u8_buf[offset + 1] = (__u8) ((num & 0xFF00) >> 8);
        u8_buf[offset + 2] = (__u8) ((num & 0xFF0000) >> 16);
        u8_buf[offset + 3] = (__u8) ((num & 0xFF000000) >> 24);
    };
    auto le16_to_u8 = [&](__le16 num, int offset) {
        u8_buf[offset] = (__u8) (num & 0xFF);
        u8_buf[offset + 1] = (__u8) ((num & 0xFF00) >> 8);
    };
    le32_to_u8(bg_block_bitmap_lo, 0x0);
    le32_to_u8(bg_inode_bitmap_lo, 0x4);
    le32_to_u8(bg_inode_table_lo, 0x8);
    le16_to_u8(bg_free_blocks_count_lo, 0xC);
    le16_to_u8(bg_free_inodes_count_lo, 0xE);
    le16_to_u8(bg_used_dirs_count_lo, 0x10);
    le16_to_u8(bg_flags, 0x12);
    le32_to_u8(bg_exclude_bitmap_lo, 0x14);
    le16_to_u8(bg_block_bitmap_csum_lo, 0x18);
    le16_to_u8(bg_inode_bitmap_csum_lo, 0x1A);
    le16_to_u8(bg_itable_unused_lo, 0x1C);
    le16_to_u8(bg_checksum, 0x1E);
    le32_to_u8(bg_block_bitmap_hi, 0x20);
    le32_to_u8(bg_inode_bitmap_hi, 0x24);
    le32_to_u8(bg_inode_table_hi, 0x28);
    le16_to_u8(bg_free_blocks_count_hi, 0x2C);
    le16_to_u8(bg_free_inodes_count_hi, 0x2E);
    le16_to_u8(bg_used_dirs_count_hi, 0x30);
    le16_to_u8(bg_itable_unused_hi, 0x32);
    le32_to_u8(bg_exclude_bitmap_hi, 0x34);
    le16_to_u8(bg_block_bitmap_csum_hi, 0x38);
    le16_to_u8(bg_inode_bitmap_csum_hi, 0x3A);
    le32_to_u8(bg_reserved, 0x3C);
}

void GroupDescriptor::create_default_settings(int group_id) {
    Superblock *superblock_instance = Superblock::get_instance();
    int block_size = 2 << (10 + superblock_instance->s_log_block_size);
    unsigned long long group_size = block_size * superblock_instance->s_blocks_per_group;
    unsigned long long bg_block_bitmap = group_id == 0 ? 0x800 : group_id * group_size;
    unsigned long long bg_inode_bitmap =
            bg_block_bitmap + ((superblock_instance->s_blocks_per_group - 1) / block_size + 1) * block_size;
    unsigned long long bg_inode_table =
            bg_inode_bitmap + ((superblock_instance->s_inodes_per_group - 1) / block_size + 1) * block_size;
    unsigned int num_used_blocks = (bg_inode_table - group_id * group_size) / block_size +
                                   (Inode::INODE_SIZE * superblock_instance->s_inodes_per_group - 1) / block_size +
                                   1;
    unsigned int bg_free_blocks_count = superblock_instance->s_blocks_per_group - num_used_blocks;
    unsigned int bg_free_inodes_count = superblock_instance->s_inodes_per_group;
    unsigned int bg_used_dirs_count = 0;

    bg_block_bitmap_lo = (__le32) (bg_block_bitmap & 0xFFFF);
    bg_block_bitmap_hi = (__le32) ((bg_block_bitmap & 0xFFFF0000) >> 16);
    bg_inode_bitmap_lo = (__le32) (bg_inode_bitmap & 0xFFFF);
    bg_inode_bitmap_hi = (__le32) ((bg_inode_bitmap & 0xFFFF0000) >> 16);
    bg_inode_table_lo = (__le32) (bg_inode_table & 0xFFFF);
    bg_inode_table_hi = (__le32) ((bg_inode_table & 0xFFFF0000) >> 16);
    bg_free_blocks_count_lo = (__le16) (bg_free_blocks_count & 0xFF);
    bg_free_blocks_count_hi = (__le16) ((bg_free_blocks_count & 0xFF00) >> 8);
    bg_free_inodes_count_lo = (__le16) (bg_free_inodes_count & 0xFF);
    bg_free_inodes_count_hi = (__le16) ((bg_free_inodes_count & 0xFF00) >> 8);
    bg_used_dirs_count_lo = (__le16) (bg_used_dirs_count & 0xFF);
    bg_used_dirs_count_hi = (__le16) ((bg_used_dirs_count & 0xFF00) >> 8);

    // remaining checksum
}