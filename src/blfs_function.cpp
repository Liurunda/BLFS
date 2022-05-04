//
// Created by lighthouse on 4/23/22.
//

#include "blfs_functions.h"
#include "disk.h"
#include "superblock.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int blfunc_init() {
    if(access(DISK_PATH, F_OK | R_OK | W_OK) == -1) {
        // disk file does not exist
        if(access(DISK_PATH, F_OK) == 0) {
            // exist file cannot be read or write
            if(remove(DISK_PATH) != 0) {
                // Remove disk path error
                return -1;
            }
        }
        int fd = open(DISK_PATH, O_RDWR | O_CREAT | O_DIRECT | O_NOATIME, 0777);
        if (fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, 0, DISK_SIZE) < 0) {
            // Preallocate failed
            return -1;
        }

        // create disk
        Disk::get_instance()->create_block_groups();
        void *block_group_buf = nullptr;
        if (posix_memalign(&block_group_buf, 512, Disk::get_instance()->block_group_size) < 0) {
            // Failed to alloc aligned buffer for block group
            return -1;
        }
        for (int i = 0; i < Disk::get_instance()->num_block_group; i++) {
            int data_size = Disk::get_instance()->traverse_block_metadata_to_data(i, block_group_buf);
            lseek64(fd, i * Disk::get_instance()->block_group_size, SEEK_SET);
            if (write(fd, block_group_buf, data_size) < 0) {
                free(block_group_buf);
                // Failed to write block group data into disk
                return -1;
            }
        }
        free(block_group_buf);

        // create root dir
    }
    else {
        int fd = open(DISK_PATH, O_RDONLY | O_DIRECT | O_NOATIME);
        void *block_group_data = nullptr;
        int ret = posix_memalign(&block_group_data, 512, Disk::get_instance()->block_group_size);
        if (ret < 0) {
            // Failed to alloc aligned buffer for block group data
            return -1;
        }
        for (int i = 0; i < Disk::get_instance()->num_block_group; i++) {
            lseek64(fd, i * Disk::get_instance()->block_group_size, SEEK_SET);
            if (i == 0) {
                if (read(fd, block_group_data, Disk::get_instance()->block_group_size) < 0) {
                    // Failed to read block group data into disk
                    free(block_group_data);
                    return -1;
                }
            } else {
                int data_size = Disk::get_instance()->get_metadata_size(i);
                if (read(fd, block_group_data, data_size) < 0) {
                    // Failed to read block group data into disk
                    free(block_group_data);
                    return -1;
                }
            }
            Disk::get_instance()->init_block_group(i, block_group_data);
        }
        free(block_group_data);
        if (!Superblock::get_instance()->validate_checksum()) {
            // Superblock checksum validation failed
            return -1;
        }
    }
    return 0;
}
