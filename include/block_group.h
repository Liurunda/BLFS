//
// Created by lighthouse on 4/25/22.
//

#ifndef BLFS_BLOCK_GROUP_H
#define BLFS_BLOCK_GROUP_H

#include "superblock.h"
#include "group_descriptor.h"
#include "inode.h"

class BlockGroup {
public:
    BlockGroup();

    ~BlockGroup();

    void traverse_data_to_block_bitmap(void *);

    void traverse_data_to_inode_bitmap(void *);

    void traverse_block_bitmap_to_data(void *);

    void traverse_inode_bitmap_to_data(void *);

    void set_block_bitmap_zero();

    void set_inode_bitmap_zero();

    const int superblock_offset = 0x400;
    Superblock *superblock; // have value only in the first block
    const int gdt_offset = 0x800;
    GroupDescriptor *gdt; // have value only in the first block
    unsigned long long block_bitmap_offset;
    bool *block_bitmap;
    unsigned long long inode_bitmap_offset;
    bool *inode_bitmap;
    unsigned long long inode_table_offset;
    Inode *inode_table;
};

#endif //BLFS_BLOCK_GROUP_H
