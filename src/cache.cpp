//
// Created by lighthouse on 5/19/22.
//

#include "cache.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <map>
#include <set>
#include <stdio.h>

Cache *Cache::cache_instance = nullptr;

int Cache::sync_file(int inode_id, int datasync) {
    int block_size = Disk::get_instance()->block_size;
    WCNode *head = wc_hash_table->get(-inode_id);
    if (head == nullptr) {
        puts("Error: cannot find inode id in sync file");
        return -1;
    }
    int fd = open(DISK_PATH, O_RDWR | O_DIRECT | O_NOATIME);
    if (head->metadata->inode_op == INODE_DELETE) {
        // delete inode
        return 0;
    }
    while (head->next_data != nullptr) {
        lseek64(fd, head->next_data->data->block_id * block_size, SEEK_SET);
        int res = write(fd, head->next_data->data->buf, block_size);
        if (res < 0) {
            puts("Error: error occurred when sync file");
            return res;
        }
        RCNode *rc_node = rc_hash_table->get(head->next_data->data->block_id);
        if (rc_node != nullptr) {
            memcpy(rc_node->buf, head->next_data->data->buf, block_size);
        }
        wc_pool.push(head->next_data);
        head->next_data = head->next_data->next_data;
    }
    if (datasync != 0) {
        // synchronize all file data including metadata
        // find changed block
        bool *blocks_to_be_written = new bool[num_bitmap_blocks];
        for (int i = 0; i < num_bitmap_blocks; i++) blocks_to_be_written[i] = false;
        std::map<int, bool> cblocks; // changed blocks
        for (int i = 0; i < head->metadata->num_changed_blocks; i++) {
            const int cblock_id = head->metadata->changed_blocks[i].block_id;
            const bool is_delete = head->metadata->changed_blocks[i].is_delete;
            auto iter = cblocks.find(cblock_id);
            if (iter == cblocks.end()) cblocks.insert(std::make_pair(cblock_id, is_delete));
            else {
                if (iter->second != is_delete) iter->second = is_delete;
            }
        }
        std::set<int> cbufs; // changed bufs
        for (auto iter = cblocks.begin(); iter != cblocks.end(); iter++) {
            int buf_id = iter->first / block_size;
            cbufs.insert(buf_id);
            bool *bool_bitmap_buf = reinterpret_cast<bool *>(block_bitmap_buf[buf_id]);
            bool_bitmap_buf[iter->first % block_size] = iter->second;
        }
        for (auto iter = cbufs.begin(); iter != cbufs.end(); iter++) {
            int res = write(fd, block_bitmap_buf[*iter], block_size);
            if (res < 0) {
                puts("Error: error occurred when sync file");
                return res;
            }
        }

        head->prev->next = head->next;
        head->next->prev = head->prev;
        wc_pool.push(head);

        int group_id = inode_id / Superblock::get_instance()->s_blocks_per_group;
        int inode_offset = inode_id % Superblock::get_instance()->s_blocks_per_group;

        const GroupDescriptor &current_gdt = Disk::get_instance()->block_group[0].gdt[group_id];
        ull bg_inode_table = ((ull) current_gdt.bg_inode_table_hi << 32) | (ull) current_gdt.bg_inode_table_lo;
        int inode_block_id = (bg_inode_table + (ull) inode_offset * Inode::INODE_SIZE) / (ull) block_size;
        read_block(inode_block_id, data_buf);
        Disk::get_instance()->block_group[group_id].inode_table[inode_offset].traverse_data_to_settings(
                reinterpret_cast<__u8 *>(data_buf) +
                Inode::INODE_SIZE * (inode_offset % (block_size / Inode::INODE_SIZE)));
        return write_block(inode_id, inode_block_id, data_buf);
    }
    return 0;
}

