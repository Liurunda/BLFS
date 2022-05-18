//
// Created by Flager on 2022/4/20.
//

#define FUSE_USE_VERSION 35

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "blfs_functions.h"
#include "disk.h"
#include <errno.h>
#include <stdlib.h>
#include <assert.h>


static int blfs_getattr(const char *path, struct stat *buf, struct fuse_file_info *fi) {
    (void) fi;
    printf("%s\n", path);
    int inode_id = find_inode_by_path(path);
    if (inode_id < 0) return -ENOENT;
    Inode *inode = get_inode_by_inode_id(inode_id);
    if (inode == nullptr) return -ENOENT;
    buf->st_nlink = inode->i_links_count;
    buf->st_mode = inode->i_mode;
    buf->st_uid = inode->i_uid;
    buf->st_gid = inode->i_gid;
//buf->st_atim = inode.i_atime;


    puts("blfs getattr");
    return 0;
}

static int blfs_mknod(const char *path, mode_t mode, dev_t rdev) {
    puts("blfs mknod");
    return 0;
}

static int blfs_mkdir(const char *path, mode_t mode) {
    puts("blfs mkdir");
    // find last inode of the path
    int inode_id = find_inode_by_path(path);
    if(inode_id!=-1){
        puts("mkdir error: already exists");
        return -1;//the path already exists
    }

    int len = strlen(path);
    char* modify_path = new char[len+1];
    strcpy(modify_path, path);

    if(modify_path[len-1]=='/')modify_path[len-1] = '\0';
    
    int findr = strrchr(modify_path,'/') - modify_path;
    int parent_inode_id;
    if(findr!=0){
        modify_path[findr] = '\0';
        parent_inode_id = find_inode_by_path(modify_path);//parent inode
    } else {
        parent_inode_id = find_inode_by_path("/");
    }
    if (parent_inode_id == -1) {
        puts("mkdir error: cannot find parent");
        delete[] modify_path;
        return -1;
    }
    //we got the parent's directory now
    int new_inode_id = Disk::get_instance()->acquire_unused_inode();
    Inode *new_inode = get_inode_by_inode_id(new_inode_id);
    if (new_inode == nullptr) {
        delete[] modify_path;
        return -ENOENT;
    }
    Inode *parent = get_inode_by_inode_id(parent_inode_id);
    if (parent == nullptr) {
        delete[] modify_path;
        return -ENOENT;
    }
    new_inode->i_mode = S_IXOTH | S_IROTH | S_IXGRP | S_IRGRP | S_IXUSR | S_IWUSR | S_IRUSR | S_IFDIR; //mode & 0xffff;
    new_inode->i_links_count = 1;
    int block_size = Disk::get_instance()->block_size;
    ull i_size = ((ull) parent->i_size_high << 32) | (ull) parent->i_size_lo;
    ull block_num = i_size == 0 ? 0 : (i_size - 1) / block_size + 1;

    //need a new block?
    if (i_size % block_size == 0) {//full
        int new_block_id = Disk::get_instance()->acquire_unused_block();
        parent->add_block(new_block_id);
        block_num += 1;
    }
    int last_block = parent->get_kth_block_id(block_num - 1);
    //modify parent inode block    
    int offset = (i_size%block_size) / DIRECTORY_LENGTH;
    DirectoryItem* items = new DirectoryItem[block_size / DIRECTORY_LENGTH];
    Disk::get_instance()->read_from_block(last_block,(void*)items);
    items[offset].inode_id = new_inode_id;
    if (len - findr - 1 > DIRECTORY_LENGTH - 4) {//too long
        puts("Mkdir error: name too long");
        delete[] modify_path;
        delete[] items;
        return -1;
    }
    strcpy(items[offset].name, modify_path + findr + 1);
    Disk::get_instance()->update_data(last_block, (void *) items);
    //update i_size
    i_size += DIRECTORY_LENGTH;
    parent->i_size_high = i_size >> 32;
    parent->i_size_lo = i_size & (0xffffffff);
    Disk::get_instance()->update_inode(parent_inode_id);//parent inode
    Disk::get_instance()->update_inode(new_inode_id);//parent inode
    delete[] modify_path;
    delete[] items;
    return 0;
}

