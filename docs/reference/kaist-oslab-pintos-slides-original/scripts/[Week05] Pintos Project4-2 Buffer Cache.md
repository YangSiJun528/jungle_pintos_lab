# [Week05] Pintos Project4-2 Buffer Cache

Source: https://youtu.be/AydN8n7PCaY?si=6c5_Lk4gVNU_LYk_

## Buffer Cache Overview

In project number four, we are going to implement three things for the file system. The first one is buffer cache, the second one is make the files indexed and extensible, and the third one is implement the concept of subdirectories.

The first one I am going to explain is buffer cache.

The idea of buffer cache is simple. We use part of memory as the disk. This is the opposite concept of virtual memory and swap space.

Let's say this is memory and this is disk. Virtual memory is the concept of using part of disk as memory. It is something like this. Say this part is mapped to memory and this part is mapped to the disk, and this portion is called swap space. This is virtual memory.

In buffer cache, the concept is the opposite. Let's say this is buffer cache. We use part of the memory as the disk, something like this. Okay. So use part of the memory as a disk. That is the big concept of buffer cache. Part of the disk is mapped to the memory. That is buffer cache.

Buffer cache is consecutive physical pages, and it is initialized when the system starts or when the file system is mounted. So if you unmount the file system and mount it again, all the contents in the buffer cache are reset.

In current Pintos, there is no cache for disk I/O. In reality, however, most operating systems have a buffer cache for the disk I/O. So we are going to modify the file system so that we can cache the file blocks into physical memory.

In this project, you are going to allocate 64 file pages to be cached in memory.

## What to Implement

This is how you are going to implement it. Current Pintos accesses storage on every read/write request. So we have to change the user I/O to be performed through buffer cache, and we are going to add a buffer cache layer here.

This is before you implement the buffer cache, and this is after you implement the buffer cache.

This is the list of things to do. The first thing, of course, is define the data structure for buffer cache. Then you have to write the routine to allocate and initialize the buffer cache. Then you modify the read and write routine so that they utilize buffer cache in reading and writing the data.

Then of course you have to change the read/write routine so that, if cache miss occurs, if you want to read data blocks from the buffer cache but that block does not exist in the buffer cache, then you have to read it from the disk. Okay. So you have to implement a routine to handle cache miss.

There is another important routine you have to write. When you want to bring in a new block to the buffer cache, it is possible that buffer cache is already full, and you have to make a room for bringing in the new block. In that case, you have to evict one of the buffer cache entries and make it free. So you have to implement buffer cache replacement algorithm.

Then you have to write a routine to write the dirty buffer cache. It is possible that, after a block is read from the disk to the buffer cache, the application might have changed the contents of the buffer cache. In that case, you have to write the dirty page back to where it was read from. So you have to write a routine that synchronizes dirty buffer cache to the disk.

This is the list of things to do. It is very simple, and it is very fun.

## Buffer Cache Data Structure

All right. The first thing you have to do is define the data structure for the buffer cache.

Buffer cache is a page in memory. For using a page entry as a buffer cache, you have to define some kind of metadata structure that represents the contents in this page.

It has to contain a dirty flag. The dirty flag denotes whether a given content has been changed since it was read from the disk.

It has this type of flag in the heading indicating whether the entry is being used or not.

Of course, we have to introduce the access flag. It indicates whether the entry has been accessed recently or not. This access flag is used for the victim selection when the operating system has to select one of the buffer cache entries to make a room for the new one.

There has to be a mechanism to distinguish hot blocks from cold blocks. Hot blocks means the buffer cache entry that is being accessed actively, and cold buffer cache entry is the one that has not been used recently.

So we have to have some type of mechanism to precisely identify the hotness of the block. This is to let the operating system select the data blocks that have not been used recently.

It is important that, when you have to kick out one of the buffer cache entries, you have to choose the buffer cache entry that has not been used recently, so that the performance is not affected by kicking out those entries.

Also it has to have on-disk location, and then of course it has to have a virtual address of the associated cache entry.

We are going to allocate 64 pages for the buffer cache. So let's name the data structure as buffer head. We have to allocate 64 buffer heads when the system starts.

