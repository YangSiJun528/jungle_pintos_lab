# [Week05] Pintos Project4-1 File System Details

Source: https://youtu.be/mCAKZvZ1brs?si=wcu5zaZdeR6ICy6y

## Block Device

Hello. This lab is about the file system details in Pintos.

A block device refers to storage like SSD, hard disk drive, and such and so forth. A block device consists of blocks. It is a set of blocks, a linear array of blocks, and each block has its number. We call it a logical block address, LBA.

In some context, each block refers to the 512-byte sector, or in some context it refers to the 4-kilobyte file system block. So you have to check what LBA refers to on each occasion.

This is how we represent a block device in the Pintos operating system. This is a data structure. The name of the data structure is `block`. The most important attribute for the block is probably the size, the size of this in terms of sectors. Then Pintos allocates a 16-character-length name as an attribute for the block device. Then there is a block device type, and then there is a pointer to the device driver which contains a set of operations defined for the specific block device.

Pintos defines a read count and write count for a block device, which refers to the number of sectors read and sectors written. So this is the block device.

Formatting a file system is the process of writing some contents over each of these blocks.

## Pintos File System Layout

This is the layout of the Pintos file system. This is block 0, block 1, block 2, block 3, block 4, and such and so forth.

Here in Pintos, each block is 512 bytes. The file system block size is determined by the file system. You can format a file system with 4-kilobyte blocks, or you can format a file system with 32-kilobyte blocks, or even you can format a file system with 512-byte blocks. Here in Pintos, there is 512 bytes as a sector size as well as file system block size.

This is a basic sample layout of the file system in Pintos. In the first block there is an inode for the bitmap. The bitmap is a data structure which determines whether each of the blocks is being used or not. The second block is occupied by the inode for the root directory. Then there come four blocks, each representing a bitmap for this file system partition. Then there is the root directory. Of course, I am going to explain each of these data structures in detail as we go on.

Creating the inode for the bitmap, creating the inode for the root directory, creating and initializing the bit stream, creating the directory, and initializing and saving the initial value of this data structure is called formatting a file system.

So basically, formatting a file system is, from a clean block device, writing initial values for the essential data structures that are required to operate the file system.

## Bitmap Size

Let's look at a bitmap size here. We allocate four sectors for blocks, and each block corresponds to a sector. Each sector is 512 bytes, so we are allocating 2048 bytes for a bitmap, and it corresponds to 16,384 bits.

Okay. So in the bitmap there are 16,384 bits, and each bit represents whether a given sector is free or being used. So if you multiply the number of bits in the bitmap by the sector size, then we get the file system size. In this file system partition, you have 8 megabytes.

If you have four sectors for a bitmap, then that can cover an 8-megabyte file system partition. If you double the bitmap to eight, then of course it is going to cover a 16-megabyte file system partition. So this is how it works.

## Formatting Steps

From now on, I am going to explain how each of these fields are occupied in the initial file system boot stage.

In formatting a file system, Pintos basically creates three steps.

First, create and initialize how each of these sectors in the file system partition is being used. Of course, most of the sectors are free at the file-system formatting phase.

After creating a bitmap, you regard a bitmap as a file. The bitmap is also a file, so you create an inode for a bitmap also.

The basic data structure for the file system is the root directory, so you have to also create a root directory. Of course, after creating an inode for a bitmap, you have to write it to the disk, write the bitmap to the disk, to synchronize the file-system state to the disk safely.

There are three steps. Create and initialize the bitmap, first step. In the second step, create the inode for the bitmap and write its data on the disk. The third step is create the inode of a root directory. This is first step, second step, and third step.

Okay. So in the course of formatting a file system, now you write from sector 0. I am using sector and block interchangeably, especially in the Pintos file system, because in the Pintos file system each block corresponds to a sector.

So formatting a file system in Pintos corresponds to first writing and initializing the first six blocks in the file system: inode block for bitmap, inode block for root directory, and initializing the inode bitmap. This part is tricky. At the beginning, when the first file system is first created, the empty directory for the root directory is created, but it is not going to have any contents inside its directory. Let's see how it happens.

## `filesys_init()`

This is the code for initializing the file system. It is in `filesys.c`. The name of the function is `filesys_init`.

It contains three basic steps: initializing the inode, initializing the bitmap, and performing format. What does it correspond to? We are going to look at the details.

The first function, `inode_init`, creates and initializes the data structure for open files. When the system first boots up, there are two basic worlds in a computer system. The first one is memory, and the second one is disk.

