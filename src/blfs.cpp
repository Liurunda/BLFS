//
// Created by Flager on 2022/4/20.
//

#include<fuse3/fuse.h>


static int blfs_mkdir(const char *path, mode_t mode) {
    return 0;
}

static int blfs_rename(const char *oldpath, const char *newpath, unsigned int flags) {
    return 0;
}

static int blfs_open(const char *path, struct fuse_file_info *fi) {
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

static struct fuse_operations blfs_ops = {
        .mkdir      = blfs_mkdir,
        .rename     = blfs_rename,
        .open       = blfs_open,
        .read       = blfs_read,
        .write      = blfs_write,
        .flush      = blfs_flush,
        .release    = blfs_release,
        .fsync      = blfs_fsync
};

int main() {
    return 0;
}
