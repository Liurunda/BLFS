//
// Created by lighthouse on 4/23/22.
//

#include "blfs_functions.h"
#include "disk.h"
#include "superblock.h"
#include "inode.h"
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

int blfunc_init() {
    Superblock *superblock = Superblock::get_instance();
    Disk *disk = Disk::get_instance();
    if (access(DISK_PATH, F_OK | R_OK | W_OK) == -1) {
        // disk file does not exist
        if (access(DISK_PATH, F_OK) == 0) {
            // exist file cannot be read or write
            if (remove(DISK_PATH) != 0) {
                perror("Remove disk path error");
                return -1;
            }
        }
        int fd = open(DISK_PATH, O_RDWR | O_CREAT | O_DIRECT | O_NOATIME, 0777);
        if (fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, 0, DISK_SIZE) < 0) {
            close(fd);
            perror("Preallocate failed");
            return -1;
        }

        // create disk
        disk->create_block_groups();
        void *block_group_buf = nullptr;
        if (posix_memalign(&block_group_buf, 512, disk->block_group_size) < 0) {
            close(fd);
            perror("Failed to alloc aligned buffer for block group");
            return -1;
        }
        for (int i = 0; i < disk->num_block_group; i++) {
            int data_size = disk->traverse_block_metadata_to_data(i, block_group_buf);
            lseek64(fd, i * disk->block_group_size, SEEK_SET);
            if (write(fd, block_group_buf, data_size) < 0) {
                free(block_group_buf);
                perror("Failed to write block group data into disk");
                close(fd);
                return -1;
            }
        }
        free(block_group_buf);
        close(fd);
    } else {
        int fd = open(DISK_PATH, O_RDONLY | O_DIRECT | O_NOATIME);
        void *block_group_data = nullptr;
        int ret = posix_memalign(&block_group_data, 512, Disk::get_instance()->block_group_size);
        if (ret < 0) {
            perror("Failed to alloc aligned buffer for block group data");
            return -1;
        }
        for (int i = 0; i < Disk::get_instance()->num_block_group; i++) {
            lseek64(fd, i * Disk::get_instance()->block_group_size, SEEK_SET);
            if (i == 0) {
                if (read(fd, block_group_data, Disk::get_instance()->block_group_size) < 0) {
                    perror("Failed to read block group data into disk");
                    free(block_group_data);
                    return -1;
                }
            } else {
                int data_size = Disk::get_instance()->get_metadata_size(i);
                if (read(fd, block_group_data, data_size) < 0) {
                    perror("Failed to read block group data into disk");
                    free(block_group_data);
                    return -1;
                }
            }
            Disk::get_instance()->init_block_group(i, block_group_data);
        }
        free(block_group_data);
        if (!Superblock::get_instance()->validate_checksum()) {
            perror("Superblock checksum validation failed");
            return -1;
        }
    }
    return 0;
}

int find_inode_by_name(const char *name, Inode parent_inode,Inode &child_inode){
    int block_num = parent_inode.i_size_lo / block_size;
    int directory_size = block_size/sizeof(Directory);
    Directory *directory = new Directory[directory_size];
    
    for(int i=0; i<block_num; i++){
        int block_id = parent_inode.i_block[i];
        assert(sizeof(Dirctory)==256);        
        read_from_block(block_id,directory);
        for(int j=0; j<directory_size; j++){
            if(strcmp(directory[j].name,name)==0){
                child_inode = get_inode_by_inode_id(directory[j].inode_id);
                delete[] directory;
                return 0;
            }
        }        
    }
    delete[] directory;
    return -1;
}

int find_inode_by_path(const char *path, Inode &inode) {
    int path_length = strlen(path);
    if (path_length == 0) {
        puts("Path length is zero");
        return -1;
    }
    if (path[0] != '/') {
        printf("Path %s does not start with '/'\n", path);
        return -1;
    }
    if (path_length == 1) {
        // root path
        inode = get_inode_by_inode_id(0);        
        return 0; // root inode id is always 0
    }
    else {        
        // while(){

        // }
    }
    // puts("Not Implemented");
    // return -1;
}

Inode get_inode_by_inode_id(int inode_id) {
    Superblock *superblock = Superblock::get_instance();
    int group_id = inode_id / superblock->s_inodes_per_group;
    int inode_id_in_group = inode_id % superblock->s_inodes_per_group;
    return Disk::get_instance()->block_group[group_id].inode_table[inode_id_in_group];
}

int create_inode(const char *path, int flags) {
    // find parent directory inode
    // if parent_inode is -1, then path should equals '/'
    int path_length = strlen(path);
    if (path_length == 0) {
        puts("Path length is zero");
        return -1;
    }
    if (path[0] != '/') {
        printf("Path %s does not start with '/'\n", path);
    }
    int parent_inode = -1;
    if (path_length > 1) {
        int parent_dir_length = path_length - 1;
        for (; parent_dir_length > 0; parent_dir_length--) {
            if (path[parent_dir_length] == '/') {
                parent_dir_length--;
                break;
            }
        }
        char *parent_path = new char[parent_dir_length + 1];
        memcpy(parent_path, path, parent_dir_length * sizeof(char));
        parent_path[parent_dir_length] = '\0';
        parent_inode = find_inode_by_path(parent_path);
        if (parent_inode < 0) {
            printf("Cannot find path %s\n", parent_path);
            delete[] parent_path;
            return -1;
        }
        delete[] parent_path;
    }

    // find empty inode
    // should be locked when multi-thread
    Disk *disk = Disk::get_instance();
    Superblock *superblock = Superblock::get_instance();
    bool found = false;
    int inode = -1;
    int group_id = 0, inode_id = 0;
    for (group_id = 0; group_id < disk->num_block_group; group_id++) {
        for (inode_id = 0; inode_id < superblock->s_inodes_per_group; inode_id++) {
            if (!disk->block_group[group_id].inode_bitmap[inode_id]) {
                found = true;
                inode = group_id * superblock->s_inodes_per_group + inode_id;
                break;
            }
        }
        if (found) break;
    }
    if (parent_inode == -1) {
        // create root directory
        assert(inode == 0);
        disk->block_group[0].inode_bitmap[0] = true;
        Inode &current_inode = disk->block_group[0].inode_table[0];
        // use standard flags: drwxr-xr-x
        current_inode.i_mode = S_IXOTH | S_IROTH | S_IXGRP | S_IRGRP | S_IXUSR | S_IWUSR | S_IRUSR | S_IFDIR;
        for (int block_id = 0; block_id < superblock->s_blocks_per_group; block_id++) {
            if (!disk->block_group[group_id].block_bitmap[block_id]) {
                current_inode.i_block[0] = block_id;
                break;
            }
        }
    } else {
        puts("Not Implemented");
        return -1;
    }
    return inode;
};

inline int write_disk(int fd, long offset, void *buf, int size) {
    lseek64(fd, offset, SEEK_SET);
    if (write(fd, buf, size) < 0) {
        perror("Failed to write data into disk");
        return -1;
    }
    return 0;
}