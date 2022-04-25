//
// Created by lighthouse on 4/23/22.
//

#include "superblock.h"
#include "crc32c/crc32c.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>


Superblock *Superblock::superblock_instance = nullptr;

Superblock::Superblock() {
    srand(time(NULL));
    // basic settings
    s_first_data_block = 1;
    s_log_block_size = 4;               // 4KiB
    s_log_cluster_size = 4;             // bigalloc disabled
    s_blocks_per_group = 32768;
    s_clusters_per_group = 32768;
    s_inodes_per_group = 8128;

    // generate UUID
    for(int i = 0; i < 8; i ++) s_uuid[i] = (__u8)(rand() % 255);

    s_magic = 0xEF53;

    s_checksum_type = 1;                // the only valid checksum algorithm crc32
    s_checksum_seed = crc32c::Crc32c(s_uuid, ~0);

    // set checksum
    s_checksum = 0;
    char* buf = new char[Superblock::SUPERBLOCK_SIZE];
    traverse_settings_to_data(buf);
    buf[Superblock::SUPERBLOCK_SIZE - 4] = '\0';
    s_checksum = crc32c::Crc32c(buf, ~0);
    delete[] buf;
}

void Superblock::traverse_settings_to_data(void *buf) {
    __u8* u8_buf = reinterpret_cast<__u8*>(buf);
    memset(u8_buf, 0, sizeof(__u8) * 0x400);
    auto le64_to_u8 = [&](__le64 num, int offset) {
        u8_buf[offset] = (__u8)(num & 0xFF);
        u8_buf[offset + 1] = (__u8)((num & 0xFF00) >> 8);
        u8_buf[offset + 2] = (__u8)((num & 0xFF0000) >> 16);
        u8_buf[offset + 3] = (__u8)((num & 0xFF000000) >> 24);
        u8_buf[offset + 4] = (__u8)((num & 0xFF00000000) >> 32);
        u8_buf[offset + 5] = (__u8)((num & 0xFF0000000000) >> 40);
        u8_buf[offset + 6] = (__u8)((num & 0xFF000000000000) >> 48);
        u8_buf[offset + 7] = (__u8)((num & 0xFF00000000000000) >> 56);
    };
    auto le32_to_u8 = [&](__le32 num, int offset) {
        u8_buf[offset] = (__u8)(num & 0xFF);
        u8_buf[offset + 1] = (__u8)((num & 0xFF00) >> 8);
        u8_buf[offset + 2] = (__u8)((num & 0xFF0000) >> 16);
        u8_buf[offset + 3] = (__u8)((num & 0xFF000000) >> 24);
    };
    auto le16_to_u8 = [&](__le16 num, int offset) {
        u8_buf[offset] = (__u8)(num & 0xFF);
        u8_buf[offset + 1] = (__u8)((num & 0xFF00) >> 8);
    };
    le32_to_u8(s_inodes_count, 0x0);
    le32_to_u8(s_blocks_count_lo, 0x4);
    le32_to_u8(s_r_blocks_count_lo, 0x8);
    le32_to_u8(s_free_blocks_count_lo, 0xC);
    le32_to_u8(s_free_inodes_count, 0x10);
    le32_to_u8(s_first_data_block, 0x14);
    le32_to_u8(s_log_block_size, 0x18);
    le32_to_u8(s_log_cluster_size, 0x1C);
    le32_to_u8(s_blocks_per_group, 0x20);
    le32_to_u8(s_clusters_per_group, 0x24);
    le32_to_u8(s_inodes_per_group, 0x28);
    le32_to_u8(s_mtime, 0x2C);
    le32_to_u8(s_wtime, 0x30);
    le16_to_u8(s_mnt_count, 0x34);
    le16_to_u8(s_max_mnt_count, 0x36);
    le16_to_u8(s_magic, 0x38);
    le16_to_u8(s_state, 0x3A);
    le16_to_u8(s_errors, 0x3C);
    le16_to_u8(s_minor_rev_level, 0x3E);
    le32_to_u8(s_lastcheck, 0x40);
    le32_to_u8(s_checkinterval, 0x44);
    le32_to_u8(s_creator_os, 0x48);
    le32_to_u8(s_rev_level, 0x4C);
    le16_to_u8(s_def_resuid, 0x50);
    le16_to_u8(s_def_resgid, 0x52);
    le32_to_u8(s_first_ino, 0x54);
    le16_to_u8(s_inode_size, 0x58);
    le16_to_u8(s_block_group_nr, 0x5A);
    le32_to_u8(s_feature_compat, 0x5C);
    le32_to_u8(s_feature_incompat, 0x60);
    le32_to_u8(s_feature_ro_compat, 0x64);
    for(int i = 0; i < 16; i ++) u8_buf[0x68 + i] = s_uuid[i];
    for(int i = 0; i < 16; i ++) u8_buf[0x78 + i] = (__u8)s_volume_name[i];
    for(int i = 0; i < 64; i ++) u8_buf[0x88 + i] = (__u8)s_last_mounted[i];
    le32_to_u8(s_algorithm_usage_bitmap, 0xC8);
    u8_buf[0xCC] = s_prealloc_blocks;
    u8_buf[0xCD] = s_prealloc_dir_blocks;
    le16_to_u8(s_reserved_gdt_blocks, 0xCE);
    for(int i = 0; i < 16; i ++) u8_buf[0xD0 + i] = s_journal_uuid[i];
    le32_to_u8(s_journal_inum, 0xE0);
    le32_to_u8(s_journal_dev, 0xE4);
    le32_to_u8(s_last_orphan, 0xE8);
    for(int i = 0; i < 4; i ++) le32_to_u8(s_hash_seed[i], 0xEC + 4 * i);
    u8_buf[0xFC] = s_def_hash_version;
    u8_buf[0xFD] = s_jnl_backup_type;
    le16_to_u8(s_desc_size, 0xFE);
    le32_to_u8(s_default_mount_opts, 0x100);
    le32_to_u8(s_first_meta_bg, 0x104);
    le32_to_u8(s_mkfs_time, 0x108);
    for(int i = 0; i < 17; i ++) le32_to_u8(s_jnl_blocks[i], 0x10C + 4 * i);
    le32_to_u8(s_blocks_count_hi, 0x150);
    le32_to_u8(s_r_blocks_count_hi, 0x154);
    le32_to_u8(s_free_blocks_count_hi, 0x158);
    le16_to_u8(s_min_extra_isize, 0x15C);
    le16_to_u8(s_want_extra_isize, 0x15E);
    le32_to_u8(s_flags, 0x160);
    le16_to_u8(s_raid_stride, 0x164);
    le64_to_u8(s_mmp_interval, 0x166);
    le64_to_u8(s_mmp_block, 0x168);
    le32_to_u8(s_raid_stripe_width, 0x170);
    u8_buf[0x174] = s_log_groups_per_flex;
    u8_buf[0x175] = s_checksum_type;
    le16_to_u8(s_reserved_pad, 0x176);
    le64_to_u8(s_kbytes_written, 0x178);
    le32_to_u8(s_snapshot_inum, 0x180);
    le32_to_u8(s_snapshot_id, 0x184);
    le64_to_u8(s_snapshot_r_blocks_count, 0x188);
    le32_to_u8(s_snapshot_list, 0x190);
    le32_to_u8(s_error_count, 0x194);
    le32_to_u8(s_first_error_time, 0x198);
    le32_to_u8(s_first_error_ino, 0x19C);
    le64_to_u8(s_first_error_block, 0x1A0);
    for(int i = 0; i < 32; i ++) u8_buf[0x1A8 + i] = s_first_error_func[i];
    le32_to_u8(s_first_error_line, 0x1C8);
    le32_to_u8(s_last_error_line, 0x1CC);
    le32_to_u8(s_last_error_ino, 0x1D0);
    le32_to_u8(s_last_error_line, 0x1D4);
    le64_to_u8(s_last_error_block, 0x1D8);
    for(int i = 0; i < 32; i ++) u8_buf[0x1E0 + i] = s_last_error_func[i];
    for(int i = 0; i < 64; i ++) u8_buf[0x200 + i] = s_mount_opts[i];
    le32_to_u8(s_usr_quota_inum, 0x240);
    le32_to_u8(s_grp_quota_inum, 0x244);
    le32_to_u8(s_overhead_blocks, 0x248);
    le32_to_u8(s_backup_bgs[0], 0x24C);
    le32_to_u8(s_backup_bgs[1], 0x250);
    for(int i = 0; i < 4; i ++) u8_buf[0x254 + i] = s_encrypt_algos[i];
    for(int i = 0; i < 16; i ++) u8_buf[0x258 + i] = s_encrypt_pw_salt[i];
    le32_to_u8(s_lpf_ino, 0x268);
    le32_to_u8(s_prj_quota_inum, 0x26C);
    le32_to_u8(s_checksum_seed, 0x270);
    u8_buf[0x274] = s_wtime_hi;
    u8_buf[0x275] = s_mtime_hi;
    u8_buf[0x276] = s_mkfs_time_hi;
    u8_buf[0x277] = s_lastcheck_hi;
    u8_buf[0x278] = s_first_error_time_hi;
    u8_buf[0x279] = s_last_error_time_hi;
    u8_buf[0x27A] = s_pad[0];
    u8_buf[0x27B] = s_pad[1];
    le16_to_u8(s_encoding, 0x27C);
    le16_to_u8(s_encoding_flags, 0x27E);
    le32_to_u8(s_orphan_file_inum, 0x280);
    for(int i = 0; i < 94; i ++) le32_to_u8(s_reserved[i], 0x284 + 4 * i);
    le32_to_u8(s_checksum, 0x3FC);
}