There are many data structures that can accommodate 64 data structures. We can use array, list, or hash table. Here we are using a table. This is the table of 64 buffer heads, and each buffer head has a pointer to its actual location in virtual memory.

This is the diagram for the buffer cache. The act of reading a disk block from the disk to buffer cache is called caching. This is caching.

When some of the buffer cache is updated after it has been read from the disk, then you have to synchronize the contents of the buffer cache back to the place where it belongs. That is called flush.

Okay. This is the entire diagram for the buffer cache data structure, and this is the one you have to implement.

When an application program wants to read a certain block, it has to search the buffer cache table first to identify the location of the buffer cache.

Okay. The next step is, I pretty much have explained everything for you to be able to define the data structure for the buffer head. So you should not have any difficulty in implementing the buffer head data structure.

## Allocating and Initializing Buffer Cache

The second issue is allocating and initializing the buffer cache. Of course, this has to be done at system boot time and when the file system is mounted.

We need a space for 64 file system blocks. The block size is 512 bytes, so we need 32 kilobytes to accommodate 64 file blocks. Of course, we also have to allocate memory for buffer heads, 64 buffer heads. This is size of buffer head times 64. This is the amount of memory you have to allocate for defining 64 buffer entries.

This is done at file system initialization time. So you have that code here. That is pretty much easy.

Okay. That is easy.

## Current Pintos Read

Now let me explain read and write in current Pintos. This is the function `file_read`, and it calls a function called `inode_read_at`.

The read system call takes parameters, of course file descriptor and the amount of data blocks to read, and then the operating system reads a certain amount of data blocks from the disk to memory.

In current Pintos, this is how read is implemented. There is a variable called size. Every time it reads from the disk, it reads 512 bytes. So it iterates this loop, reads and reads and reads, until it reads the amount of data that has been specified as a parameter to the `file_read` operation. Each time, the size decreases by 512 bytes. Eventually the size becomes zero or negative.

Okay. So read the full sector, and then `block_read` from the disk, and compute the remaining read size. If the size of the read is still greater than 0, then it reads the full sector again.

But it is possible that, for example, you want to read 1500 bytes. Then it reads 512 bytes first, and then it reads 512 bytes again. Then it sums up to 1024 bytes. The remaining part, 476 bytes, needs to be read from the disk, but it is not a single block. Right? It is not a single block. So you should not call `block_read` from the disk for only that part.

Then you allocate a bounce buffer and perform partial read. Partial read means, even though you read the entire block from the disk, which is 512 bytes, the application has asked you to read only 476 bytes from the disk. So read 512 bytes to the bounce buffer, and then copy only 476 bytes from the bounce buffer to the buffer designated by the read. This is 476 bytes. This is how you perform partial read.

## Reading Through Buffer Cache

Okay. Now you have to implement this part. Of course the entire diagram looks a lot more complicated, but do not be afraid. It is easy and you should be able to do it very quickly.

After calling the function `inode_read_at`, instead of reading directly from the disk, you read the buffer cache. So you perform read in the buffer cache.

The first thing you have to do is find the buffer head. If it does exist, then read data from the buffer cache to the buffer, and then update the buffer head. Here what is updated is probably the access bit, access flag, denoting that this buffer cache has just been accessed. Okay. That is the lucky part of the story.

But life does not always go as easy as you want.

The first issue is, what if the entry does not exist? Then you have to bring in, you have to read the disk block from the disk to the buffer cache and perform the read again.

In that case, if the entry does not exist, then you have to bring in the disk block from the disk to the buffer cache. If the cache is not full, then that is better. You can just read the blocks from the disk to the buffer cache and read it again. Select empty entry from the buffer head, bring in the disk blocks, and then perform read. That is good.

But the worst part is the case when the buffer cache is entirely full.

If the cache is full, then select victim entry. It has to select the victim to kick out and to make a room for bringing the new disk blocks. Select the victim entry. If the victim entry has been modified since it was brought into memory from the disk, then we say it is dirty. Then we have to save it to the disk. We have to flush the victim entry to the disk.

