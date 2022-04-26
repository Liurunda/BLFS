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
}

Disk::~Disk() {
    delete[] block_group;
}

bool Disk::init_block_group(int group_id, void *buf) {
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
        if (bg_inode_bitmap - bg_block_bitmap != Superblock::get_instance()->s_blocks_per_group) return false;
        if (bg_inode_table - bg_inode_bitmap != Superblock::get_instance()->s_inodes_per_group) return false;
        block_group[group_id].block_bitmap_offset = bg_block_bitmap;
        block_group[group_id].block_bitmap = new bool[bg_inode_bitmap - bg_block_bitmap];
        block_group[group_id].inode_bitmap_offset = bg_inode_bitmap;
        block_group[group_id].inode_bitmap = new bool[bg_inode_table - bg_inode_bitmap];
        block_group[group_id].inode_table_offset = bg_inode_table;
        // num inode table equals num bits of inode bitmap
        block_group[group_id].inode_table = new Inode[bg_inode_table - bg_inode_bitmap];
        block_group[group_id].traverse_data_to_block_bitmap(u8_buf + bg_block_bitmap);
        block_group[group_id].traverse_data_to_inode_bitmap(u8_buf + bg_inode_bitmap);
        for (int i = 0; i < bg_inode_table - bg_inode_bitmap; i++)
            block_group[group_id].inode_table->traverse_data_to_settings(
                    u8_buf + bg_inode_table + i * Inode::INODE_SIZE);
        return true;
    } else {
        // initialize other groups
        GroupDescriptor &current_gdt = block_group[0].gdt[group_id];
        unsigned long long bg_block_bitmap = ((unsigned long long) current_gdt.bg_block_bitmap_hi << 32) |
                                             (unsigned long long) current_gdt.bg_block_bitmap_lo;
        unsigned long long bg_inode_bitmap = ((unsigned long long) current_gdt.bg_inode_bitmap_hi << 32) |
                                             (unsigned long long) current_gdt.bg_inode_bitmap_lo;
        unsigned long long bg_inode_table = ((unsigned long long) current_gdt.bg_inode_table_hi << 32) |
                                            (unsigned long long) current_gdt.bg_inode_table_lo;
        if (bg_inode_bitmap - bg_block_bitmap != Superblock::get_instance()->s_blocks_per_group) return false;
        if (bg_inode_table - bg_inode_bitmap != Superblock::get_instance()->s_inodes_per_group) return false;
        block_group[group_id].block_bitmap_offset = bg_block_bitmap;
        block_group[group_id].block_bitmap = new bool[bg_inode_bitmap - bg_block_bitmap];
        block_group[group_id].inode_bitmap_offset = bg_inode_bitmap;
        block_group[group_id].inode_bitmap = new bool[bg_inode_table - bg_inode_bitmap];
        block_group[group_id].inode_table_offset = bg_inode_table;
        // num inode table equals num bits of inode bitmap
        block_group[group_id].inode_table = new Inode[bg_inode_table - bg_inode_bitmap];
        block_group[group_id].traverse_data_to_block_bitmap(u8_buf);
        block_group[group_id].traverse_data_to_inode_bitmap(u8_buf + bg_inode_bitmap - bg_block_bitmap);
        for (int i = 0; i < bg_inode_table - bg_inode_bitmap; i++)
            block_group[group_id].inode_table->traverse_data_to_settings(
                    u8_buf + bg_inode_table - bg_inode_bitmap + i * Inode::INODE_SIZE);
        return true;
    }
}