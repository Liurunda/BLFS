//
// Created by Flager on 2022/4/21.
//

#include "superblock.h"

Superblock *Superblock::get_instance() {
    if (super_block == nullptr) {
        super_block = new Superblock();
        return super_block;
    } else return super_block;
}

void Superblock::destroy_instance() {
    if (super_block != nullptr) {
        delete super_block;
        super_block = nullptr;
    }
}