int Cache::read_block(int block_id, void *buf) {
    int block_size = Disk::get_instance()->block_size;
    RCNode *rc_node = rc_hash_table->get(block_id);
    if (rc_node == nullptr) {
        // block doesn't exist in read cache
        // read from disk
        if (num_rc < READ_CACHE_SIZE) {
            // create new cache item
            rc_node = new RCNode;
            posix_memalign(&rc_node->buf, 512, block_size);
        } else {
            rc_node = rc_tail->prev;
            rc_hash_table->remove(rc_node->block_id);
        }
        rc_node->block_id = block_id;
        int fd = open(DISK_PATH, O_RDONLY | O_NOATIME | O_DIRECT);
        int res = read(fd, rc_node->buf, block_size);
        if (res < 0) {
            free(rc_node->buf);
            delete rc_node;
            puts("Error occurred when reading block");
            return res;
        }
        rc_hash_table->put(block_id, rc_node);
        rc_node->next = rc_head->next;
        rc_node->prev = rc_head;
        rc_head->next = rc_node;
        rc_node->next->prev = rc_node;
        memcpy(buf, rc_node->buf, block_size);
        return res;
    } else {
        rc_node->prev->next = rc_node->next;
        rc_node->next->prev = rc_node->prev;
        rc_node->next = rc_head->next;
        rc_node->prev = rc_head;
        rc_head->next = rc_node;
        rc_node->next->prev = rc_node;
        memcpy(buf, rc_node->buf, block_size);
        return 0;
    }
}

int Cache::write_block(int inode_id, int block_id, void *buf) {
    int block_size = Disk::get_instance()->block_size;

    WCNode *head = wc_hash_table->get(-inode_id); // uses negative value to distinguish inode id and block id
    if (head == nullptr) {
        head = wc_pool.front();
        wc_pool.pop();
        if (wc_pool.empty()) {
            int res = sync_all();
            if (res < 0) {
                puts("Error occurred when writing block");
                return res;
            }
        }
        if (head->metadata == nullptr) head->metadata = new WCNode::MetaData;
        head->metadata->inode_id = inode_id;
        head->metadata->inode_op = INODE_NO_CHANGE;
        head->metadata->metadata_changed = false;
        head->next_data = nullptr;
        head->next = wc_head->next;
        head->prev = wc_head;
        wc_head->next = head;
        head->next->prev = head;
        wc_hash_table->put(-inode_id, head);
    }

    // find if there is same block
    WCNode *wc_node = wc_hash_table->get(block_id);

    if (wc_node == nullptr) {
        wc_node = wc_pool.front();
        wc_pool.pop();
        if (wc_node->data == nullptr) wc_node->data = new WCNode::Data;
        wc_node->data->block_id = block_id;
        memcpy(wc_node->data->buf, buf, block_id);
        wc_node->next_data = head->next_data;
        head->next_data = wc_node;
        wc_hash_table->put(block_id, wc_node);
    } else {
        memcpy(wc_node->data->buf, buf, block_size);
    }

    // check if write cache is full
    if (wc_pool.empty()) return sync_all();
    return 0;
}

int Cache::update_metadata(int inode_id, INODE_OP inode_op, bool is_add_block, int block_id) {
    WCNode *head = wc_hash_table->get(-inode_id);
    if (head == nullptr) {
        head = wc_pool.front();
        wc_pool.pop();
        if (head->metadata == nullptr) head->metadata = new WCNode::MetaData;
        head->metadata->inode_id = inode_id;
        head->next_data = nullptr;
        head->next = wc_head->next;
        head->prev = wc_head;
        wc_head->next = head;
        head->next->prev = head;
        wc_hash_table->put(-inode_id, head);
    }
    head->metadata->inode_op = inode_op;
    head->metadata->metadata_changed = true;
    if (inode_op == INODE_UPDATE) return 0;
    if (inode_op != INODE_NO_CHANGE) return 0;
    bool found = false;
    for (int i = 0; i < head->metadata->num_changed_blocks; i++) {
        if (head->metadata->changed_blocks[i].block_id == block_id) {
            head->metadata->changed_blocks[i].is_delete = !is_add_block;
            found = true;
            break;
        }
    }
    if (!found) {
        head->metadata->changed_blocks[head->metadata->num_changed_blocks++] = {block_id, !is_add_block};
    }
    if (!is_add_block) {
        // delete block in write cache(if it exists)
        for (WCNode *node = head; node->next_data != nullptr; node = node->next_data) {
            if (node->next_data->data->block_id == block_id) {
                WCNode *d_node = node->next_data;
                node->next_data = d_node->next_data;
                wc_pool.push(d_node);
                break;
            }
        }
    }
    if (wc_pool.empty()) return sync_all();
    return 0;
}

