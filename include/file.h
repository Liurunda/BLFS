//
// Created by lighthouse on 5/3/22.
//

#ifndef BLFS_FILE_H
#define BLFS_FILE_H

#include "inode.h"

class BLFSFile {
public:
    char path[100];
    Inode file_inode;
};


bool create_file(char *path, int flags);

#endif //BLFS_FILE_H
