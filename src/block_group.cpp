//
// Created by lighthouse on 4/26/22.
//

#include "block_group.h"
#include <string.h>

BlockGroup::BlockGroup() {

}

BlockGroup::~BlockGroup() {
    delete[] block_bitmap;
    delete[] inode_bitmap;
    delete[] inode_table;
}

void BlockGroup::traverse_data_to_block_bitmap(void *buf) {
    bool *bool_buf = reinterpret_cast<bool *>(buf);
    memcpy(block_bitmap, bool_buf, sizeof(bool) * (inode_bitmap_offset - block_bitmap_offset));
}

void BlockGroup::traverse_data_to_inode_bitmap(void *buf) {
    bool *bool_buf = reinterpret_cast<bool *>(buf);
    memcpy(inode_bitmap, bool_buf, sizeof(bool) * (inode_table_offset - inode_bitmap_offset));
}

void BlockGroup::traverse_block_bitmap_to_data(void *buf) {
    bool *bool_buf = reinterpret_cast<bool *>(buf);
    memcpy(bool_buf, block_bitmap, sizeof(bool) * (inode_bitmap_offset - block_bitmap_offset));
}

void BlockGroup::traverse_inode_bitmap_to_data(void *buf) {
    bool *bool_buf = reinterpret_cast<bool *>(buf);
    memcpy(bool_buf, inode_bitmap, sizeof(bool) * (inode_table_offset - inode_bitmap_offset));
}