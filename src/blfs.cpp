//
// Created by Flager on 2022/4/20.
//

#define FUSE_USE_VERSION 35

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "blfs_functions.h"


static int blfs_getattr(const char *path, struct stat *buf, struct fuse_file_info *fi) {
    (void) fi;
    int inode_id = find_inode_by_path(path);
    Inode inode = get_inode_by_inode_id(inode_id);
    buf->st_nlink = inode.i_links_count;
    buf->st_mode = inode.i_mode;
    buf->st_uid = inode.i_uid;
    buf->st_gid = inode.i_gid;
    //buf->st_atim = inode.i_atime;


    puts("blfs getattr");
    return 0;
}

static int blfs_mkdir(const char *path, mode_t mode) {
    puts("blfs mkdir");
    return 0;
}

static int blfs_rename(const char *oldpath, const char *newpath, unsigned int flags) {
    puts("blfs rename");
    return 0;
}

static int blfs_open(const char *path, struct fuse_file_info *fi) {
    puts("blfs open");
    return 0;
}

static int blfs_read(const char *path, char *buf, size_t size, off_t off, struct fuse_file_info *fi) {
    puts("blfs read");
    return 0;
}

static int blfs_write(const char *path, const char *buf, size_t size, off_t off, struct fuse_file_info *fi) {
    puts("blfs write");
    return 0;
}

static int blfs_flush(const char *path, struct fuse_file_info *fi) {
    puts("blfs flush");
    return 0;
}

static int blfs_release(const char *path, struct fuse_file_info *fi) {
    puts("blfs release");
    return 0;
}

static int blfs_fsync(const char *path, int datasync, struct fuse_file_info *fi) {
    puts("blfs fsync");
    return 0;
}

static int blfs_opendir(const char *path, struct fuse_file_info *fi) {
    puts("blfs opendir");
    return 0;
}

static int
blfs_readdir(const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *, enum fuse_readdir_flags) {
    puts("blfs readdir");
    return 0;
}

static void *blfs_init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
    struct fuse_context *context;
    blfunc_init();
    context = fuse_get_context();
    return context->private_data;
}

static struct fuse_operations blfs_ops = {
        .getattr        = blfs_getattr,
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
        .opendir        = blfs_opendir,
        .readdir        = blfs_readdir,
        .releasedir     = nullptr,
        .fsyncdir       = nullptr,
        .init           = blfs_init
};

int main(int argc, char *argv[]) {
    int ret = fuse_main(argc, argv, &blfs_ops, nullptr);
    return ret;
}
