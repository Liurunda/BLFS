//
// Created by Flager on 2022/4/21.
//

#include "inode.h"

byte_array::byte_array(int length): length(length) {
    bytes = new unsigned char[length];
}

byte_array::~byte_array() {
    delete[] bytes;
}

bool byte_array::set_bytes(unsigned char* new_bytes, size_t len) {
    if(len > length) return false;
    memcpy(bytes, new_bytes, len * sizeof(unsigned char));
    return true;
}

bool byte_array::get_bytes(unsigned char* new_bytes, size_t len) {
    if(len > length) return false;
    memcpy(new_bytes, bytes, len);
    return true;
}