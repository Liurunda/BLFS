//
// Created by lighthouse on 4/26/22.
//

#include "inode.h"
#include "disk.h"
#include <stdlib.h>
#include <stdio.h>

Inode::Inode() {
    i_mode = 0;
    i_uid = 0;
    i_size_lo = 0;
    i_atime = 0;
    i_ctime = 0;
    i_mtime = 0;
    i_gid = 0;
    i_links_count = 0;
    i_blocks_lo = 0;
    i_flags = 0;
    i_osd1.l_i_version = 0;
    for (int i = 0; i < 15; i++) i_block[i] = 0;
    i_generation = 0;
    i_file_acl_lo = 0;
    i_size_high = 0;
    i_obso_faddr = 0;
    i_osd2.l_i_blocks_high = 0;
    i_osd2.l_i_file_acl_high = 0;
    i_osd2.l_i_uid_high = 0;
    i_osd2.l_i_gid_high = 0;
    i_osd2.l_i_checksum_lo = 0;
    i_osd2.l_i_reserved = 0;
    i_extra_isize = 0;
    i_checksum_hi = 0;
    i_ctime_extra = 0;
    i_mtime_extra = 0;
    i_crtime = 0;
    i_crtime_extra = 0;
    i_version_hi = 0;
    i_projid = 0;

    num_block_id_per_block = Disk::get_instance()->block_size / sizeof(__le32);
    block_threshold_1 = 12;
    block_threshold_2 = block_threshold_1 + num_block_id_per_block;
    block_threshold_3 = block_threshold_2 + num_block_id_per_block * num_block_id_per_block;
    block_threshold_4 = block_threshold_3 + num_block_id_per_block * num_block_id_per_block * num_block_id_per_block;
}

Inode::~Inode() {

}

void Inode::traverse_data_to_settings(void *buf) {
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
    u8_to_le16(i_mode, 0x0);
    u8_to_le16(i_uid, 0x2);
    u8_to_le32(i_size_lo, 0x4);
    u8_to_le32(i_atime, 0x8);
    u8_to_le32(i_ctime, 0xC);
    u8_to_le32(i_mtime, 0x10);
    u8_to_le16(i_gid, 0x18);
    u8_to_le16(i_links_count, 0x1A);
    u8_to_le32(i_blocks_lo, 0x1C);
    u8_to_le32(i_flags, 0x20);
    u8_to_le32(i_osd1.l_i_version, 0x24);
    for (int i = 0; i < 15; i++) u8_to_le32(i_block[i], 0x28 + 4 * i);
    u8_to_le32(i_generation, 0x64);
    u8_to_le32(i_file_acl_lo, 0x68);
    u8_to_le32(i_size_high, 0x6C);
    u8_to_le32(i_obso_faddr, 0x70);
    u8_to_le16(i_osd2.l_i_blocks_high, 0x74);
    u8_to_le16(i_osd2.l_i_file_acl_high, 0x76);
    u8_to_le16(i_osd2.l_i_uid_high, 0x78);
    u8_to_le16(i_osd2.l_i_gid_high, 0x7A);
    u8_to_le16(i_osd2.l_i_checksum_lo, 0x7C);
    u8_to_le16(i_osd2.l_i_reserved, 0x7E);
    u8_to_le16(i_extra_isize, 0x80);
    u8_to_le16(i_checksum_hi, 0x82);
    u8_to_le32(i_ctime_extra, 0x84);
    u8_to_le32(i_mtime_extra, 0x88);
    u8_to_le32(i_atime_extra, 0x8C);
    u8_to_le32(i_crtime, 0x90);
    u8_to_le32(i_crtime_extra, 0x94);
    u8_to_le32(i_version_hi, 0x98);
    u8_to_le32(i_projid, 0x9C);
}

