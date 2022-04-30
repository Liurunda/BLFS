//
// Created by lighthouse on 4/23/22.
//

#include "disk.h"


Disk *Disk::disk_instance = nullptr;

Disk::Disk() {
    Superblock *superblock_instance = Superblock::get_instance();
    int block_size = 2 << (10 + superblock_instance->s_log_block_size);
    num_block_group = (DISK_SIZE - 1) / block_size / superblock_instance->s_blocks_per_group + 1;
    block_group = new BlockGroup[num_block_group];
    block_group_size = block_size * superblock_instance->s_blocks_per_group;
}

Disk::~Disk() {
    delete[] block_group;
}

void Disk::init_block_group(int group_id, void *buf) {
    __u8 *u8_buf = reinterpret_cast<__u8 *>(buf);
    if (group_id == 0) {
        // contains superblock and gdt
        block_group[group_id].superblock = Superblock::get_instance();
        block_group[group_id].superblock->traverse_data_to_settings(u8_buf + 0x400);
        block_group[group_id].gdt = new GroupDescriptor[num_block_group];
        for (int i = 0; i < num_block_group; i++)
            block_group[group_id].gdt[i].traverse_data_to_settings(u8_buf + i * GroupDescriptor::GD_SIZE);
        GroupDescriptor &current_gdt = block_group[group_id].gdt[group_id];
        unsigned long long bg_block_bitmap = ((unsigned long long) current_gdt.bg_block_bitmap_hi << 32) |
                                             (unsigned long long) current_gdt.bg_block_bitmap_lo;
        unsigned long long bg_inode_bitmap = ((unsigned long long) current_gdt.bg_inode_bitmap_hi << 32) |
                                             (unsigned long long) current_gdt.bg_inode_bitmap_lo;
        unsigned long long bg_inode_table = ((unsigned long long) current_gdt.bg_inode_table_hi << 32) |
                                            (unsigned long long) current_gdt.bg_inode_table_lo;
        block_group[group_id].block_bitmap_offset = bg_block_bitmap;
        block_group[group_id].block_bitmap = new bool[Superblock::get_instance()->s_blocks_per_group];
        block_group[group_id].inode_bitmap_offset = bg_inode_bitmap;
        block_group[group_id].inode_bitmap = new bool[Superblock::get_instance()->s_inodes_per_group];
        block_group[group_id].inode_table_offset = bg_inode_table;
        // num inode table equals num bits of inode bitmap
        block_group[group_id].inode_table = new Inode[Superblock::get_instance()->s_inodes_per_group];
        block_group[group_id].traverse_data_to_block_bitmap(u8_buf + bg_block_bitmap);
        block_group[group_id].traverse_data_to_inode_bitmap(u8_buf + bg_inode_bitmap);
        for (int i = 0; i < bg_inode_table - bg_inode_bitmap; i++)
            block_group[group_id].inode_table->traverse_data_to_settings(
                    u8_buf + bg_inode_table + i * Inode::INODE_SIZE);
    } else {
        // initialize other groups
        GroupDescriptor &current_gdt = block_group[0].gdt[group_id];
        unsigned long long bg_block_bitmap = ((unsigned long long) current_gdt.bg_block_bitmap_hi << 32) |
                                             (unsigned long long) current_gdt.bg_block_bitmap_lo;
        unsigned long long bg_inode_bitmap = ((unsigned long long) current_gdt.bg_inode_bitmap_hi << 32) |
                                             (unsigned long long) current_gdt.bg_inode_bitmap_lo;
        unsigned long long bg_inode_table = ((unsigned long long) current_gdt.bg_inode_table_hi << 32) |
                                            (unsigned long long) current_gdt.bg_inode_table_lo;
        block_group[group_id].block_bitmap_offset = bg_block_bitmap;
        block_group[group_id].block_bitmap = new bool[Superblock::get_instance()->s_blocks_per_group];
        block_group[group_id].inode_bitmap_offset = bg_inode_bitmap;
        block_group[group_id].inode_bitmap = new bool[Superblock::get_instance()->s_inodes_per_group];
        block_group[group_id].inode_table_offset = bg_inode_table;
        // num inode table equals num bits of inode bitmap
        block_group[group_id].inode_table = new Inode[Superblock::get_instance()->s_inodes_per_group];
        block_group[group_id].traverse_data_to_block_bitmap(u8_buf);
        block_group[group_id].traverse_data_to_inode_bitmap(u8_buf + bg_inode_bitmap - bg_block_bitmap);
        for (int i = 0; i < bg_inode_table - bg_inode_bitmap; i++)
            block_group[group_id].inode_table->traverse_data_to_settings(
                    u8_buf + bg_inode_table - bg_inode_bitmap + i * Inode::INODE_SIZE);
    }
}