When the system starts, which we call system boot, you create an empty list that points to the list of open files. Inode is the data structure that represents a file.

Also, in `free_map_init`, you have to create a bitmap that can cover 8 megabytes, and you have to initialize it properly.

Formatting is writing this data structure to the disk properly. Formatting here has two things. The first one is create and write the bitmap file, and the second part of formatting is create the root directory. There are two important things.

So remember: initialize the inode, initialize the bitmap, and then perform format. Format is the process of writing the bitmap to the disk, the process of creating the root directory on the disk, and creating the file consists of two parts. The first one is creating the inode, and the second one is creating the data blocks associated with the inode.

Okay. Let's move on. This is the process of initializing the file system. We call it format.

As we saw before, the first two important parts are create the empty inode list for open files, and second, create the bitmap, and then perform format. Format consists of creating the inode of the bitmap file and saving it to the disk, to save the disk for crash recovery, and then creating the root directory. There are two things.

## Open Inode List

Okay. Let's look at the actual code. The first thing we have to do is initialize the list of open inodes. It corresponds to initializing the list of in-memory inodes.

The `inode_init` function calls `list_init`. This is a typical function that initializes the empty inode list at the beginning. It is `open_inodes`. This is an essential function to create an empty list.

These are in Pintos. As you see, it maintains a set of inodes for the opened files as a linked list.

My question is, what is the data structure that represents open files in a V6 operating system? Probably Pintos and xv6, or more widely, operating systems for operating system education, are using slightly different data structures for representing each of the concepts in the operating system.

In the class, in the textbook, we are using xv6 code, and in the lab we are dealing with Pintos. If you are interested in different ways of implementing operating system concepts, you may want to read both of the codes for comparison purpose. So my question is, what is the data structure that represents open files in xv6? Please check it out.

## Creating and Initializing the Bitmap

The first step is creating and initializing the bitmap. First, you create the bitmap of block size. Here the file system is 8 megabytes as we saw before.

At the very beginning, we need one inode for bitmap and the other inode for a root directory. So we have to mark those bitmap entries as being used. We are going to allocate the inode for the bitmap at sector 0, and we are going to allocate the inode for the root directory at sector 1. So you have to mark the associated entries in the bitmap.

This is the location we are going to mark. This is the location we are going to mark also for root directory. This is the pointer to the array of bitmaps.

In the initial line of the file system, there are three steps: creating the inode of bitmap, and of course after you are done with initializing bitmap array, you have to write them to the disk for synchronization purpose, and then you have to create the root directory. Write the contents of the bitmap to the disk, and create the inode of the root directory to the disk.

Okay. So in this function there are two steps: create the free map file, and second, create the root directory. There are two things you have to do. The first one is creating the bitmap file, and second one is creating the root directory.

Especially for the root directory, the second parameter is the number of entries in the directory, number of entries in the directory. Here you are initializing the root directory with maximum 16 entries. This is the location of the inode where you are going to place the inode for the root directory.

## Creating the Free Map File

The first step: create and save the bitmap file. You create the inode and write it to the disk. This is actual code for creating a bitmap.

You have to first create the inode for the bitmap. The first parameter is the location of the inode where you want to create the inode for a given file. Then there are three functions you want to pay attention to: `inode_create`, `file_open`, and `bitmap_write`.

After creating the inode for a bitmap file, you bring that inode to memory and save the contents of the bitmap to the disk. These are three basic functions for creating a free map.

## Creating the Root Directory

The next thing is creating a root directory. Creating a root directory corresponds to creating an inode for the root directory. So creating an inode at this location, the location of inode, and then the size of the directory entries.

Again, `inode_create` accepts two parameters. For root directory, this is the block number which has the inode for the root directory, and entry count is the maximum number of entries in the root directory. So you specify it as 16 in the code.

The second thing is, after you are creating an inode, of course every directory needs a place to save the directory entries other than the inode. So you have to allocate data blocks for the root directory, and you have to save the start address of the data block at the inode.

There are two functions. `bytes_to_sectors` is a simple function that translates the number of bytes into the number of sectors, and then `free_map_allocate` allocates a certain amount of data blocks from the bitmap. It allocates contiguous blocks.

