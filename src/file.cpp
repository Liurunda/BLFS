//
// Created by lighthouse on 5/3/22.
//

#include "file.h"
#include "disk.h"
#include <string.h>


bool create_file(char *path, int flags) {
    int path_len = strlen(path);
    if (path_len > 500) return false; // path too long
    if (path_len == 1) {
        // root path '/'
        // if root exists, return false;

    }
    for (int i = 0; i < path_len; i++)
}