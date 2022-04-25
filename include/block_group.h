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

    const int superblock_offset = 0x400;
    Superblock* superblock; // have value only in the first block
    const int gdt_offset = 0x800;
    GroupDescriptor* gdt; // have value only in the first block
    bool* block_bitmap;
    bool* inode_bitmap;
    Inode* inode_table;
};

#endif //BLFS_BLOCK_GROUP_H