For example, if you want to allocate 16 sectors, then what you have to do is scan the free bitmap and look for the consecutive bits array where all these bits are free. This is 16-bit array, for example. You find that this block is occupied, occupied, occupied, or maybe it is not occupied, it is occupied. Of course this bit is not occupied, but because the next bit is being used, even though this third bit is free, because the fourth bit is being used, you cannot allocate consecutive 16 bits.

If you find that consecutive 16 bits are free, then you allocate consecutive 16 bits and corresponding blocks, and then return the start address of the corresponding sectors. That is what `free_map_allocate` is used for.

Then you have to save the starting address to the associated inode, the starting point. Then you write the disk inode to the disk safely.

One thing you have to make sure is that memory, we call it DRAM, is volatile. It means that if the power goes off, then you lose all the contents. Storage, usually either disk, is non-volatile. If you save contents to the disk, you can have it even though the power goes out.

But every operating system operation deals with the memory data structure. So you have to be careful. If the data structure needs to be saved carefully and saved safely even with the existence of power crash, then you have to make sure that every data structure needs to be stored at the disk safely when the operation completes. So you write the disk inode to the disk.

Okay. After you are writing the bitmap contents to the disk, then you close inode. After closing the inode, you deallocate and remove the in-memory inode from the open inode list and save it to the disk.

The next step to do is to load the bitmap to memory. So instead, read the bitmap contents on the disk from the disk to memory. It consists of two phases. First one is open the file, and then read it.

## Creating a File

Now we are going to explain the details of creating a file.

Creating a file is done by `filesys_create`. This is the driver function of the system call `create`. By looking at the details of the code, you are going to learn how the file system creates a file.

Creating a file consists of creating the inode. Let's think about what kind of data structures we have to modify in creating a file in the file system.

Let's say we are going to create a file. Then we have to create the inode for the new file. Of course, you may want to initialize the data blocks associated with the given file.

Then you have to modify the parent directory which the newly created file belongs to. For example, let's say the name of the file is `a.c`. Then `a.c` contains a pair, which is file name and inode number.

Okay. So this is data, this is the new file data block, and the associated inode. Then there is the directory block. Is that all? Actually there is one more thing. There is a bitmap in the file system.

We have four sectors allocated for the bitmap in the file system. Let's say this is the first sector, second sector, and the third sector. Because you have allocated two blocks, one sector for inode and one sector for data block in this specific example, you may have updated some part of this bitmap. Let's say you have updated two bits in the third part.

Okay. Then in the course of creating a file, you have updated at least four blocks: the first block, second block, third block, and fourth block. One, two, three, and four. So in the course of creating a file, we have updated and modified four blocks. We have to synchronize all these blocks to the disk.

As you can see, creating a file is a non-trivial exercise.

The details of the file system are create and initialize the inode, write it to the disk, and then add a new entry in the root directory. Okay. These are the simplified steps of creating a file. In this example, they did not allocate any data blocks, and it does not show how the bitmap is allocated and updated, but we are going to show how it works.

## `filesys_create("testfile")`

These are the detailed steps of creating a file. The name of the file is `testfile`. So we call our function `filesys_create("testfile")`.

For the very first step, it reads the inode of the root directory from the disk to memory and inserts it to the open inode list.

The second step is write the inode of the test file to the disk, create inode.

Then read the entries on the root directory, add a new directory entry for the test file, and then write the entries of the root directory. Okay. So this consists of five steps.

Let me ask a question. Among these five steps, it only shows the steps of modifying and updating the inode block and directory block. The directory block is updated, but the inode block is newly allocated. So in the course of allocating a new inode, new block for inode, we have to update bitmap. We have to find a free block and mark the associated bitmap as marked.

Then where among the five steps do we have to include the process of updating the bitmap? Let's say this is 1.5, 2.5, 3.5, 4.5, 5.25, and 0.5. Among these possible six positions, where do we have to include the process of finding the free entry in the bitmap? Think about it.

Of course, the answer is going to show up shortly.

## `filesys_create()` Code

Okay. Let's move on to the next page. Of course, there comes an answer right after.

We first have to open the root directory. Creating a file consists of creating an initialized inode and adding a new directory entry to the root directory. In the current Pintos file system, here we do not have a hierarchical directory structure. We only have root directory.

So open root directory, and if the root directory is successfully opened, then we allocate a free block.

This is the number of sectors we want to allocate. This is number of sectors, number of sectors you want to allocate. In the course of allocating a new block, you insert the start address of those sectors into the specified inode address.

Okay. `free_map_allocate` here specifically allocates one sector, as specified here, from the free map, and then saves the start address at the inode sector. This is how you create the sector.

