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
                perror("Remove disk path error");
                return -1;
            }
        }
        int fd = open(DISK_PATH, O_RDWR | O_CREAT | O_DIRECT | O_NOATIME);
        if(fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, 0, DISK_SIZE) < 0) {
            perror("Preallocate failed");
            return -1;
        }

        // init superblock
        void* superblock_data = nullptr;
        if(posix_memalign(&superblock_data, 512, 1024) < 0) {
            perror("Failed to alloc aligned buffer for superblock data");
            return -1;
        }
        Superblock::get_instance()->traverse_settings_to_data(superblock_data);
        lseek64(fd, 1024, SEEK_SET);
        if(write(fd, superblock_data, 1024) < 0) {
            perror("Failed to write superblock data into disk");
            return -1;
        }
    }
    return 0;
}