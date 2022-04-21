cmake_minimum_required(VERSION 3.10)
project(BLFS)

set(CMAKE_CXX_STANDARD 17)

set(SRC_FILES
        blfs.cpp
)

set(HEADER_FILES

        disk.h superBlock.h inode.h inode.cpp)

add_executable(${PROJECT_NAME} ${SRC_FILES} ${HEADER_FILES})
