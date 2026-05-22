# [Week05] Pintos Project4-3 Indexed and Extensible Files

Source: https://youtu.be/4Vg66wWwkXE?si=et5lvozGrkyBRALs

## Indexed File Motivation

I am going to explain how we can build an indexed file in the Pintos file system.

In original Pintos, the file size is fixed when it is created. It is somewhat very limited. So in this project, we will modify the Pintos file system to change the file size dynamically. The maximum file size will be 8 megabytes. This is what you are going to modify.

This is how we allocate the block when Pintos creates a file. When creating a file, say file A, it saves the start block address and length to the inode. Then when you create another file, again you save the start block address and length to the inode. So this is disk. This is disk.

But in file system, we perform append very frequently. We append a block to the end of this new file, append another file, and append another file.

Okay. But in the Pintos file system, we cannot do that in the current file system structure, because a file is represented by the start address and the size. This means that all the blocks associated with the file should be placed at the consecutive region on the disk. Consecutive region.

However, in this configuration, there is file A. If you want to extend a file, but the next address has been occupied already, then we cannot extend the file. So we have to fundamentally change the way the inode represents a file.

After modification, after what you are going to modify in the file structure in the Pintos operating system, how? Let me show you.

After modification, an inode is going to have a bunch of pointers, and each pointer will point to the data blocks. At the creation of the first file, you allocate 1, 2, 3, 4, 5, 6, 7, 8 blocks with 8 pointers. At the creation of the next file, file B, you create another file with 8 blocks, and there are 8 pointers.

If you want to expand file A, then you allocate another block wherever there is a free block, and set the ninth pointer to point to the newly allocated block. Then we can extend a file. So that is what you are required to do.

## Seek Beyond End of File

Okay. Let me go into the detail a little bit.

The seek operation changes the current offset of the file. Let's consider this case. This is the size of the current file. Let's say this is 512 bytes times 8 blocks. This is 4 kilobytes. So the current file size is 4 kilobytes.

But the application calls the seek system call and sets it to 10 kilobytes, for example. Then current offset is 4 kilobytes. I am sorry, I think it is too large. Let's say it is 6 kilobytes. Okay. Then the current offset pointer is updated to the position where there are no file blocks allocated.

Okay. So when you modify the file system operation, and when you modify the file to the indexed organization, you have to check, you have to modify the seek system call also. When the seek actually seeks beyond the size of the file, it does not change the file size, nor does it allocate the blocks. It just updates the offset.

When a write is called at this position, this block is still free and it has not been allocated to any of the files. But the offset is updated to this point. At this point, if your application calls the write system call, then it places contents at this point and initializes the intermediate blocks that have not been accessed anyway with some initial value.

This is called a hole, and the hole is initialized with the value 0. So this is a very interesting part of the seek operation.

## Three Things to Do

There are three things to do.

First, you have to modify the on-disk inode structure. Then you have to modify the code that uses on-disk inode. That includes changing file offset to the block address, creating a new inode, and deleting an inode.

Also, you have to modify and create the function that handles extension of a file.

## Modifying `inode_disk`

Okay. This is `inode_disk` in Pintos. This is sector start address, and this is length of the file. In current Pintos, the file block, a file, is represented by the start address and length. So we are going to modify that.

We are going to modify the on-disk inode structure for the file so that we can extend file.

This is a sample inode. There are some number of direct index, an indirect block, and then double indirect block.

In the direct block, there are direct block entries, number of direct pointers, that directly point to data blocks. Then indirect block points to the pointer block that points to the data block. The third one is double indirect block. It points to the pointer block that points to another pointer block, and then that points to the data blocks. So this is two-level indirection.

Okay. This is the layout of the actual file that belongs to a single file.

In Pintos, each inode occupies a single block. So the inode can be very large. We do not want to waste any of the space. For that reason, we are going to allocate as many direct pointers as possible.

Here there are from 0 to 124. This is just an example inode structure, a sample inode. If you want to use a different idea for the file organization, you are welcome to use your own idea and implement it.

Anyhow, there are 124 direct blocks, and then there is single indirect, and there is another double indirect. This is the total number of pointers in the inode.

Okay. In the new inode structure, there is file length, and there is magic number, and then there are 126 pointers. This is a total of 126 pointers.

Here, from here to here, there are 128 integers, and each integer is 4 bytes. So 128 integers times 4 bytes is 512 bytes. Here a single inode, a single on-disk inode, occupies an entire sector.

## Computing Sector from File Offset

The next thing to do is compute the sector number from the file offset. We should be able to compute the sector number from the file offset.

Pintos already defines a function returning `block_sector_t`. This is the return value type, and this is `byte_to_sector`. It takes an offset and returns the sector number associated with that offset. We have to change that.

This is the `byte_to_sector` function. It converts the position to a sector number. You have to change this function properly.

Another change is the code for creating an inode. There is a function called `inode_create`. In `inode_create`, the function is supplied with two parameters. The first one is sector, and there is length. Sector is the location where the inode should be created, and length is the size of the file at the time it is created.

Originally Pintos allocates contiguous blocks and saves its start address. However, here, now we modify the code so that the block addresses are all allocated. It is simple.

## Deleting an Inode

Deleting an inode should be changed slightly. When we delete an inode, we have to add block deallocating code at `inode_close`.

This is inode, and then there are a bunch of pointers, and then there are blocks associated with it. When you delete an inode, you have to deallocate all these blocks. That means that you have to set the free block bitmap for these data blocks to zero.

## Extending a File

Now it is time to handle an extension of a file. When the file size changes, allocate a new block and update a data block pointer in the inode. Then you have to fill the allocated blocks with zero. I think that is the basic steps you have to do.

There is function `inode_write_at`. Of course, we have taught locks. You have to acquire appropriate locks. When you write beyond the size of the existing file, then you have to update some field.

## Next Topic: Subdirectories

The next topic is subdirectory. Original Pintos has only a root directory, but not the other subdirectories. So we will implement the concept of subdirectory feature to make the file system more reasonable, and you will bring in the concept of hierarchical tree structure.

This is the list of files you need to modify in the course of creating the subdirectory concept in the Pintos file system.

Okay. This is the structure of the directory in original Pintos. There is only root directory. There is only root directory, so directory is flat. Root directory, of course, has inode. Then inode has the associated data block, and the data block has a directory entry.

Every directory entry in Pintos file system is just a file. File means it is a file name and inode number pair. Based upon these, it allocates a file inode, and then it contains a data block associated with the inode.

Now we are going to change the concept of this flat directory structure into a hierarchical directory structure. Here, the root directory can contain a regular file as well as another directory. That is the important part.

Okay. So a directory contains a regular file as well as a directory file.

The important thing is that, in a hierarchical directory structure, there are two pre-allocated directory entries in every directory structure. One is current directory, and then parent directory.

Let's look at the details. Let's say this is root directory, and then there is inode. Then inode points to the data blocks of the directory, and each of these are directory entries. Directory entries may refer to the normal file, or may refer to another directory tree.

In directory entries, the first two directory entries are reserved for the special purpose: current directory and parent directory. However, for the root directory, there is no parent directory, so it points to itself. These are the data structures you are going to implement in realizing the hierarchical directory structure.
