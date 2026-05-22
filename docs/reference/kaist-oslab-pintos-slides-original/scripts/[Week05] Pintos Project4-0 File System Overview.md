# [Week05] Pintos Project4-0 File System Overview

Source: https://youtu.be/bqtjcc7-_yA?si=XcxTIi-JIpqSXjue

## Project 4 Overview

Hi. We are going to explain the detailed steps of implementing project number four. This is the file system.

There are basically three things in project number four. We are going to implement the buffer cache in the Pintos file system. Then we are going to extend the Pintos file system such that the individual files are represented by a set of blocks, indexed and made extensible. Then we are going to implement subdirectories.

Before we move on, we are going to explain a few basic concepts in the file system.

## Inode

One of the most important data structures in the file system is the inode. Sorry, inode.

An inode represents a file on the disk, a file on the disk. So every file has its own inode as a data structure. And what does it represent? It contains the size of the file, how large the file is, and it contains the locations of data blocks that belong to a file. So it may be a pointer to the disk blocks that belong to a file. Also, it contains permissions, the time it has been accessed for the last time, the most recent modification time, etc.

There are two types of inode. There are on-disk inode and in-memory inode. On the disk, there is an array of inodes, and each of the inodes represents the files on the disk. When the operating system wants to access a file on the disk, it has to read the inode from the disk, and based upon the disk-based inode, it builds an in-memory inode.

So in-memory inode is a superset of on-disk inode. This is on-disk inode, and in-memory inode, or in-core inode, represents a data structure that contains the on-disk inode and some other information. The other information includes the disk location of the on-disk inode, and also it may contain the file system that the inode belongs to.

## File Object and Current Offset

Another important data structure is a file object. So there is an inode object and there is a file object. A file object represents an open file.

In UNIX, when a file is opened, the operating system defines the current offset. This is one of the most important concepts in a modern file system. Current offset denotes the position within a file where the read or write application, the read or write system calls, have to be applied.

For example, here this is a big file, and this is the start of the file, and this is the end of the file. Then the UNIX operating system defines an attribute called current offset, which represents a location where the read or write operation has to be applied. Once the read or write system call is applied, then the offset is updated by the amount of data that has been read or that has been written, based upon the read/write system call. So it is updated.

Okay. So there are two concepts, inode and file object.

## Regular Files, Directories, and Bitmap

Before we move on, let me introduce another basic concept. There is a file, this is just a regular file, and this is directory, and this is bitmap.

Each file has actual data. These are the data blocks. It may be a music file, it may be a video file, it may be a Microsoft Word file, or it may be C code. Anyway, it may contain music contents, video images, documents, or C files. This is the actual contents of the file.

However, every file has its associated inode. The inode contains, as I told you before, a size, permissions, and each inode contains a pointer to the data blocks which it holds.

Okay. So this is how a regular file is represented. In this form, the block of a file is represented by the start address and start address. Sorry, this should be size. My apologies. So at the beginning of the file this is start address, and this is size.

In this type of representation, data that belongs to a file is represented by the start address on the disk and the size of the data that it occupies on the disk. If it occupies three megabytes, then file size is going to be 3 megabytes. If it is a movie file and the size of the file corresponds to 1 gigabyte, then this file occupies consecutive 1 gigabyte of data blocks, consecutively. That is very important.

So this is how Pintos represents a file. This is the current form.

The second thing is directory. A directory is a set of files. I guess all of you are very familiar with the term directory, except what you probably do not know is the exact definition of the directory. A directory is a set of file name and inode number pairs. So directory is file name and inode number pair. Yeah, this is the definition of directory.

But in Pintos, directory itself is also a file. That is the standard way of representing a directory in a UNIX file system. The same as in UNIX, Pintos defines a directory as a file. It means that a directory has its own inode. The inode has a pointer to the data block location.

Different from a regular file, a directory file contains an array of file name and inode pairs. Each pair is represented by file name and inode number. Each entry is called an entry in the directory, which is called a directory entry.

The third concept you might want to know is bitmap. Bitmap is an array of bits. Very easy. So 0, 0, 1, 0, 0, 1, 0, 0, something, something. This is bitmap. We are going to explain why and how it may be applied into this operating system.