void Disk::create_block_groups() {
    for (int group_id = 0; group_id < num_block_group; group_id++) {
        if (group_id == 0) {
            block_group[group_id].superblock = Superblock::get_instance();
            block_group[group_id].gdt = new GroupDescriptor[num_block_group];
            for (int i = 0; i < num_block_group; i++) block_group[group_id].gdt[i].create_default_settings(i);
        }
        GroupDescriptor &current_gdt = block_group[0].gdt[group_id];
        block_group[group_id].block_bitmap_offset = ((unsigned long long) current_gdt.bg_block_bitmap_hi << 32) |
                                                    (unsigned long long) current_gdt.bg_block_bitmap_lo;
        block_group[group_id].inode_bitmap_offset = ((unsigned long long) current_gdt.bg_inode_bitmap_hi << 32) |
                                                    (unsigned long long) current_gdt.bg_inode_bitmap_lo;
        block_group[group_id].inode_table_offset = ((unsigned long long) current_gdt.bg_inode_table_hi << 32) |
                                                   (unsigned long long) current_gdt.bg_inode_table_lo;
        block_group[group_id].block_bitmap = new bool[Superblock::get_instance()->s_blocks_per_group];
        block_group[group_id].inode_bitmap = new bool[Superblock::get_instance()->s_inodes_per_group];
        block_group[group_id].inode_table = new Inode[Superblock::get_instance()->s_inodes_per_group];
    }
}

int Disk::traverse_block_metadata_to_data(int group_id, void *buf) {
    __u8 *u8_buf = reinterpret_cast<__u8 *>(buf);
    const GroupDescriptor &current_gdt = block_group[0].gdt[group_id];
    if (group_id == 0) {
        Superblock::get_instance()->traverse_settings_to_data(u8_buf + 0x400);
        for (int i = 0; i < num_block_group; i++)
            block_group[0].gdt[i].traverse_settings_to_data(u8_buf + 0x800 + GroupDescriptor::GD_SIZE * i);
    }
    unsigned long long bg_block_bitmap = ((unsigned long long) current_gdt.bg_block_bitmap_hi << 32) |
                                         (unsigned long long) current_gdt.bg_block_bitmap_lo;
    unsigned long long bg_inode_bitmap = ((unsigned long long) current_gdt.bg_inode_bitmap_hi << 32) |
                                         (unsigned long long) current_gdt.bg_inode_bitmap_lo;
    unsigned long long bg_inode_table = ((unsigned long long) current_gdt.bg_inode_table_hi << 32) |
                                        (unsigned long long) current_gdt.bg_inode_table_lo;
    block_group[group_id].traverse_block_bitmap_to_data(u8_buf + bg_block_bitmap - group_id * block_group_size);
    block_group[group_id].traverse_inode_bitmap_to_data(u8_buf + bg_inode_bitmap - group_id * block_group_size);
    for (int i = 0; i < Superblock::get_instance()->s_inodes_per_group; i++)
        block_group[group_id].inode_table[i].traverse_settings_to_data(
                u8_buf + bg_inode_table - group_id * block_group_size + i * Inode::INODE_SIZE);
    int block_size = 2 << (10 + Superblock::get_instance()->s_log_block_size);
    int num_inode_table_blocks =
            (Superblock::get_instance()->s_inodes_per_group * Inode::INODE_SIZE - 1) / block_size + 1;
    if (group_id == 0) return bg_inode_table + num_inode_table_blocks * block_size;
    else return bg_inode_table - group_id * block_group_size + num_inode_table_blocks * block_size;
}