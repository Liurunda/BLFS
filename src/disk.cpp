//
// Created by lighthouse on 4/23/22.
//

#include "disk.h"
#include "blfs_functions.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>


Disk *Disk::disk_instance = nullptr;

Disk::Disk() {
    Superblock *superblock_instance = Superblock::get_instance();
    block_size = 2 << (10 + superblock_instance->s_log_block_size);
    num_block_group = (DISK_SIZE - 1) / block_size / superblock_instance->s_blocks_per_group + 1;
    block_group = new BlockGroup[num_block_group];
    block_group_size = block_size * superblock_instance->s_blocks_per_group;
    if (posix_memalign(&block_buf, 512, block_size) < 0) {
        perror("Error in block buf memory allocation");
    }
    empty_buf = malloc(block_size);
    buf = malloc(Inode::INODE_SIZE * superblock_instance->s_inodes_per_group + block_size);
}

Disk::~Disk() {
    delete[] block_group;
    free(block_buf);
    free(empty_buf);
    free(buf);
}

void Disk::init_block_group(int group_id, void *buf) {
    __u8 *u8_buf = reinterpret_cast<__u8 *>(buf);
    if (group_id == 0) {
        // contains superblock and gdt
        block_group[group_id].superblock = Superblock::get_instance();
        block_group[group_id].superblock->traverse_data_to_settings(u8_buf + BlockGroup::SUPERBLOCK_OFFSET);
        block_group[group_id].gdt = new GroupDescriptor[num_block_group];
        for (int i = 0; i < num_block_group; i++)
            block_group[group_id].gdt[i].traverse_data_to_settings(
                    u8_buf + BlockGroup::GDT_OFFSET + i * GroupDescriptor::GD_SIZE);
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
        for (int i = 0; i < Superblock::get_instance()->s_inodes_per_group; i++)
            block_group[group_id].inode_table[i].traverse_data_to_settings(
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
        for (int i = 0; i < Superblock::get_instance()->s_inodes_per_group; i++)
            block_group[group_id].inode_table[i].traverse_data_to_settings(
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
Superblock *superblock = Superblock::get_instance();
block_group[group_id].block_bitmap = new bool[superblock->s_blocks_per_group];
for (int i = 0; i < superblock->s_blocks_per_group; i++) block_group[group_id].block_bitmap[i] = false;
// set bitmap whose block used by metadata to true
ull bytes_used = block_group[group_id].inode_table_offset + Inode::INODE_SIZE * superblock->s_inodes_per_group -
group_id * block_group_size;
int block_used = (bytes_used - 1) / block_size + 1;
for (int i = 0; i < block_used; i++) block_group[group_id].block_bitmap[i] = true;
block_group[group_id].inode_bitmap = new bool[superblock->s_inodes_per_group];
for (int i = 0; i < superblock->s_inodes_per_group; i++) block_group[group_id].inode_bitmap[i] = false;
block_group[group_id].inode_table = new Inode[superblock->s_inodes_per_group];
}

char root_path[2] = {
'/', '\0'
};
create_inode(root_path, 0);
}

int Disk::traverse_block_metadata_to_data(int group_id, void *data_buf) {
__u8 *u8_buf = reinterpret_cast<__u8 *>(data_buf);
const GroupDescriptor &current_gdt = block_group[0].gdt[group_id];
    if (group_id == 0) {
        Superblock::get_instance()->traverse_settings_to_data(u8_buf + BlockGroup::SUPERBLOCK_OFFSET);
        for (int i = 0; i < num_block_group; i++)
            block_group[0].gdt[i].traverse_settings_to_data(
                    u8_buf + BlockGroup::GDT_OFFSET + GroupDescriptor::GD_SIZE * i);
    }
    ull bg_block_bitmap = ((ull) current_gdt.bg_block_bitmap_hi << 32) | (ull) current_gdt.bg_block_bitmap_lo;
    ull bg_inode_bitmap = ((ull) current_gdt.bg_inode_bitmap_hi << 32) | (ull) current_gdt.bg_inode_bitmap_lo;
    ull bg_inode_table = ((ull) current_gdt.bg_inode_table_hi << 32) | (ull) current_gdt.bg_inode_table_lo;
    block_group[group_id].traverse_block_bitmap_to_data(u8_buf + bg_block_bitmap - (ull) group_id * block_group_size);
    block_group[group_id].traverse_inode_bitmap_to_data(u8_buf + bg_inode_bitmap - (ull) group_id * block_group_size);
    for (int i = 0; i < Superblock::get_instance()->s_inodes_per_group; i++)
        block_group[group_id].inode_table[i].traverse_settings_to_data(
                u8_buf + bg_inode_table - group_id * block_group_size + i * Inode::INODE_SIZE);
    int num_inode_table_blocks =
            (Superblock::get_instance()->s_inodes_per_group * Inode::INODE_SIZE - 1) / block_size + 1;
    if (group_id == 0) return bg_inode_table + num_inode_table_blocks * block_size;
    else return bg_inode_table - group_id * block_group_size + num_inode_table_blocks * block_size;
}

int Disk::get_metadata_size(int group_id) {
    const GroupDescriptor &current_gdt = block_group[0].gdt[group_id];
    ull bg_inode_table = ((ull) current_gdt.bg_inode_table_hi << 32) | (ull) current_gdt.bg_inode_table_lo;
    int num_inode_table_blocks =
            (Superblock::get_instance()->s_inodes_per_group * Inode::INODE_SIZE - 1) / block_size + 1;
    if (group_id == 0) return bg_inode_table + num_inode_table_blocks * block_size;
    else return bg_inode_table - group_id * block_group_size + num_inode_table_blocks * block_size;
}

void Disk::update_inode(int inode_id) {
    int group_id = inode_id / Superblock::get_instance()->s_blocks_per_group;
    int i = inode_id % Superblock::get_instance()->s_blocks_per_group;
    // found and write changed block
    const GroupDescriptor &current_gdt = block_group[0].gdt[group_id];
    ull bg_inode_bitmap = ((ull) current_gdt.bg_inode_bitmap_hi << 32) | (ull) current_gdt.bg_inode_bitmap_lo;
    int bitmap_block_id = (bg_inode_bitmap + (ull) i) / block_size;
    block_group[group_id].traverse_inode_bitmap_to_data(buf);
    int block_offset = i / block_size;
    if (write_block(bitmap_block_id, reinterpret_cast<__u8 *>(buf) + block_offset * block_size) < 0) {
        perror("Error when write block bitmap into disk");
    }
    // found and write changed inode
    ull bg_inode_table = ((ull) current_gdt.bg_inode_table_hi << 32) | (ull) current_gdt.bg_inode_table_lo;
    int inode_block_id = (bg_inode_table + (ull) i * Inode::INODE_SIZE) / block_size;
    int num_inode_per_block = block_size / Inode::INODE_SIZE;
    int start_inode_id = (i / num_inode_per_block) * num_inode_per_block;
    for (int j = start_inode_id; j < num_inode_per_block; j++) {
        block_group[group_id].inode_table[j].traverse_settings_to_data(
                reinterpret_cast<__u8 *>(buf) + j * Inode::INODE_SIZE);
    }
    if (write_block(inode_block_id, buf) < 0) {
        perror("Error when write inode into disk");
    }
}

void Disk::update_data(int block_id, void *data) {
    if (write_block(block_id, data) < 0) {
        perror("update data error");
    }
}

void Disk::update_null_data(int block_id) {
    if (write_block(block_id, empty_buf) < 0) {
        perror("update null data error");
    }
}

int Disk::acquire_unused_block() {
    int block_id = -1;
    bool found = false;
    int blocks_per_group = Superblock::get_instance()->s_blocks_per_group;
    int group_id = 0, i = 0;
    for (; group_id < num_block_group; group_id++) {
        for (; i < blocks_per_group; i++) {
            if (!block_group[group_id].block_bitmap[i]) {
                block_group[group_id].block_bitmap[i] = true;
                block_id = group_id * blocks_per_group + i;
                found = true;
                break;
            }
        }
        if (found) break;
    }
    // found changed block
    const GroupDescriptor &current_gdt = block_group[0].gdt[group_id];
    ull bg_block_bitmap = ((ull) current_gdt.bg_block_bitmap_hi << 32) | (ull) current_gdt.bg_block_bitmap_lo;
    int bitmap_block_id = (bg_block_bitmap + (ull) i) / block_size;
    block_group[group_id].traverse_block_bitmap_to_data(buf);
    int block_offset = i / block_size;
    if (write_block(bitmap_block_id, reinterpret_cast<__u8 *>(buf) + block_offset * block_size) < 0) {
    perror("Error when write block bitmap into disk");
            return -1;
            }
            return block_id;
            }

            int Disk::acquire_unused_inode() {
            int inodes_per_group = Superblock::get_instance()->s_inodes_per_group;
for (int group_id = 0; group_id < num_block_group; group_id++) {
for (int i = 0; i < inodes_per_group; i++) {
if (!block_group[group_id].inode_bitmap[i]) {
block_group[group_id].inode_bitmap[i] = true;
return group_id * inodes_per_group + i;
}
}
}
return -1;
}

void Disk::release_block(int block_id) {
int group_id = block_id / Superblock::get_instance()->s_blocks_per_group;
int i = block_id % Superblock::get_instance()->s_blocks_per_group;
block_group[group_id].block_bitmap[i] = false;
    // write changes into disk
    const GroupDescriptor &current_gdt = block_group[0].gdt[group_id];
    ull bg_block_bitmap = ((ull) current_gdt.bg_block_bitmap_hi << 32) | (ull) current_gdt.bg_block_bitmap_lo;
    int bitmap_block_id = (bg_block_bitmap + (ull) i) / block_size;
    block_group[group_id].traverse_block_bitmap_to_data(buf);
    int block_offset = i / block_size;
    if (write_block(bitmap_block_id, reinterpret_cast<__u8 *>(buf) + block_offset * block_size) < 0) {
        perror("Error when write block bitmap into disk");
    }
}

void Disk::read_from_block(int block_id, void *data) {
    if (read_block(block_id, data) < 0) {
        perror("Error occurred when read from block");
    }
}

int Disk::write_block(int block_id, void *data) {
    int disk_fd = open(DISK_PATH, O_RDWR | O_DIRECT | O_NOATIME);
    memcpy(block_buf, data, block_size);
    ull offset = block_id * block_size;
    lseek64(disk_fd, offset, SEEK_SET);
    int res = write(disk_fd, block_buf, block_size);
    close(disk_fd);
    return res;
}

int Disk::read_block(int block_id, void *data) {
    int disk_fd = open(DISK_PATH, O_RDWR | O_DIRECT | O_NOATIME);
    ull offset = block_id * block_size;
    lseek64(disk_fd, offset, SEEK_SET);
    int res = read(disk_fd, block_buf, block_size);
    if (res < 0) return res;
    else memcpy(data, block_buf, block_size);
    close(disk_fd);
    return res;
}