Anyway, in Pintos, bitmap is also represented as a file. Represented as a file means that the bitmap has its own inode, and the inode has the location, or contains the location, of the bitmap.

Okay. So that is a very coarse explanation and basic concepts of the Pintos operating system and Pintos file system.

## Pintos File System Layout

Let's move on. This is the file system layout in Pintos.

Let's consider, currently, I guess you have defined the Pintos file system with 8 megabytes file-system size. 8 megabytes is pretty small, a very, very small file-system partition.

In Pintos, the block size is 512 bytes. The term block in a modern operating system is the unit of I/O to the disk. That is what we call the size of the block. In the Pintos operating system, block size corresponds to 512 bytes. These days, normally the block size is 4 kilobytes.

Since the file system partition is 8 megabytes, there are 16 thousand blocks in this file-system partition. This is the total file-system partition layout. There are 16,000 blocks, from block number 0 to number 16,383. This is the last one.

Every block is located by its index. Okay, that is the basic size, from block 0 to block 16,383. So many blocks.

These are the detailed layout of the Pintos file-system partition. One thing you might want to remember is that in Pintos, on-disk inode is 512 bytes large. It is very big. Of course inode does not use all its 512 bytes of space. Only a fraction of the 512-byte block is used for containing information from the inode.

Anyway, the first block contains an inode for the bitmap file. I am going to explain what the bitmap is shortly later. The second block, which is block 1, contains an inode for the root directory. From block 2 to block 5, it contains a block bitmap.

Okay. Block 0 contains an inode for the bitmap file, and these are the actual contents of the bitmap file. So block 0 contains the inode, and it should contain the start address of the bitmap file like this. The block bitmap consists of four blocks, from block 2 to block 5.

Then block number 6 is contents of the directory. It contains inode and file-name pairs for the files that reside in the root directory. So this says inode number and file name. This is the detailed structure of this part.

Then there comes a bunch of data blocks.

## Bitmap Size

Okay. Let me explain what the bitmap is. Bitmap is an array of bits, of course, and each bit represents whether the associated block is being used or not.

There are four blocks, and each block has 512 bytes, and each byte is 8 bits. So we need to allocate 16,384 bits to represent whether each of the blocks in the file-system partition is being used or not.

There are 16,384 blocks in this file-system partition, so we need to allocate this many bits to represent whether each of the blocks is being used or not. So we are allocating four blocks for bitmap.

But if you double the file-system partition from 8 megabytes, for example to 32 megabytes, if we increase the file-system partition size from 8 megabytes to 32 megabytes, then of course we have to allocate 16 blocks for the bitmap, not 4, because we have to quadruple the number of blocks that has been used for block bitmap.

So this is the file-system layout in Pintos.

## Block 0 and Root Directory Contents

This is the details. The first block was inode for the bitmap, and the second block was inode for the root directory. Blocks 2 to 5 were the bitmap, and this was the root directory. Okay. And then there are data blocks. This is plain data blocks.

Let's look at the contents of block number 0. This is the inode block for the bitmap. In Pintos, each file block is represented by the start address and the length. Okay, so it says the start address of its contents is block number 2, and the length of the file is 2048 bytes. This is unit bytes.

There comes a magic number. You know, this is part of this data structure design. The integer array of 125 integers is being unused in this data structure. Also assume that this is 4 bytes in this session. This is 4 bytes, and 125 times 4, which is 500 bytes, is being unused.

All right. Well, this is the design of the Pintos file system. If you have time to modify it, and if you have time to make it look better, then you are welcome to modify this file-system partition. I am sure that you should be able to do that.

Okay. Then let's look at the root directory. Its start address is 6. The data block for the root directory file starts at block number 6, and the size of the file is 320 bytes. So it fits within a block, because the block size is 512 bytes. Then there comes a huge space of unused space.

Okay, so this is the inode structure.

Then let's look at the contents of the root directory. As we have covered before, a directory is an array, or sorry, a directory is a set of file name and inode. File name and inode.

