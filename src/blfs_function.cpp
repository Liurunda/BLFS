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
#include <errno.h>

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

int find_inode_by_name(const char *name, Inode *parent_inode) {
    if (parent_inode == nullptr) return -1;
    int block_size = Disk::get_instance()->block_size;
    ull i_size = ((ull) parent_inode->i_size_high << 32) | (ull) parent_inode->i_size_lo;
    ull block_num = i_size == 0 ? 0 : (i_size - 1) / block_size + 1;
    int directory_size = block_size / DIRECTORY_LENGTH;
    DirectoryItem *directoryItem = new DirectoryItem[directory_size];

    for (int i = 0; i < block_num; i++) {
        int block_id = parent_inode->get_kth_block_id(i);
        assert(block_id > 0);
        Disk::get_instance()->read_from_block(block_id, directoryItem);
        int num_items_in_current_block =
                i == block_num - 1 ? ((i_size - 1) % Disk::get_instance()->block_size + 1) / DIRECTORY_LENGTH
                                   : directory_size;
        for (int j = 0; j < num_items_in_current_block; j++) {
            if (strcmp(directoryItem[j].name, name) == 0) {
                int child_inode_id = directoryItem[j].inode_id;
                delete[] directoryItem;
                return child_inode_id;
            }
        }
    }
    delete[] directoryItem;
    return -1;
}

int find_inode_by_path(const char *path) {
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
        return 0; // root inode id is always 0
    } else {
        // path may end with '/'
        char *temp_path = new char[strlen(path)];
        strcpy(temp_path, path);
        char *name = strtok(temp_path, "/");
        int inode_id = 0;
        while (name != NULL) {
            inode_id = find_inode_by_name(name, get_inode_by_inode_id(inode_id));
            name = strtok(NULL, "/");
            if (inode_id == -1) {
                return -1;
            }
        }
        return inode_id;
    }
}

Inode *get_inode_by_inode_id(int inode_id) {
    Superblock *superblock = Superblock::get_instance();
    int group_id = inode_id / superblock->s_inodes_per_group;
    int inode_id_in_group = inode_id % superblock->s_inodes_per_group;
    if (Disk::get_instance()->block_group[group_id].inode_bitmap[inode_id_in_group])
        return &Disk::get_instance()->block_group[group_id].inode_table[inode_id_in_group];
    else return nullptr;
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
    int inode = disk->acquire_unused_inode();
    int group_id = inode / superblock->s_inodes_per_group;
    if (parent_inode == -1) {
        // create root directory
        assert(inode == 0);
        Inode &current_inode = disk->block_group[0].inode_table[inode];
        // use standard flags: drwxr-xr-x
        current_inode.i_mode = S_IXOTH | S_IROTH | S_IXGRP | S_IRGRP | S_IXUSR | S_IWUSR | S_IRUSR | S_IFDIR;
        current_inode.i_links_count += 1;
        disk->update_inode(inode);
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

int remove_file_from_dir(const char *file_name) {
    assert(strlen(file_name) > 1);
    char *temp_path = new char[strlen(file_name)];
    strcpy(temp_path, file_name);
    char *name = nullptr;
    int last_inode_id = 0;
    int inode_id = 0;
    while (true) {
        if (inode_id == 0) name = strtok(temp_path, "/");
        else name = strtok(NULL, "/");
        if (name == nullptr) break;
        last_inode_id = inode_id;
        inode_id = find_inode_by_name(name, get_inode_by_inode_id(last_inode_id));
        if (inode_id == -1) return -ENOENT;
    }
    // last_inode_id is directory inode, and inode_id is file inode
    // now find inode_id in the directory

    Inode *dir_inode = get_inode_by_inode_id(last_inode_id);

    int block_size = Disk::get_instance()->block_size;
    ull i_size = ((ull) dir_inode->i_size_high << 32) | (ull) dir_inode->i_size_lo;
    ull block_num = i_size == 0 ? 0 : (i_size - 1) / block_size + 1;
    int directory_size = block_size / DIRECTORY_LENGTH;
    DirectoryItem *directoryItem = new DirectoryItem[directory_size];

    for (int i = 0; i < block_num; i++) {
        int block_id = dir_inode->get_kth_block_id(i);
        assert(block_id > 0);
        Disk::get_instance()->read_from_block(block_id, directoryItem);
        int num_items_in_current_block =
                i == block_num - 1 ? ((i_size - 1) % Disk::get_instance()->block_size + 1) / DIRECTORY_LENGTH
                                   : directory_size;
        for (int j = 0; j < num_items_in_current_block; j++) {
            if (directoryItem[j].inode_id == inode_id) {
                // remove this item
                if (i == block_num - 1) {
                    int num_items_in_last_block =
                            ((i_size - 1) % Disk::get_instance()->block_size + 1) / DIRECTORY_LENGTH;
                    if (j != num_items_in_last_block - 1) {
                        // swap two items
                        directoryItem[j].inode_id = directoryItem[num_items_in_last_block - 1].inode_id;
                        memcpy(directoryItem[j].name, directoryItem[num_items_in_last_block - 1].name,
                               DIRECTORY_LENGTH - 4);
                    }
                    i_size -= DIRECTORY_LENGTH;
                    dir_inode->i_size_high = (__le32) ((i_size & 0xFFFFFFFF00000000) >> 32);
                    dir_inode->i_size_lo = (__le32) (i_size & 0xFFFFFFFF);
                    if (num_items_in_last_block == 1) Disk::get_instance()->release_block(block_id);
                    else Disk::get_instance()->update_data(block_id, directoryItem);
                    Disk::get_instance()->update_inode(last_inode_id);
                } else {
                    // read last block
                    DirectoryItem *lastItems = new DirectoryItem[directory_size];
                    int last_block_id = dir_inode->get_kth_block_id(block_num - 1);
                    Disk::get_instance()->read_from_block(last_block_id, lastItems);
                    int num_items_in_last_block =
                            ((i_size - 1) % Disk::get_instance()->block_size + 1) / DIRECTORY_LENGTH;
                    // swap two items
                    directoryItem[j].inode_id = lastItems[num_items_in_last_block - 1].inode_id;
                    memcpy(directoryItem[j].name, lastItems[num_items_in_last_block - 1].name, DIRECTORY_LENGTH - 4);
                    i_size -= DIRECTORY_LENGTH;
                    dir_inode->i_size_high = (__le32) ((i_size & 0xFFFFFFFF00000000) >> 32);
                    dir_inode->i_size_lo = (__le32) (i_size & 0xFFFFFFFF);
                    Disk::get_instance()->update_data(block_id, directoryItem);
                    Disk::get_instance()->update_inode(last_inode_id);
                    delete[] lastItems;
                }
                delete[] directoryItem;
                return inode_id;
            }
        }
    }
    delete[] directoryItem;
    return -ENOENT;
}