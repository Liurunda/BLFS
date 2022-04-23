//
// Created by lighthouse on 4/23/22.
//

#include "superblock.h"
#include "crc32c/crc32c.h"
#include <time.h>
#include <stdlib.h>


Superblock *Superblock::superblock_instance = nullptr;

Superblock::Superblock() {
    srand(time(NULL));
    // basic settings
    s_first_data_block = 1;
    s_log_block_size = 4;               // 4KiB
    s_log_cluster_size = 4;             // bigalloc disabled
    s_blocks_per_group = 8192;
    s_clusters_per_group = 8192;
    s_inodes_per_group = 2008;

    // generate UUID
    for(int i = 0; i < 8; i ++) s_uuid[i] = (__u8)(rand() % 255);

    s_magic = 0xEF53;

    s_checksum_type = 1;                // the only valid checksum algorithm crc32
    s_checksum_seed = crc32c::Crc32c(s_uuid, ~0);

    // set checksum
    s_checksum = 0;
    char* buf = new char[Superblock::SUPERBLOCK_SIZE];
    traverse_setting_to_data(buf);
    buf[Superblock::SUPERBLOCK_SIZE - 4] = '\0';
    s_checksum = crc32c::Crc32c(buf, ~0);
}

void Superblock::traverse_setting_to_data(void *buf) {

}