In Pintos data structure, a directory entry is an array of file name and inode, and the length of the file name is fixed to 14 bytes at maximum. That is how Pintos defines file name. In Pintos, the file name cannot exceed 14 characters. However, in modern operating systems, the length of the file name is virtually infinite. You can use a 100-character file name or a 200-character file name.

Anyway, this is how Pintos represents a directory.

From block number 7, there comes data block. Data block means you can use the blocks that start from block number 7 to 16,000. You can use those blocks as an inode or as the data block that belongs to an inode.

In this file-system layout, inode number 7, block number 7, contains an inode block. The name of the file, we do not know. We do not know what the name of the file is, but let me get back to the point later.

Anyway, the inode data block starts at 8, and the length of the file is 1024. So the length of the file is two blocks, and it starts from 8. This is the data block for a file block pointed by the inode block which is stored at block number 7.

There is another block. Block number 10 contains an inode block, and its data block starts at 11. It consists of 2048 bytes, which corresponds to four blocks. So it will be 11, 12, if we have more space in 13 and 14, then this file will consist of four blocks.

Okay. The inode that is stored at block number 10 is pointing to block number 11, and it contains information about the file size, and the file size is 2048, like this here.

## File Names Are in Directory Entries

Okay. Let me get back to the root directory structure.

The first entry in the root directory says that the name of the file is `myfile` and its inode number is number 7. So it is inode number, but actually it represents the location of the associated inode. The file name is `myfile`, and the associated inode is stored at block number 7.

The second entry is `file.c`. This is the file name, and the inode number is 10. But it represents the inode location. The inode for `file.c` is stored at inode number 10.

There is a very, very interesting phenomenon here. As you see in inode, there is no file name. If you look at the inode, there is no file name field. No.

You know what? In modern operating systems, in modern file systems, file name is not part of the file attributes. For us human beings, we recognize a file by the string which we call file name. But from the computer systems point of view, file name has nothing to do with the file itself. It is not a part of the file attribute.

Directory data structure relates a character string called file name to its inode. That is the important, interesting characteristic of a modern file system.

## In-Memory Inode

Okay. So there are two types of inode. The first one is in-memory inode, and the second one is on-disk inode.

Inode represents a file on the disk. All right. So let's first look at the inode we call in-memory inode. Sometimes we call this in-core inode.

In Pintos, the name of the data structure for in-memory inode is `struct inode`. In-memory inode contains the address of the on-disk inode on the disk. It represents a block number where the inode is stored. Then it contains the disk inode. That is it. Then it contains a flag, whether to delete the file or not.

Let's look at the actual data structure. The easiest part, the sector, is the location of the inode on the disk. So this part is done. And the last part is the data structure for on-disk inode.

As I told you before, in-memory inode is a superset of the on-disk inode. In-memory inode, or in-core inode, harbors the on-disk inode. This is on-disk inode. It contains a sector which represents the location of the inode on the disk, and then it contains the flag `removed` to denote whether a file has been deleted or not.

Of course the file can be deleted, but the operating system does not immediately deallocate the in-memory inode when a file is deleted. Normally, operating systems deallocate the in-memory inodes in an asynchronous manner. So when you delete a file, the operating system just marks that this file, this data structure, the in-memory inode, needs to be deallocated, and does it later at some time. That is what this is for.

As we covered before, some files are not writable. For example, the executable file, or the files that are being loaded to disk, they should not be modified while the operating system modifies it. So the Pintos operating system defines the field in the in-memory inode to denote whether the file can be writable or not.

So the text is `deny_write_cnt`. The reason this is a count, not a flag, is that there are multiple processes accessing the file. In that case, the file cannot be modified until none of the processes are accessing the file. For those reasons, Pintos file system defines a deny write count, not a deny write flag.

The last thing, there is open count. It represents the number of processes, or number of open system calls, that have opened a given file. So that is the structure of in-memory inode.

## On-Disk Inode

Okay. Next part is a more essential data structure called on-disk inode. It represents a file on the disk. The name of the data structure is `struct inode_disk`.

This is very large. This is 512 bytes. That is very sad. Of course, unfortunately, out of 512 bytes, 500 bytes is not used.