After flushing that buffer cache entry to the disk, it has to release the victim entry and make it free. If the victim entry is not dirty, then it does not have to perform flush. It goes directly to releasing the victim entry.

Again, because now the free block is available in the buffer cache entry, the operating system can perform read from the disk to memory.

Okay. So we have to modify disk read to buffer cache read. The function we have to change is `inode_read_at`. We have to modify `block_read`. When reading a file, we modify read to read the data from the buffer cache, not from the disk. Okay. So you have to modify this read part.

## Current Pintos Write

The write has to be changed in similar fashion. Write is the same. It iterates in the while loop. If you want to write `n` bytes, then each time it iterates, the Pintos operating system writes one block at a time. Each time it performs write, it decreases this size by 512 bytes. Then it calculates the remaining write size, does it again and again and again.

If the size becomes zero, then the write is done, so it releases the bounce buffer. But in case the remaining amount of data to write is less than a sector, this is the important and tricky part. We call it a partial write.

Actually partial write takes more time than a blind write. We call it a blind write. Blind write means write full sector, so you do not have to read the disk block from the disk. When you write an entire sector, you can just write it to the disk.

If you want to write the full block, then write it to the disk. But if you want to write part of the block, then what you have to do is first read the contents from the disk to memory. Then fill the memory part with the data blocks you want to write, then write it to the disk.

So partial write is more expensive. Allocate the bounce buffer. You have to read the data blocks from the disk to the bounce buffer, then perform partial write to the bounce buffer, and then write the bounce buffer to the disk. So partial write is more expensive.

## Writing Through Buffer Cache

Writing with buffer cache is similar. You have to write in the buffer cache.

Find the buffer head. If those blocks already exist in the buffer cache, if they exist, then you are lucky. Then update the buffer cache, and you are done.

However, if the entry does not exist, then you have to write that to the buffer cache. In that case, if the cache is not full, you are lucky. You find the empty buffer head, and then write the block, not to the disk, to the buffer cache, and return.

Okay. There is an interesting part. `block_read` data from the disk to the buffer cache here, this part. This `block_read` here is required only for the partial write. If you perform full block write, or if you perform blind write, you do not have to read the disk block from the buffer cache.

If the cache is full, then you have to select the victim entry. If it is dirty, then you have to flush the victim entry to the disk. This is the same as before, and then release this block. If it is not dirty, then release the victim entry from the buffer head.

In modifying write, you have to modify disk write to write to the buffer cache. Write to the buffer cache. When writing your file, modify it to write the data to buffer cache rather than to the disk. This is what you are supposed to do, and you are supposed to modify this part, `block_write`.

Okay. That is going to be easy. So you are done. Good.

## Synchronizing Dirty Buffer Cache Entries

One of the important things that remains is synchronizing the dirty buffer cache entry.

When you release a buffer cache entry to accommodate a newly incoming disk block, you have to check whether the existing buffer cache content is dirty or not. If the existing buffer cache entry is dirty, then you have to write it to the disk. We call it a synchronization activity. Synchronize.

Write dirty buffer cache entries. This happens when the buffer cache entry is evicted first, or if the file system is unmounted. Shutdown means unmounted. There is a function called `filesys_done`, and then you have to write code here. Okay. That is going to be easy, so you should be able to do that.

There is another situation where you want to write the dirty buffer cache entry to the disk: periodically. For example, in a five-second interval. In this case, you use timer interrupt.

## File Read and File Write Path

Okay. Let me explain the details of read/write in Pintos.

Reading a file is performed by the `file_read` function. It contains three parameters. This is the pointer to the file object, this is the pointer to the memory area which you want to read the data blocks into, and this is the amount of data blocks you want to read, size.

The data structure `struct file` contains a pointer to the in-core or in-memory inode. In in-memory inode, Pintos maintains the linked list of inodes that have been opened by the operating system. There is an inode list here, and then each inode takes this form. In in-memory inode, it contains the structure inode, and inode disk contains a start address and size of the file. So looking at inode disk, you should be able to find the location of the data blocks.