void Superblock::traverse_data_to_settings(void *buf) {
    __u8* u8_buf = reinterpret_cast<__u8*>(buf);
    memset(u8_buf, 0, sizeof(__u8) * 0x400);
    auto u8_to_le64 = [&](__le64& num, int offset) {
        num =
                (__le64)u8_buf[offset] |
                ((__le64)u8_buf[offset + 1] << 8) |
                ((__le64)u8_buf[offset + 2] << 16) |
                ((__le64)u8_buf[offset + 3] << 24) |
                ((__le64)u8_buf[offset + 4] << 32) |
                ((__le64)u8_buf[offset + 5] << 40) |
                ((__le64)u8_buf[offset + 6] << 48) |
                ((__le64)u8_buf[offset + 7] << 56);
    };
    auto u8_to_le32 = [&](__le32& num, int offset) {
        num =
                (__le64)u8_buf[offset] |
                ((__le64)u8_buf[offset + 1] << 8) |
                ((__le64)u8_buf[offset + 2] << 16) |
                ((__le64)u8_buf[offset + 3] << 24);
    };
    auto u8_to_le16 = [&](__le16& num, int offset) {
        num =
                (__le64)u8_buf[offset] |
                ((__le64)u8_buf[offset + 1] << 8);
    };
    u8_to_le32(s_inodes_count, 0x0);
    u8_to_le32(s_blocks_count_lo, 0x4);
    u8_to_le32(s_r_blocks_count_lo, 0x8);
    u8_to_le32(s_free_blocks_count_lo, 0xC);
    u8_to_le32(s_free_inodes_count, 0x10);
    u8_to_le32(s_first_data_block, 0x14);
    u8_to_le32(s_log_block_size, 0x18);
    u8_to_le32(s_log_cluster_size, 0x1C);
    u8_to_le32(s_blocks_per_group, 0x20);
    u8_to_le32(s_clusters_per_group, 0x24);
    u8_to_le32(s_inodes_per_group, 0x28);
    u8_to_le32(s_mtime, 0x2C);
    u8_to_le32(s_wtime, 0x30);
    u8_to_le16(s_mnt_count, 0x34);
    u8_to_le16(s_max_mnt_count, 0x36);
    u8_to_le16(s_magic, 0x38);
    u8_to_le16(s_state, 0x3A);
    u8_to_le16(s_errors, 0x3C);
    u8_to_le16(s_minor_rev_level, 0x3E);
    u8_to_le32(s_lastcheck, 0x40);
    u8_to_le32(s_checkinterval, 0x44);
    u8_to_le32(s_creator_os, 0x48);
    u8_to_le32(s_rev_level, 0x4C);
    u8_to_le16(s_def_resuid, 0x50);
    u8_to_le16(s_def_resgid, 0x52);
    u8_to_le32(s_first_ino, 0x54);
    u8_to_le16(s_inode_size, 0x58);
    u8_to_le16(s_block_group_nr, 0x5A);
    u8_to_le32(s_feature_compat, 0x5C);
    u8_to_le32(s_feature_incompat, 0x60);
    u8_to_le32(s_feature_ro_compat, 0x64);
    for(int i = 0; i < 16; i ++) s_uuid[i] = u8_buf[0x68 + i];
    for(int i = 0; i < 16; i ++) s_volume_name[i] = (char)u8_buf[0x78 + i];
    for(int i = 0; i < 64; i ++) s_last_mounted[i] = (char)u8_buf[0x88 + i];
    u8_to_le32(s_algorithm_usage_bitmap, 0xC8);
    s_prealloc_blocks = u8_buf[0xCC];
    s_prealloc_dir_blocks = u8_buf[0xCD];
    u8_to_le16(s_reserved_gdt_blocks, 0xCE);
    for(int i = 0; i < 16; i ++) s_journal_uuid[i] = u8_buf[0xD0 + i];
    u8_to_le32(s_journal_inum, 0xE0);
    u8_to_le32(s_journal_dev, 0xE4);
    u8_to_le32(s_last_orphan, 0xE8);
    for(int i = 0; i < 4; i ++) u8_to_le32(s_hash_seed[i], 0xEC + 4 * i);
    s_def_hash_version = u8_buf[0xFC];
    s_jnl_backup_type = u8_buf[0xFD];
    u8_to_le16(s_desc_size, 0xFE);
    u8_to_le32(s_default_mount_opts, 0x100);
    u8_to_le32(s_first_meta_bg, 0x104);
    u8_to_le32(s_mkfs_time, 0x108);
    for(int i = 0; i < 17; i ++) u8_to_le32(s_jnl_blocks[i], 0x10C + 4 * i);
    u8_to_le32(s_blocks_count_hi, 0x150);
    u8_to_le32(s_r_blocks_count_hi, 0x154);
    u8_to_le32(s_free_blocks_count_hi, 0x158);
    u8_to_le16(s_min_extra_isize, 0x15C);
    u8_to_le16(s_want_extra_isize, 0x15E);
    u8_to_le32(s_flags, 0x160);
    u8_to_le16(s_raid_stride, 0x164);
    u8_to_le64(s_mmp_interval, 0x166);
    u8_to_le64(s_mmp_block, 0x168);
    u8_to_le32(s_raid_stripe_width, 0x170);
    s_log_groups_per_flex = u8_buf[0x174];
    s_checksum_type = u8_buf[0x175];
    u8_to_le16(s_reserved_pad, 0x176);
    u8_to_le64(s_kbytes_written, 0x178);
    u8_to_le32(s_snapshot_inum, 0x180);
    u8_to_le32(s_snapshot_id, 0x184);
    u8_to_le64(s_snapshot_r_blocks_count, 0x188);
    u8_to_le32(s_snapshot_list, 0x190);
    u8_to_le32(s_error_count, 0x194);
    u8_to_le32(s_first_error_time, 0x198);
    u8_to_le32(s_first_error_ino, 0x19C);
    u8_to_le64(s_first_error_block, 0x1A0);
    for(int i = 0; i < 32; i ++) u8_buf[0x1A8 + i] = s_first_error_func[i];
    u8_to_le32(s_first_error_line, 0x1C8);
    u8_to_le32(s_last_error_line, 0x1CC);
    u8_to_le32(s_last_error_ino, 0x1D0);
    u8_to_le32(s_last_error_line, 0x1D4);
    u8_to_le64(s_last_error_block, 0x1D8);
    for(int i = 0; i < 32; i ++) s_last_error_func[i] = u8_buf[0x1E0 + i];
    for(int i = 0; i < 64; i ++) s_mount_opts[i] = u8_buf[0x200 + i];
    u8_to_le32(s_usr_quota_inum, 0x240);
    u8_to_le32(s_grp_quota_inum, 0x244);
    u8_to_le32(s_overhead_blocks, 0x248);
    u8_to_le32(s_backup_bgs[0], 0x24C);
    u8_to_le32(s_backup_bgs[1], 0x250);
    for(int i = 0; i < 4; i ++) s_encrypt_algos[i] = u8_buf[0x254 + i];
    for(int i = 0; i < 16; i ++) s_encrypt_pw_salt[i] = u8_buf[0x258 + i];
    u8_to_le32(s_lpf_ino, 0x268);
    u8_to_le32(s_prj_quota_inum, 0x26C);
    u8_to_le32(s_checksum_seed, 0x270);
    s_wtime_hi = u8_buf[0x274];
    s_mtime_hi = u8_buf[0x275];
    s_mkfs_time_hi = u8_buf[0x276];
    s_lastcheck_hi = u8_buf[0x277];
    s_first_error_time_hi = u8_buf[0x278];
    s_last_error_time_hi = u8_buf[0x279];
    s_pad[0] = u8_buf[0x27A];
    s_pad[1] = u8_buf[0x27B];
    u8_to_le16(s_encoding, 0x27C);
    u8_to_le16(s_encoding_flags, 0x27E);
    u8_to_le32(s_orphan_file_inum, 0x280);
    for(int i = 0; i < 94; i ++) u8_to_le32(s_reserved[i], 0x284 + 4 * i);
    u8_to_le32(s_checksum, 0x3FC);
}

bool Superblock::validate_checksum() {
    char* buf = new char[Superblock::SUPERBLOCK_SIZE];
    traverse_settings_to_data(buf);
    buf[Superblock::SUPERBLOCK_SIZE - 4] = '\0';
    __le32 checksum = crc32c::Crc32c(buf, ~0);
    delete[] buf;
    return checksum == s_checksum;
}