This is not good. However, that is the way Pintos defines inode. If you have time, then spend some time on modifying this data structure and make it more efficient. Give it a try. It is very easy.

Anyway, the important part is how the Pintos operating system defines a file. In the Pintos operating system, the file system represents a file as a single block of large chunk. It is pointed by the start address and the size. This is how the Pintos file system defines a file.

## Directory Object and Directory Entry

Okay. This is the data structure of a directory object.

A directory object is a format of data block for the directory file. So a directory file has its own inode. Its own inode contains a pointer to the start address of the data block and its size, and it will contain something.

This is an in-memory data structure that represents a directory block. `struct dir` contains the pointer to the associated inode and the position. The position defines the next directory entry to read or write. So it represents an open directory.

There are two fields: inode, the pointer to the associated in-memory inode, and then `pos`, which is the position of the next directory entry to read and write. That is the directory object.

Let me explain the directory entry. A directory data block consists of an array of directory entries, which I have explained before. Each directory entry is actually a file name or inode number pair. Right?

So `inode_sector` is the sector number of the associated inode. Then there comes a file name. That slot may not be in use. In that case, we need a flag to represent whether the current slot is being used or not.

So the actual data structure for directory entry is represented by `inode_sector`, a character string whose maximum number is 14 plus 1, and a flag to denote whether a given directory slot is being used or not. As you can see, this is the data structure for directory entry.

In this case, whenever you search, whenever you need to find a certain inode for a given file name, every time it needs to scan the directory block to find a matching filename. Well, if there are 10 files or 15 files, linearly scanning the 10 elements or 15 elements may be reasonable. But what if a directory contains like 300,000 files in a directory? Then if it is not sorted, or if it is not organized to a certain search structure such as radix tree, B+ tree, or red-black tree, then linearly scanning the array of 300,000 elements requires a substantial amount of search. It takes very long.

Okay. Anyway, in Pintos, the directory is an array of directory entries, and directory entries are not sorted.

## Free Map and Struct File

Okay. The next data structure is block bitmap. The block bitmap represents whether a given block in the file-system partition is being used or not.

The data structure name is free map. It is a bitmap to represent the status of the blocks in the file-system partition. The bitmap is stored as a file, which means that it has its own inode. It has two fields: the number of disk blocks in the entire file system as bit count, and the actual bit array.

There is another important data structure, `struct file`. It is created when a file is opened. Remember, for a closed file, this data structure is not created.

So this `struct file` is allocated and created when a file is opened. It contains a pointer to the inode, and the most important attribute of `struct file` is position. It represents the position of a file where the read and write operation should apply. It has a field to indicate whether a file is writable or not.

## Three Things in Project 4

So in this project, we have to do basically three things. The first one is implement buffer cache, the second one is make the file-system file abstraction indexed and extensible, and the third is implement the subdirectories.

For buffer cache, the purpose of buffer cache is using part of memory as a disk. It is the opposite concept of virtual memory. In virtual memory, we are using part of disk as memory. Now, in buffer cache, it is the opposite. We are using part of memory as disk.

Okay. So for buffer cache, we allocate buffer cache. We are going to allocate physical pages to accommodate 64 disk blocks. We are going to cache the data blocks to this buffer cache. When reading or writing data blocks, we are going to save it to the buffer cache, and once we are done with accessing the block, sometimes we have to save the modified data blocks back to the disk space, or when the file system shuts down.

For the file, current Pintos represents a file as a single extent, as a single consecutive block, which contains the start address and size. So in Pintos inode, it contains two fields: the start address of the file blocks and size.

But what if we want to extend the block, we want to extend a file, but the next location right next to the file is already occupied by another file, file B for example? Then there is no way for the Pintos file system to extend this file. Then we have to find free space, and we have to find a free space that can accommodate the extended file, and then we have to migrate the entire file. It will consume a huge amount of time for this copy.

So we are going to implement block pointers in an inode. There are a variety of ways to represent file blocks that belong to a file, but now we are going to use a UNIX-like file structure, and then we are going to implement hierarchical space for a file.
