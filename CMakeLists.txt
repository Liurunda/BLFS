cmake_minimum_required(VERSION 3.10)
project(BLFS)

set(CMAKE_CXX_STANDARD 17)

IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_definitions(-D LINUX)
ENDIF()
add_compile_options(
        -D_FILE_OFFSET_BITS=64
)

set(SRC_FILES
        blfs.cpp
        inode.cpp
        superblock.cpp
)

set(HEADER_FILES
        disk.h
        superblock.h
        inode.h
)

add_executable(${PROJECT_NAME} ${SRC_FILES} ${HEADER_FILES})
