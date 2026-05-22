# [Week05] Pintos Project4-0 File System Overview

Source: https://youtu.be/bqtjcc7-_yA?si=XcxTIi-JIpqSXjue

## Project 4 Scope

Project 4 is the file system project.

There are three major features to implement. First, implement a buffer cache for the Pintos file system. Second, change the file representation so that files are indexed and extensible. Third, implement subdirectories.

Before implementing those features, we need to understand the basic data structures in the Pintos file system.

## Inodes

An inode represents a file on disk. Every file has its own inode.

An inode contains metadata about the file, such as the size of the file and the locations of the data blocks that belong to the file. In a real file system, it can also contain permissions and timestamps such as access time and modification time.

There are two kinds of inodes to distinguish.

The first is the on-disk inode. This is the representation stored on the disk. The second is the in-memory inode, sometimes called an in-core inode. When the operating system wants to access a file, it reads the on-disk inode and builds an in-memory inode from it.

The in-memory inode is a superset of the on-disk inode. It contains the on-disk inode data, plus runtime information such as the disk sector where the inode itself is stored, whether the file has been removed, the number of open references, and write-denial state.

## File Objects

An inode represents the file itself, but a file object represents an open file.

When a file is opened, the operating system creates a file object. The most important field in the file object is the current offset. The current offset is the position inside the file where the next read or write operation will be applied.

When a read or write system call is executed, the operation starts at the current offset. After the operation completes, the offset is advanced by the number of bytes that were read or written.

This distinction is important. Several file objects can refer to the same inode, but each open file object can have its own current offset.

## Regular Files, Directories, and Bitmap Files

A regular file has data blocks. Those blocks contain the actual contents of the file, such as text, source code, document data, image data, or video data. The inode records the size of the file and the locations of the data blocks.

In the original Pintos file system, a regular file is represented by a start sector and a size. This means the file occupies a consecutive region on disk. If the file is 1 GB, the original representation expects a consecutive 1 GB region.

A directory is also a file. A directory file contains an array of directory entries. Each directory entry maps a file name to an inode sector. In other words, a directory is a set of file-name and inode-number pairs.

A bitmap is also represented as a file in Pintos. The bitmap is an array of bits. Each bit tells whether the corresponding disk sector is free or in use. Because the bitmap is stored as a file, it also has its own inode.

## Pintos File System Layout

Assume an 8 MB Pintos file system partition. Pintos uses 512-byte sectors, so an 8 MB partition contains 16,384 sectors. The sectors are numbered from 0 to 16,383.

The basic layout is:

Sector 0 contains the inode for the free-map bitmap file.

Sector 1 contains the inode for the root directory.

Sectors 2 through 5 contain the bitmap data itself.

Sector 6 contains the data block for the root directory.

The remaining sectors are used as inode sectors or file data sectors.

The free-map bitmap needs one bit per sector. Four sectors provide 2,048 bytes, which is 16,384 bits. Therefore, four bitmap sectors are enough to represent an 8 MB file system partition with 16,384 sectors.

If the file system partition becomes larger, the bitmap must also become larger.

## On-Disk Inodes in the Original File System

In the original Pintos file system, each on-disk inode occupies one 512-byte sector.

The original `struct inode_disk` stores the start sector of the file, the length of the file, and a magic number. Most of the 512-byte sector is unused padding.

This representation is simple, but it has an important limitation. Because the inode stores only a start sector and a length, all data blocks for the file must be consecutive on disk. This makes it hard to extend files after they are created.

One objective of Project 4 is to replace this single-extent representation with an indexed representation.

## Directory Entries

A directory file contains directory entries.

Each directory entry stores the sector number of the associated inode, a file name, and a flag that indicates whether the entry is in use.

In Pintos, file names are limited to `NAME_MAX`, which is 14 characters, plus one byte for the null terminator. A directory is therefore a linear array of fixed-size entries.

To find a file name, Pintos scans the directory entries linearly. This is simple, but it can become expensive if a directory contains many files.

An important point is that the file name is not stored in the inode. The file name belongs to the directory entry. The directory maps a human-readable name to the inode sector that represents the file.

## Free Map

The free map records which sectors in the file system partition are free and which are already in use.

In Pintos, the free map is stored as a bitmap. The bitmap itself is stored as a file. That means the free map has an inode and data blocks, just like other files.

When Pintos allocates a sector for a new inode or data block, it updates the free map. When a sector is released, Pintos marks the corresponding bit free again.

## Struct File

`struct file` is created when a file is opened.

It contains a pointer to the associated inode, the current file position, and a flag used for denying writes. The position field is the current offset used by read and write operations.

This object exists only for open files. A closed file still has an inode on disk, but it does not have a live `struct file` object for that open instance.

## Project 4 Features

The first feature is the buffer cache. The buffer cache uses part of memory as a cache for disk blocks. It is the opposite direction of virtual memory: virtual memory can use disk as an extension of memory, while the buffer cache uses memory as a cache for disk.

In this project, the cache is expected to hold 64 disk blocks. When a block is read or written, the operation should go through the buffer cache. Dirty cached blocks must eventually be written back to disk.

The second feature is indexed and extensible files. The original Pintos inode stores a start sector and a length, so each file is one consecutive extent. Project 4 changes the inode so it can point to blocks through direct, indirect, and double-indirect pointers. This allows a file to grow even if the next sector after the file is already occupied.

The third feature is subdirectories. The original Pintos file system has only the root directory. Project 4 adds a hierarchical directory structure, so directories can contain regular files and other directories.