The second one is write in current Pintos. Let me go back. It performs write. Yeah, this is the data blocks we have covered in the previous slide, so let me skip this.

`file_write` gets three parameters. First parameter is file data structure, buffer, and size. In `file_write`, it calls the function `inode_write_at`, and it gets parameters. The first parameter is inode, the second parameter is the address of the buffer, the third parameter is size, and the fourth parameter is the current offset.

It passes the position from which you have to write the file to. Finally you write the data blocks too, and right after writing some contents you have to update the file position. Then you return the amount of data that has been written to the file.

## `inode_write_at()`

This is the details of the important function `inode_write_at`. It takes four parameters: inode, buffer, size, and offset. Here this is file and this is offset.

In `inode_write_at`, you write size amount of data. This is buffer. So you write this size here. This is how `inode_write_at` works.

Let's look at the details of the write function. Let's say this is a file, and you want to write from this position to this amount. Let's say this is three and four blocks. Let's say this is block 1, block 2, block 3, block 4, and block 5, but in block 5 you only write half of the block.

This is file, this is beginning of the file, this is offset, this is start address of the file. Then when you write a file, the first thing you have to do is change the offset to the actual location on the disk. This is the function which translates the offset into the real sector address.

Then you have to check whether the location where you want to write the data block is aligned with the sector address. You have to make sure that in writing to the disk, all the disk blocks should start with the starting of the sector address.

If the sector offset is zero and the size of the chunk you want to write is the sector size, then you are happy and then you can perform block write.

Let's assume that you want to write data blocks, and the data block starts with the sector address. Then you should perform full block write repeatedly until the size is greater than zero. But at some point, if the chunk size is less than block size, then you have to perform partial write.

Partial write means that, when you are writing part of the block, you need a bounce buffer. You first have to read the data blocks from the disk to the bounce buffer. Then you have to read this block from the disk to memory, and then merge the contents of the bounce buffer with the fractional blocks you want to write.

So there is disk, and you want to write part of the block. This is the contents you want to write. This is bounce buffer, and this is the block you want to write. Then you read the data block from the disk to the bounce buffer, and then copy this portion to the bounce buffer, updating part of the contents in the bounce buffer. This is first step, and this is second step. Then write the entire contents of the bounce buffer to the disk. This is how you update the disk block.

That is easy. So you are done again. We are happy. We are happy hackers. Happy hackers. Good.

## `byte_to_sector()`

All right. This is the function called `byte_to_sector`. It gets the inode and it gets the position, and then it returns the sector address of the given position.

This is very simple. Inode has a start address of the file. Then we have a position. If the position is here, then you divide it by the sector size. Position here is a position within a file. Of course the position is bytes. We divide it by the sector. It means how many sectors it is apart from the beginning of the file. Then you add it to the start sector number of the file, and then you will get the sector address of the block you want to write.

That is easy.

## `file_read()` and `inode_read_at()`

Next, we are going to explain read. This is the read of the current Pintos. It reads from the disk, and if it is partial read, then allocate bounce buffer and read part of that.

This is the same code. This is the code for `file_read`. It gets three parameters. First one is pointer to the file. This is pointer to the memory chunk, and this memory chunk is the place where the operating system reads the data blocks from the disk to this buffer. Then the amount of data blocks you want to read is size. So it contains three parameters: file, buffer, and size.

This is file, and it contains pointer to inode data structure. After reading a file, the file position is updated.

This is the last slide of the explanation on the buffer cache. The function is `inode_read_at`. It reads the given file. It takes four parameters: inode, buffer, size, and offset.

The inode represents the file you want to read, and the buffer is the start of the memory address you want to read the file to. This is amount of data blocks you want to read, and this is the position of the file where you will start reading.

Similar to `inode_write_at`, it iterates, and for each iteration it reads a block. Each time it performs read, it performs a read on the block device, and each time you provide a sector index. So this is the sector you want to read. After reading the full block, you will perform the partial read at the end of `inode_read_at`.

This is the end of the explanation, and I hope you enjoy the project. Good luck. We are happy hackers.