In the course of calling `free_map_allocate`, inside this function it automatically updates the free bitmap array. Then of course, in `inode_create`, in the course of creating an inode, it initializes an inode with initial size bytes and writes it on the disk. Then it adds the created directory entry to `dir`, specified as the parent directory of the just-created file.

Okay. So this is how we create a file.

## Opening an Inode

Now let me explain the procedure of opening an inode for the first time.

The function we are concerned with is `inode_open`. It reads the on-disk inode. This is the first parameter. It reads the inode from the disk at the sector location, and then creates its pointer.

Of course, the important operation of `inode_open` is inserting the read inode to the linked list of open inode. Pintos maintains a list of open inodes in the system. It is a global linked list. In the course of opening an inode, you have to insert this inode to the list. So this is the process of inserting an inode to the open inode list.

Then you have to set the fields properly. There is one important field you may want to pay attention to. There is this open count. This is open count.

More than one process can open a file. If there are two processes, process 1 and process 2, and both of the files call a function `open("a.c")`, for example, then the same file is opened twice by different processes. In this case, this open count is set to 2. So this represents the number of processes that opened a given file.

Opening a directory is similar to opening a file, except that directory entries are opened instead of the inode itself. `dir_open` gets the inode pointer as its parameter. In opening a file, it allocates directory structure, reads this directory entry, and initializes the fields of the directory structure with the inode and the associated position.

## `free_map_allocate()`

The important function, the most essential function, is allocating a free block. This is the function `free_map_allocate`.

It accepts two parameters. The first one is count. The objective of `free_map_allocate` is find `cnt` consecutive blocks by scanning free blocks. So `cnt` stands for the number of blocks to allocate. As a result, the sector position `sectorp` specifies the start address of the blocks allocated.

In the course of allocating a free block, it sets the free bitmap. My question a few slides back was where the free block bitmap is updated in the course of allocating a new block. It is within `free_map_allocate`.

So it finds the consecutive false bitmap, or false bits. Bitmap is set with 0, and it sets them to true.

## Creating a Directory Entry

The next function is how we create a directory. A few slides before, we looked at the function that creates a file. Now we are looking at the function of how we create a directory.

What we would like to do is add a named file to the directory. The inode of the file then needs to be inserted as the `inode_sector` parameter. So it has three parameters: name, directory, and inode sector.

Directory is the target directory where the newly created file is located. Name is the name of the newly created file. Then the associated location of the inode for the newly created file.

The first thing you have to do is look up the directory and check whether a file under the given name already exists or not. If it exists, then the creation attempt should fail.

Then it has to scan the directory block and find the empty spots. Let's say this is directory block. For example, the first entry is `a.c`, second entry is `b.c`, third entry is empty, fourth entry is `d.c`. Then in the course of creating a directory entry, it first has to scan the entire directory block and find the first empty spot, and then create the block at this position.

What you may want to notice is scanning the directory block is very time-consuming. It is a very expensive operation. If you write an application that frequently creates a file, then your file system should be very, very efficient in creating a file. Every time when you create a file, you cannot linearly scan all the directory blocks to find an empty spot. It is going to be very, very expensive.

Anyway, you might want to use a more sophisticated data structure for creating a file, especially in finding empty spots in the directory block.

Okay. So this is how we create the directory entry.

## Directory Lookup

Let me skip to the next slide. Now the directory lookup.

Directory lookup takes a directory pointer and it takes the name, and it is a directory entry pointer and offset. Lookup is a function, and it checks if the file name exists in the directory or not. Then it returns the address of the directory entry structure using the parameter it has supplied.

So the input is, for a given directory, find the name, and then return the address of the directory entry using the parameter supplied. This is lookup.

## Opening a File

The next function we are going to cover is opening a file. It is called `filesys_open`, and it gets the name. It opens a file system, and it returns the pointer to `struct file`.

`filesys_open` is called by the system call `open`, and it does two things.

First, it adds an inode to the open inode list. Second, it allocates and initializes `struct file` and returns its address.

For the first one, you have to first check if the file has already been opened by another open system call. In that case, you do not have to insert the same inode to the list twice. Instead, you just increase the reference count of the inode entry in the inode list.

However, you have to allocate and initialize a `struct file` structure every time when a file is opened, meaning every time `filesys_open` is called. So this is the process of allocating and opening a file.

