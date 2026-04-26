cmake_minimum_required(VERSION 3.16)
project(pintos_lab_index C ASM)

# Indexing-only CMake model for CLion.
# Build Pintos with the existing Makefile/Docker targets, not with CMake.

add_subdirectory(pintos)

file(GLOB_RECURSE PINTOS_LAB_DOCS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/*.md
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    ${CMAKE_CURRENT_SOURCE_DIR}/pintos/README.md
)

if(PINTOS_LAB_DOCS)
    add_custom_target(pintos_lab_docs SOURCES
        ${PINTOS_LAB_DOCS}
    )
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
        ${PINTOS_LAB_DOCS}
    )
endif()
