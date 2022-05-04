//
// Created by Flager on 2022/4/21.
//

#ifndef BLFS_DISK_H
#define BLFS_DISK_H

#include <linux/types.h>
#include "superblock.h"
#include "block_group.h"


typedef unsigned long long ull;
const char DISK_PATH[] = "/tmp/disk";
const ull DISK_SIZE = 17179869184; // 16G

class Disk {
public:
    Disk(Disk &) = delete;

    Disk(Disk &&) = delete;

    void operator=(const Disk &) = delete;

    static Disk *get_instance() {
        if (disk_instance == nullptr) {
            disk_instance = new Disk();
        }
        return disk_instance;
    }

    static void destroy_instance() {
        if (disk_instance != nullptr) {
            delete disk_instance;
            disk_instance = nullptr;
        }
    }

    void init_block_group(int group_id, void *buf);

    void create_block_groups();

    // returns size of block metadata
    int traverse_block_metadata_to_data(int block_id, void *buf);

    int get_metadata_size(int group_id);

    static const ull BOOTBLOCK_OFFSET = 0;
    static const ull BOOTBLOCK_SIZE = 1024;
    static const ull SUPERBLOCK_OFFSET = 1024;
    static const ull SUPERBLOCK_SIZE = 1024;


    BlockGroup *block_group;
    int num_block_group;
    ull block_group_size;
private:
    static Disk *disk_instance;

    Disk();

    ~Disk();
};

#endif //BLFS_DISK_H
