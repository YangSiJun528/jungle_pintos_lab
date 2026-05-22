# [Week05] Pintos Project4-2 Buffer Cache

Source: https://youtu.be/AydN8n7PCaY?si=6c5_Lk4gVNU_LYk_

## Buffer Cache

The first Project 4 feature is the buffer cache.

The idea of the buffer cache is to use part of memory as a cache for disk blocks. It is the opposite direction of virtual memory. Virtual memory can use part of disk as an extension of memory, while the buffer cache uses part of memory as an extension of disk access.

In the original Pintos file system, file reads and writes go directly to the block device. After adding the buffer cache, file-system reads and writes should go through the cache layer.

Project 4 requires a cache for 64 file-system blocks.

## Tasks

There are several things to implement.

First, define the data structure for buffer cache entries.

Second, allocate and initialize the buffer cache when the file system starts.

Third, modify read and write routines so they use the buffer cache instead of calling the disk directly.

Fourth, handle cache misses. If a requested disk block is not in the cache, Pintos must bring it from disk into the cache.

Fifth, implement a replacement algorithm. If the cache is full, Pintos must evict one cache entry to make room for the new block.

Sixth, write dirty cache entries back to disk when they are evicted, when the file system shuts down, or periodically.

## Buffer Cache Entry

A buffer cache entry needs both data and metadata.

The data area stores one cached disk sector. The metadata describes what that data area currently contains.

Useful metadata includes a dirty flag, an in-use or valid flag, an accessed flag, the on-disk sector number, and a pointer to the memory area that stores the cached sector.

The dirty flag means the cached contents have been modified after being read from disk. Dirty entries must be written back before they are discarded.

The accessed flag records whether the entry was recently used. A replacement algorithm can use this flag to avoid evicting hot entries and prefer entries that have not been used recently.

## Allocation and Initialization

The cache has 64 entries. Since each Pintos file-system block is 512 bytes, the data area for 64 cached blocks needs 32 KB of memory.

The system also needs metadata for 64 buffer cache entries.

These structures are initialized when the file system is initialized. If the file system is unmounted and mounted again, the buffer cache state is reset.

## Original Read Path

The original read path starts from `file_read()`. It calls `inode_read_at()`, passing the inode, the destination buffer, the size, and the current file position.

`inode_read_at()` loops over the file data sector by sector. For each sector, it converts the file offset into a disk sector using `byte_to_sector()`.

If the read covers a full sector, the original code calls `block_read()` directly.

If the read covers only part of a sector, Pintos uses a bounce buffer. It reads the full sector into the bounce buffer and copies only the requested bytes into the caller's buffer.

## Reading Through the Buffer Cache

After adding the buffer cache, the file system should not call `block_read()` directly in normal file read paths. It should request the sector from the buffer cache.

On a cache hit, Pintos finds the cache entry, copies the requested bytes from the cached sector, and marks the entry as accessed.

On a cache miss, Pintos must bring the sector into the cache. If there is an empty cache entry, it can use that entry. If the cache is full, Pintos must select a victim entry.

If the victim entry is dirty, it must be flushed to disk before it is reused. Then the requested disk sector can be read into the cache entry, and the original read can proceed from memory.

## Original Write Path

The original write path starts from `file_write()`. It calls `inode_write_at()`, passing the inode, the source buffer, the size, and the current file position.

`inode_write_at()` also loops over sectors.

If the write covers a full sector, Pintos can write the whole sector directly.

If the write covers only part of a sector, Pintos must preserve the other bytes in that sector. Therefore, it reads the original sector into a bounce buffer, overwrites only the target range in the bounce buffer, and writes the full sector back to disk.

Partial writes are more expensive than full-sector writes because they require a read before the write.

## Writing Through the Buffer Cache

After adding the buffer cache, writes should update the cached sector rather than writing directly to disk.

On a cache hit, Pintos copies the data into the cached sector, marks the entry dirty, and marks it accessed.

On a cache miss, Pintos needs a cache entry for the sector. If the write is a partial-sector write, Pintos must first load the old sector contents into the cache so the unchanged bytes are preserved. If the write overwrites the full sector, it does not need to read the old sector first.

If the cache is full, Pintos selects a victim entry. A dirty victim must be flushed before reuse. After the entry is available, Pintos updates the cached contents and marks the entry dirty.

## Flushing Dirty Entries

A dirty cache entry must eventually be written back to disk.

One flush point is eviction. If a dirty cache entry is selected as a victim, the file system must write it back before reusing that entry.

Another flush point is file system shutdown. Pintos has a file system shutdown path, such as `filesys_done()`, where dirty entries should be synchronized to disk.

A third possible flush point is periodic write-behind. A timer-based routine can periodically scan the cache and write dirty entries back to disk.

The important rule is that modified cached data cannot simply be discarded.

## `byte_to_sector()`

Both read and write operations need to translate a file offset into a disk sector.

In the original Pintos file system, `byte_to_sector()` can compute this by taking the file's start sector and adding the offset divided by the sector size.

After indexed files are implemented, this function must change. It can no longer assume that the file's blocks are consecutive.

The buffer cache sits below this translation. Once the file system determines which disk sector contains the requested byte range, it asks the cache to read or write that sector.