void Inode::traverse_settings_to_data(void *buf) {
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
    le16_to_u8(i_mode, 0x0);
    le16_to_u8(i_uid, 0x2);
    le32_to_u8(i_size_lo, 0x4);
    le32_to_u8(i_atime, 0x8);
    le32_to_u8(i_ctime, 0xC);
    le32_to_u8(i_mtime, 0x10);
    le16_to_u8(i_gid, 0x18);
    le16_to_u8(i_links_count, 0x1A);
    le32_to_u8(i_blocks_lo, 0x1C);
    le32_to_u8(i_flags, 0x20);
    le32_to_u8(i_osd1.l_i_version, 0x24);
    for (int i = 0; i < 15; i++) le32_to_u8(i_block[i], 0x28 + 4 * i);
    le32_to_u8(i_generation, 0x64);
    le32_to_u8(i_file_acl_lo, 0x68);
    le32_to_u8(i_size_high, 0x6C);
    le32_to_u8(i_obso_faddr, 0x70);
    le16_to_u8(i_osd2.l_i_blocks_high, 0x74);
    le16_to_u8(i_osd2.l_i_file_acl_high, 0x76);
    le16_to_u8(i_osd2.l_i_uid_high, 0x78);
    le16_to_u8(i_osd2.l_i_gid_high, 0x7A);
    le16_to_u8(i_osd2.l_i_checksum_lo, 0x7C);
    le16_to_u8(i_osd2.l_i_reserved, 0x7E);
    le16_to_u8(i_extra_isize, 0x80);
    le16_to_u8(i_checksum_hi, 0x82);
    le32_to_u8(i_ctime_extra, 0x84);
    le32_to_u8(i_mtime_extra, 0x88);
    le32_to_u8(i_atime_extra, 0x8C);
    le32_to_u8(i_crtime, 0x90);
    le32_to_u8(i_crtime_extra, 0x94);
    le32_to_u8(i_version_hi, 0x98);
    le32_to_u8(i_projid, 0x9C);
}

int Inode::get_kth_block_id(int k) {
    if (k <= block_threshold_1) {
        return i_block[k] == 0 ? -1 : i_block[k];
    };
    if (k <= block_threshold_2) {
        Disk *disk = Disk::get_instance();
        __le32 *block_1 = (__le32 *) malloc(disk->block_size);
        disk->read_from_block(i_block[12], block_1);
        int id = block_1[k - block_threshold_1];
        free(block_1);
        return id == 0 ? -1 : id;
    }
    if (k <= block_threshold_3) {
        Disk *disk = Disk::get_instance();
        __le32 *block_1 = (__le32 *) malloc(disk->block_size);
        __le32 *block_2 = (__le32 *) malloc(disk->block_size);
        int offset1 = (k - block_threshold_2) / num_block_id_per_block;
        int offset2 = (k - block_threshold_2) % num_block_id_per_block;
        disk->read_from_block(i_block[13], block_1);
        disk->read_from_block(block_1[offset1], block_2);
        int id = block_2[offset2];
        free(block_1);
        free(block_2);
        return id == 0 ? -1 : id;
    }
    if (k <= block_threshold_4) {
        Disk *disk = Disk::get_instance();
        __le32 *block_1 = (__le32 *) malloc(disk->block_size);
        __le32 *block_2 = (__le32 *) malloc(disk->block_size);
        __le32 *block_3 = (__le32 *) malloc(disk->block_size);
        int offset1 = (k - block_threshold_3) / num_block_id_per_block / num_block_id_per_block;
        int offset2 = (k - block_threshold_3) / num_block_id_per_block % num_block_id_per_block;
        int offset3 = (k - block_threshold_3) % num_block_id_per_block;
        disk->read_from_block(i_block[14], block_1);
        disk->read_from_block(block_1[offset1], block_2);
        disk->read_from_block(block_2[offset2], block_3);
        int id = block_3[offset3];
        free(block_1);
        free(block_2);
        free(block_3);
        return id == 0 ? -1 : id;
    }
    return -1; // block id too big
}

void Inode::add_block(int block_id) {
    for (int i = 0; i < 12; i++)
        if (i_block[i] == 0) {
            i_block[i] = block_id;
            return;
        }
    puts("Inode add block not implemented");
}