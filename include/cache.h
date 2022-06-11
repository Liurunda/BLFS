//
// Created by lighthouse on 5/19/22.
//

#ifndef BLFS_CACHE_H
#define BLFS_CACHE_H


#define READ_CACHE_SIZE 8192 // 8192 blocks, 32MB if a block is 4KB
#define WRITE_CACHE_SIZE 256 // 256 blocks, 1MB if a block is 4KB

#include "disk.h"
#include <queue>
#include <stdlib.h>
#include <unistd.h>


enum INODE_OP {
    INODE_NO_CHANGE,
    INODE_ADD,
    INODE_DELETE,
    INODE_UPDATE
};

class Cache {
public:
    Cache(Cache &) = delete;

    Cache(Cache &&) = delete;

    void operator=(const Cache &) = delete;

    int sync_file(int inode_id, int datasync);

    static Cache *get_instance() {
        if (cache_instance == nullptr) {
            cache_instance = new Cache();
        }
        return cache_instance;
    }

    static void destroy_instance() {
        if (cache_instance != nullptr) {
            delete cache_instance;
            cache_instance = nullptr;
        }
    }

    friend Disk;

private:
    Cache();

    ~Cache();

    static Cache *cache_instance;

    int read_block(int block_id, void *buf);

    int write_block(int inode_id, int block_id, void *buf);

    /**
     *
     * @param inode_id which inode the changed metadata belongs to
     * @param inode_op what should be done to inode metadata.
     * @param is_add_block if inode_op is zero, then it refers to add block or not. True refers to add, false refers to delete
     * @param block_id which block is deleted or added
     */
    int update_metadata(int inode_id, INODE_OP inode_op, bool is_add_block, int block_id);

    int sync_all();

    // write cache node
    struct WCNode {
        struct MetaData {
            int inode_id;
            INODE_OP inode_op;
            bool metadata_changed;
            struct {
                int block_id;
                bool is_delete;
            } changed_blocks[WRITE_CACHE_SIZE];
            int num_changed_blocks;

            MetaData() {
                for (int i = 0; i < WRITE_CACHE_SIZE; i++) changed_blocks[i].block_id = -1;
                num_changed_blocks = 0;
            }
        } *metadata;

        struct Data {
            int block_id;
            void *buf;

            Data() {
                posix_memalign(&buf, 512, Disk::get_instance()->block_size);
            }

            ~Data() {
                free(buf);
            }
        } *data;

        WCNode *next_data;
        WCNode *next, *prev;
    } *wc_head, *wc_tail;

    std::queue<WCNode *> wc_pool;

    // read cache node
    struct RCNode {
        int block_id;
        void *buf;
        RCNode *next, *prev;
    } *rc_head, *rc_tail;

    int num_rc; // read cache num blocks, write cache num blocks

    const static int READ_TABLE_SIZE = READ_CACHE_SIZE + 5;
    const static int WRITE_TABLE_SIZE = WRITE_CACHE_SIZE + 1;

    template<typename T>
    struct HashTable {

        HashTable(int table_size) {
            this->table_size = table_size;
            table = new Table[table_size];
            for (int i = 0; i < table_size; i++) {
                table[i].block_id = -1;
                table[i].node = nullptr;
                table[i].next = nullptr;
            }
        }

        ~HashTable() {
            delete[] table;
        }

        void put(int block_id, T *node) {
            int id = block_id % table_size;
            if (table[id].block_id == -1) {
                table[id].block_id = block_id;
                table[id].node = node;
                return;
            }
            Table *t = &table[id];
            while (t->next != nullptr) {
                if (t->block_id == block_id) {
                    t->node = node;
                    return;
                }
                t = t->next;
            }
            if (t->block_id == block_id) {
                t->node = node;
                return;
            }
            Table *new_table = new Table;
            new_table->block_id = block_id;
            new_table->node = node;
            new_table->next = nullptr;
            t->next = new_table;
            return;
        }

        T *get(int block_id) {
            int id = block_id % table_size;
            if (table[id].block_id == -1) {
                return nullptr;
            }
            Table *t = &table[id];
            while (t != nullptr) {
                if (t->block_id == block_id) {
                    return t->node;
                }
                t = t->next;
            }
            return nullptr;
        }

        void remove(int block_id) {
            int id = block_id % table_size;
            if (table[id].block_id == -1) return;
            Table *t = &table[id];
            while (t != nullptr) {
                if (t->block_id == block_id) {
                    if (t->next == nullptr) {
                        t->block_id = -1;
                        t->node = nullptr;
                    } else {
                        Table *nt = t->next;
                        t->block_id = nt->block_id;
                        t->node = nt->node;
                        t->next = nt->next;
                        delete nt;
                    }
                }
                t = t->next;
            }
        }

        int table_size;
        struct Table {
            int block_id;
            T *node;
            Table *next;
        } *table;
    };

    HashTable<RCNode> *rc_hash_table;
    HashTable<WCNode> *wc_hash_table;

    void *data_buf;
    void **block_bitmap_buf;
    int *block_id_map;
    int num_bitmap_blocks;
};


#endif //BLFS_CACHE_H
