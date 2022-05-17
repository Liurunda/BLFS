//
// Created by lighthouse on 4/23/22.
//

#ifndef BLFS_BLFS_FUNCTIONS_H
#define BLFS_BLFS_FUNCTIONS_H

#include "inode.h"

int blfunc_init();

int find_inode_by_name(const char *name, Inode parent_inode,Inode &child_inode);

int find_inode_by_path(const char *path, Inode &inode);

Inode get_inode_by_inode_id(int inode_id);

int create_inode(const char *path, int flags);

int write_disk(int fd, long offset, void *buf, int size);

#endif //BLFS_BLFS_FUNCTIONS_H