static int blfs_unlink(const char *path) {
    int inode_id = find_inode_by_path(path);
    int res = remove_file_from_dir(path);
    if (res != 0) return res;
    Inode *inode = get_inode_by_inode_id(inode_id);
    if (inode == nullptr) return -ENOENT;
    inode->i_links_count -= 1;
    Disk::get_instance()->update_inode(inode_id);
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
    int inode_id = find_inode_by_path(path);
    if (inode_id < 0) return -ENOENT;
    Inode *inode = get_inode_by_inode_id(inode_id);
    if (inode == nullptr) return -ENOENT;
    if (!(inode->i_mode & S_IFDIR)) return -ENXIO;

    char *bbuf;

    int block_size = Disk::get_instance()->block_size;
    off_t offset = off;
    int block_id = off / block_size;
    offset = off % block_size;

    int read_size = 0;
    while (read_size < size + offset) {
        Disk::get_instance()->read_from_block(inode->get_kth_block_id(block_id), bbuf + read_size);
        block_id++;
        read_size = strlen(bbuf);
    }

    memcpy(buf, bbuf+offset ,size);
    delete[] bbuf;
    puts("blfs read");
    return size;    

}

static int blfs_write(const char *path, const char *buf, size_t size, off_t off, struct fuse_file_info *fi) {
    int inode_id = find_inode_by_path(path);
    if (inode_id < 0) return -ENOENT;
    Inode *inode = get_inode_by_inode_id(inode_id);
    if (inode == nullptr) return -ENOENT;
    if (!(inode->i_mode & S_IFDIR)) return -ENXIO;

    char *bbuf;

    int block_size = Disk::get_instance()->block_size;
    off_t offset = off;
    int block_id = off / block_size;
    offset = off % block_size;

    int to_write_size = 0;
    int already_write_size = 0;
    if(block_size-offset>size)
        to_write_size = size;
    else
        to_write_size = block_size-offset;

    //第一部分，有offset时的写入
    memcpy(bbuf + offset, buf, to_write_size - already_write_size);
    Disk::get_instance()->update_data(inode->get_kth_block_id(block_id), bbuf);
    already_write_size = to_write_size;
    if(to_write_size + block_size > size){
        to_write_size = size;            
    }
    else{
        to_write_size += block_size;
    }

    while(to_write_size<size){
        memcpy(bbuf, buf + already_write_size, to_write_size - already_write_size);
        Disk::get_instance()->update_data(inode->get_kth_block_id(block_id), bbuf);
        already_write_size = to_write_size;
        if(to_write_size + block_size > size){
            to_write_size = size;            
        }
        else{
            to_write_size += block_size;
        }
    }
    delete []bbuf;
    puts("blfs write");
    return size;
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

static int blfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t off, struct fuse_file_info *fi,
                        enum fuse_readdir_flags flags) {
    int inode_id = find_inode_by_path(path);
    if (inode_id < 0) return -ENOENT;
    Inode *inode = get_inode_by_inode_id(inode_id);
    if (inode == nullptr) return -ENOENT;
    if (!(inode->i_mode & S_IFDIR)) return -ENXIO;
    //if (filler(buf, ".", nullptr, 0, FUSE_FILL_DIR_PLUS) != 0) return 1;
    if (inode_id != 0)
        if (filler(buf, "..", nullptr, 0, FUSE_FILL_DIR_PLUS) != 0) return 1;
    ull dir_size = ((ull) inode->i_size_high << 32) | (ull) inode->i_size_lo;
    ull num_files = dir_size / DIRECTORY_LENGTH;
    int block_size = Disk::get_instance()->block_size;
    int dir_last_block = dir_size / block_size;
    int dir_item_per_block = block_size / DIRECTORY_LENGTH;
    DirectoryItem *items = new DirectoryItem[dir_item_per_block];
    for (int i = 0; i <= dir_last_block; i++) {
        Disk::get_instance()->read_from_block(inode->get_kth_block_id(i), items);
        if (i == dir_last_block) {
            int num_file_offset = num_files % dir_item_per_block;
            for (int j = 0; j < num_file_offset; j++)
                if (filler(buf, items[j].name, nullptr, 0, FUSE_FILL_DIR_PLUS) != 0)
                    return 1;
        } else {
            for (int j = 0; j < dir_item_per_block; j++)
                if (filler(buf, items[j].name, nullptr, 0, FUSE_FILL_DIR_PLUS) != 0)
                    return 1;
        }
    }
    puts("blfs readdir");
    return 0;
}

static void *blfs_init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
    struct fuse_context *context;
    blfunc_init();
    context = fuse_get_context();
    return context->private_data;
}

