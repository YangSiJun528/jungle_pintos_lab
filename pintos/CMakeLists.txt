cmake_minimum_required(VERSION 3.16)
project(pintos_index C ASM)

# Indexing-only CMake model for CLion.
# This file also lets CLion work when the pintos/ directory is opened directly.

set(PINTOS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(PINTOS_INDEX_INCLUDE_DIRS
    ${PINTOS_SOURCE_DIR}
    ${PINTOS_SOURCE_DIR}/include/lib
    ${PINTOS_SOURCE_DIR}/include
    ${PINTOS_SOURCE_DIR}/include/lib/kernel
    ${PINTOS_SOURCE_DIR}/lib/kernel
    ${PINTOS_SOURCE_DIR}/include/lib/user
)

set(PINTOS_INDEX_DEFINITIONS
    USERPROG
    FILESYS
    VM
)

include_directories(${PINTOS_INDEX_INCLUDE_DIRS})
add_compile_definitions(${PINTOS_INDEX_DEFINITIONS})

file(GLOB_RECURSE PINTOS_INDEX_SOURCES CONFIGURE_DEPENDS
    ${PINTOS_SOURCE_DIR}/*.c
    ${PINTOS_SOURCE_DIR}/*.h
    ${PINTOS_SOURCE_DIR}/*.S
)

list(FILTER PINTOS_INDEX_SOURCES EXCLUDE REGEX ".*/build/.*")

add_library(pintos_index OBJECT EXCLUDE_FROM_ALL
    ${PINTOS_INDEX_SOURCES}
)
