//
// Created by Flager on 2022/4/20.
//

#define FUSE_USE_VERSION 35

#include <fuse3/fuse.h>
#include <stdio.h>
#include "blfs_functions.h"

static int blfs_mkdir(const char *path, mode_t mode) {
    return 0;
}

static int blfs_rename(const char *oldpath, const char *newpath, unsigned int flags) {
    return 0;
}

static int blfs_open(const char *path, struct fuse_file_info *fi) {
    puts("blfs open");
    return 0;
}

static int blfs_read(const char *path, char *buf, size_t size, off_t off, struct fuse_file_info *fi) {
    return 0;
}

static int blfs_write(const char *path, const char *buf, size_t size, off_t off, struct fuse_file_info *fi) {
    return 0;
}

static int blfs_flush(const char *path, struct fuse_file_info *fi) {
    return 0;
}

static int blfs_release(const char *path, struct fuse_file_info *fi) {
    return 0;
}

static int blfs_fsync(const char *path, int datasync, struct fuse_file_info *fi) {
    return 0;
}

static void *blfs_init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
    struct fuse_context *context;
    blfunc_init();
    context = fuse_get_context();
    return context->private_data;
}

static struct fuse_operations blfs_ops = {
        .getattr        = nullptr,
        .readlink       = nullptr,
        .mknod          = nullptr,
        .mkdir          = blfs_mkdir,
        .unlink         = nullptr,
        .rmdir          = nullptr,
        .symlink        = nullptr,
        .rename         = blfs_rename,
        .link           = nullptr,
        .chmod          = nullptr,
        .chown          = nullptr,
        .truncate       = nullptr,
        .open           = blfs_open,
        .read           = blfs_read,
        .write          = blfs_write,
        .statfs         = nullptr,
        .flush          = blfs_flush,
        .release        = blfs_release,
        .fsync          = blfs_fsync,
        .setxattr       = nullptr,
        .getxattr       = nullptr,
        .listxattr      = nullptr,
        .removexattr    = nullptr,
        .opendir        = nullptr,
        .readdir        = nullptr,
        .releasedir     = nullptr,
        .fsyncdir       = nullptr,
        .init           = blfs_init
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &blfs_ops, nullptr);
}