static int blfs_access(const char *path, int mask) {
    puts("blfs access");
    return 0;
}

static int blfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    puts("blfs create");
    // find directory inode
    int path_length = strlen(path);
    for (; path_length > 0; path_length--) if (path[path_length] == '/') break;
    char *path_dir = (char *) malloc((path_length + 2) * sizeof(char));
    memcpy(path_dir, path, (path_length + 1) * sizeof(char));
    path_dir[path_length + 1] = '\0';
    int inode_id = find_inode_by_path(path_dir);
    free(path_dir);
    if (inode_id < 0) return -ENOENT;
    Inode *inode = get_inode_by_inode_id(inode_id);
    if (inode == nullptr) return -ENOENT;

    // add file name to directory
    ull dir_size = ((ull) inode->i_size_high << 32) | (ull) inode->i_size_lo;
    ull num_files = dir_size / sizeof(DirectoryItem);
    int block_size = Disk::get_instance()->block_size;
    int dir_last_block = dir_size / block_size;
    int new_inode_id = -1;
    int dir_item_per_block = block_size / sizeof(DirectoryItem);
    DirectoryItem *items = new DirectoryItem[dir_item_per_block];
    if (dir_size % block_size == 0) {
        // add new block
        int block_id = Disk::get_instance()->acquire_unused_block();
        inode->add_block(block_id);
        new_inode_id = Disk::get_instance()->acquire_unused_inode();
        items[0].inode_id = new_inode_id;
        memcpy(items[0].name, path + path_length + 1, strlen(path) - path_length - 1);
        Disk::get_instance()->update_data(block_id, items);
    } else {
        int file_offset = num_files % dir_item_per_block;
        Disk::get_instance()->read_from_block(inode->get_kth_block_id(dir_last_block), items);
        new_inode_id = Disk::get_instance()->acquire_unused_inode();
        items[file_offset].inode_id = new_inode_id;
        memcpy(items[file_offset].name, path + path_length + 1, strlen(path) - path_length - 1);
        Disk::get_instance()->update_data(inode->get_kth_block_id(dir_last_block), items);
    }
    Inode *new_inode = get_inode_by_inode_id(new_inode_id);
    assert(new_inode != nullptr);
    new_inode->i_mode = (__le16) (mode & 0xFFFF);
    new_inode->i_links_count += 1;
    Disk::get_instance()->update_inode(new_inode_id);
    dir_size += sizeof(DirectoryItem);
    inode->i_size_high = (__le32) ((dir_size & 0xFFFFFFFF00000000) >> 32);
    inode->i_size_lo = (__le32) (dir_size & 0xFFFFFFFF);
    Disk::get_instance()->update_inode(inode_id);
    return 0;
}

static int blfs_utimens(const char *path, const struct timespec tv[2], struct fuse_file_info *fi) {
    puts("blfs utimens");
    return 0;
}


static struct fuse_operations blfs_ops = {
        .getattr            = blfs_getattr,
        .readlink           = nullptr,
        .mknod              = blfs_mknod,
        .mkdir              = blfs_mkdir,
        .unlink             = blfs_unlink,
        .rmdir              = nullptr,
        .symlink            = nullptr,
        .rename             = blfs_rename,
        .link               = nullptr,
        .chmod              = nullptr,
        .chown              = nullptr,
        .truncate           = nullptr,
        .open               = blfs_open,
        .read               = blfs_read,
        .write              = blfs_write,
        .statfs             = nullptr,
        .flush              = blfs_flush,
        .release            = blfs_release,
        .fsync              = blfs_fsync,
        .setxattr           = nullptr,
        .getxattr           = nullptr,
        .listxattr          = nullptr,
        .removexattr        = nullptr,
        .opendir            = blfs_opendir,
        .readdir            = blfs_readdir,
        .releasedir         = nullptr,
        .fsyncdir           = nullptr,
        .init               = blfs_init,
        .destroy            = nullptr,
        .access             = blfs_access,
        .create             = blfs_create,
        .lock               = nullptr,
        .utimens            = blfs_utimens,
        .bmap               = nullptr,
        .ioctl              = nullptr,
        .poll               = nullptr,
        .write_buf          = nullptr,
        .read_buf           = nullptr,
        .flock              = nullptr,
        .fallocate          = nullptr,
        .copy_file_range    = nullptr
};

int main(int argc, char *argv[]) {
    int ret = fuse_main(argc, argv, &blfs_ops, nullptr);
    return ret;
}
