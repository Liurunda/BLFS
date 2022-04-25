cmake_minimum_required(VERSION 3.10)
project(BLFS)

set(CMAKE_CXX_STANDARD 17)

IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_definitions(-D LINUX)
ENDIF()
add_compile_options(
        -D_FILE_OFFSET_BITS=64
)

# -------------crc32c-------------
add_subdirectory(third-party/crc32c)
include_directories(
        third-party/crc32c/include
)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_FILES)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        )

find_library(LIBFUSE3 fuse3 /usr/local/lib/x86_64-linux-gnu)

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        ${LIBFUSE3}
        crc32c
        )