int Cache::sync_all() {
    while (wc_head->next != wc_tail) {
        int ret = sync_file(wc_head->next->metadata->inode_id, 1);
        if (ret != 0) return ret;
    }
    return 0;
}

Cache::Cache() {
    for (int i = 0; i < WRITE_CACHE_SIZE; i++) wc_pool.push(new WCNode);
    num_rc = 0;
    rc_hash_table = new HashTable<RCNode>(READ_TABLE_SIZE);
    wc_hash_table = new HashTable<WCNode>(WRITE_TABLE_SIZE);
    wc_head = new WCNode;
    wc_tail = new WCNode;
    wc_head->next = wc_tail;
    wc_head->prev = nullptr;
    wc_tail->next = nullptr;
    wc_tail->prev = wc_head;
    rc_head = new RCNode;
    rc_tail = new RCNode;
    rc_head->next = rc_tail;
    rc_head->prev = nullptr;
    rc_tail->next = nullptr;
    rc_tail->prev = rc_head;
    Disk *disk = Disk::get_instance();
    posix_memalign(&data_buf, 512, disk->block_size);
    num_bitmap_blocks = Superblock::get_instance()->s_blocks_per_group * disk->num_block_group / disk->block_size;
    block_id_map = (int *) malloc(num_bitmap_blocks * sizeof(int *));
    block_bitmap_buf = (void **) malloc(num_bitmap_blocks * sizeof(void *));
    for (int i = 0; i < num_bitmap_blocks; i++) block_bitmap_buf[i] = (void *) malloc(disk->block_size);
    // read block bitmap blocks
    int num_bitmap_blocks_per_group = Superblock::get_instance()->s_blocks_per_group / disk->block_size;
    for (int i = 0; i < disk->num_block_group; i++) {
        ull offset = ((ull) disk->block_group[0].gdt[i].bg_block_bitmap_hi << 32) |
                     (ull) disk->block_group[0].gdt[i].bg_block_bitmap_lo;
        ull block_id = offset / disk->block_size;
        for (int j = 0; j < num_bitmap_blocks_per_group; j++) {
            read_block(block_id + j, block_bitmap_buf[i * num_bitmap_blocks_per_group + j]);
            block_id_map[i * num_bitmap_blocks_per_group + j] = block_id + j;
        }
    }
}

Cache::~Cache() {
    // clean node pool
    while (!wc_pool.empty()) {
        WCNode *wc_node = wc_pool.front();
        wc_pool.pop();
        delete wc_node;
    }
    // write back write cache data before destroy
    sync_all();
    // delete write cache
    for (WCNode *cur = wc_head->next; cur != wc_tail; cur = cur->next) delete cur->prev;
    delete wc_tail->prev;
    delete wc_tail;
    // delete read cache
    for (RCNode *cur = rc_head->next; cur != rc_tail; cur = cur->next) delete cur->prev;
    delete rc_tail->prev;
    delete rc_tail;
    free(data_buf);
    free(block_id_map);
    for (int i = 0; i < num_bitmap_blocks; i++) free(block_bitmap_buf[i]);
    free(block_bitmap_buf);
}