This is the process of opening a file. Let's provide an example. The name of the file is `testfile`. `filesys_open("testfile")` consists of the following five steps.

First, you have to read the inode of the root directory first. Then you scan entries in the root directory and find the name `testfile`. You scan the directory entries and find `testfile`. Then you find the matching inode, and we find that the inode number for `testfile` is 7.

As a third step, you have to insert the in-memory inode of `testfile` to the inode list. So you are done with inserting an inode to the inode list.

Then as the next structure, you allocate a `struct file` data structure. Every time you open a file, we have to allocate a `struct file` data structure, set the address of in-memory inode in it, and return its address.

Of course, it is possible that the inode for `testfile` already exists in in-memory inodes. In that case, you do not have to read it again or insert it again.

This is the system call for opening a file. The most important task of opening a file is allocate and initialize `struct file` and return its address. So you return `file_open` for the inode. Find the inode number for a given file and call `file_open`.

## `file_open()`

This is an essential function, and it allocates an initialized `struct file` in memory.

The next function is directory lookup. Directory lookup is the easy function. It opens a directory, looks up the file under the name `name`, and returns the inode. That is what directory lookup is for.

Once you find the associated inode, it is time to open a file. You get the inode, you supply the inode number to the function `file_open`. The role of `file_open` is allocate the data structure for `struct file`, initialize it, and return its address.

So allocate `struct file` and initialize it. You fill it with some value and return the pointer, return the start address of the file.

What is the process of initializing the file structure? Initializing the `struct file` is right here. The file structure has a pointer to its inode. The most important attribute of file struct is the offset. Here we call it position. The position stands for the offset for a file where we apply the read and write operation. This is called offset, and in Pintos it is called `pos`, position.

When you blindly open a file, you have to set that deny-write field as false. But if this file is executable, or for some reason, for some special case, you may want to write this field as true.

Okay. So this is the important step of initializing `struct file` when you open the file: creating the inode and initializing the position. That is what opening a file is for.

## Removing a File

Let me provide the steps of removing a file. The matching system call is `filesys_remove`.

What it basically does is set the flag `removed` in inode to be true, and then remove the directory entry. Those are the two steps it needs to do. Set the flag and remove the directory entry. This is a flow.

It opens the root inode and searches the root directory for a given file. If it does not exist, then in that failure case it unlocks the root directory inode and returns false. It basically means the deletion fails.

If the file exists in the directory, then the directory entry becomes false. The `in_use` field of the directory entry is set to false, and the inode's removed flag will be set to true. Then it deallocates the in-memory inode for the root directory and returns.

This is the example of removing the test file. We call `filesys_remove`, we call `testfile`. Then it reads the inode of the root directory, scans the entries of root directory, finds the target file, sets `in_use` as false, and on the inode it sets the `removed` field to be true.

Let's look at the actual code. There are two things. First it removes the target file entry from the directory, and then it sets the removed flag in the inode to be true. So that is basically two things that it has to do.

First it sets the directory entry flag to false. That is the flag. The function `dir_remove` removes the entry of the target directory and then sets the removed flag in the in-memory inode to true, and then writes the updated directory entry on the disk.

This is the details of `dir_remove`. It gets two parameters. First one is target directory, and the second is name. This is the name of the file it wants to remove. The first step is search the directory and get the associated directory entry, mark it as false, and then write back the updated directory back to the disk. So `inode_write_at` updates the disk contents of the associated directory.

After removing the directory entry, it is time to remove the inode itself of the file. The first thing you have to do is open the inode, and then call a function called `inode_remove`.

But calling `inode_remove` does not mean that the inode is immediately deleted, because the inode might have been opened by another process, more than one process. So `inode_open` adds the in-memory inode to the open inode list, and then call `inode_remove`.

In `inode_remove`, the most important part is setting the `removed` flag in the in-memory inode to be true. Okay. That is it. It does not do anything other than setting the `removed` flag in the inode to be true. Nothing more than that.

After that, we call the function `inode_close`. In the course of calling `inode_close`, it does all the dirty works. It checks the reference count, and it checks if there are any other processes that refer to this inode. If all is completely free, then it deallocates the associated inode from the disk and makes the associated sectors free. That is the role of `inode_close`.

This is removing a file.

## Covered Interfaces

In this chapter, we have covered a few details of the file-system interfaces: formatting a file system, creating a file, creating a directory, directory lookup, opening a file, and removing a file. These are the basic steps of the file system in the Pintos file system. So you are done.
