//
// Created by Flager on 2022/4/21.
//

#ifndef BLFS_DISK_H
#define BLFS_DISK_H

#include <string>

template<typename T>
class Descriptor {
public:
    Descriptor(int offset, T content) : offset(offset), content(content) {}
    void set_content(T new_content) {content = new_content;}
    void get_offset() {return offset;}
    void get_content() {return content;}

private:
    int offset;
    T content;
};

typedef Descriptor<char> Descriptorc;
typedef Descriptor<uint8_t> Descriptor8;
typedef Descriptor<uint16_t> Descriptor16;
typedef Descriptor<uint32_t> Descriptor32;

#endif //BLFS_DISK_H
