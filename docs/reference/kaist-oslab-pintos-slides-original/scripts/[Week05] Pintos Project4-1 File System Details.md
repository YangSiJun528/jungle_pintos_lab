# [Week05] Pintos Project4-1 File System Details

Source: https://youtu.be/mCAKZvZ1brs?si=wcu5zaZdeR6ICy6y

## Block Devices

A block device is storage such as an SSD or a hard disk. A block device is modeled as a linear array of blocks, and each block has an address called a logical block address, or LBA.

The meaning of "block" depends on context. In Pintos, the block device sector size is 512 bytes, and the Pintos file system also uses 512-byte sectors as its file system block size. Other file systems may use larger file system blocks, such as 4 KB.

Pintos represents a block device with `struct block`. Important fields include the device size in sectors, the device name, the block device type, a pointer to device-driver operations, and counters for the number of sectors read and written.

The file system code does not manipulate the hardware directly. It calls the block layer, and the block layer calls the operations supplied by the device driver.

## Formatting the File System

Formatting a file system means writing the initial file system data structures to a clean block device.

In Pintos, the basic layout starts with the inode for the free-map bitmap, the inode for the root directory, the bitmap data blocks, and the root directory data block.

For an 8 MB partition with 512-byte sectors, there are 16,384 sectors. The free map needs 16,384 bits, so four sectors are enough to store the bitmap. Four sectors are 2,048 bytes, and 2,048 bytes are 16,384 bits.

During formatting, Pintos must initialize the free map, create the inode for the bitmap file, write the bitmap to disk, create the inode for the root directory, and initialize the root directory.

The first few sectors therefore become occupied by essential file system metadata.

## File System Initialization

File system initialization has three major steps.

First, `inode_init()` initializes the in-memory list of open inodes. Pintos maintains open inodes in a linked list.

Second, `free_map_init()` creates and initializes the in-memory bitmap that represents free and used sectors.

Third, if formatting is requested, Pintos formats the file system. Formatting creates the free-map file and the root directory on disk.

The memory data structures are volatile. If the machine loses power, memory is lost. Therefore, file system metadata that must survive reboot has to be written to disk.

## Creating the Free Map File

The free map itself is stored as a file.

To create it, Pintos creates an inode at the sector reserved for the free map. Then it opens that inode as a file and writes the bitmap contents to disk.

The important functions in this flow are `inode_create()`, `file_open()`, and the bitmap write operation used by the free-map code.

The reserved sectors for the free-map inode and the root-directory inode must be marked as used in the bitmap before ordinary allocation begins.

## Creating the Root Directory

The root directory is created by creating an inode at the root-directory sector. The root directory initially has space for a fixed number of directory entries, such as 16 entries in the lecture example.

Creating a directory is not only creating an inode. The directory also needs data blocks where directory entries can be stored. The inode records the location and size of those directory data blocks.

In the original Pintos implementation, allocation uses `free_map_allocate()` to find a consecutive run of free sectors.

## Creating a File

Creating a file modifies several pieces of file system state.

Pintos needs a new inode for the file. It may also need data blocks for the file contents. It must update the parent directory by adding a directory entry that maps the new file name to the new inode sector. It must also update the free map to mark the newly allocated sectors as used.

In a simplified creation path, `filesys_create()` opens the root directory, allocates one sector for the new inode by calling `free_map_allocate()`, creates the inode with `inode_create()`, and adds a directory entry with `dir_add()`.

Even a small file creation can therefore update multiple sectors: the bitmap, the new inode sector, file data sectors if allocated, and the directory data block.

## Opening an Inode

`inode_open()` reads an inode from disk and returns an in-memory inode object.

Pintos also keeps a global list of open inodes. If an inode is already open, Pintos does not create a duplicate in-memory inode for the same sector. Instead, it increases the open count and returns the existing object.

The open count records how many open references exist for that inode.

This matters because two processes can open the same file at the same time. They should refer to the same underlying inode, while each open file object can still have its own file position.

## Opening a Directory

Opening a directory is similar to opening a file, but the object represents directory traversal.

`dir_open()` receives an inode and creates a directory object. The directory object contains a pointer to the inode and a position field that records where the next directory entry operation should occur.

The directory's data blocks contain an array of directory entries.

## Allocating Free Blocks

`free_map_allocate()` finds free sectors by scanning the bitmap.

In the original Pintos file system, it looks for a consecutive run of free bits. If the caller asks for 16 sectors, the bitmap must contain 16 consecutive free bits. Once such a region is found, those bits are marked as used, and the starting sector is returned.

This contiguous allocation strategy is one reason the original file system cannot easily extend files.

## Adding a Directory Entry

`dir_add()` adds a file name and inode sector to a directory.

First, it checks whether the given name already exists in the directory. If it already exists, creation fails.

Then it scans the directory data block to find an unused entry. When it finds an empty slot, it writes the new name and inode sector into that slot and marks the entry as in use.

This scan is linear. It is simple, but it can be expensive for large directories.

## Looking Up a Directory Entry

`dir_lookup()` searches a directory for a given file name.

It scans directory entries and compares names. If it finds a matching name, it returns the associated inode. This is the operation that converts a file name into the inode sector that represents the file.

In the original Pintos file system, because there is only the root directory, most path resolution is reduced to looking up a name in the root directory.

## Opening a File

`filesys_open()` is the file-system-level operation used by the `open` system call.

It opens the root directory, looks up the requested file name, obtains the inode, and then calls `file_open()` on that inode.

`file_open()` allocates and initializes a `struct file`. The file object stores the inode pointer, initializes the current position to zero, and initializes the write-denial state.

Every call to open a file creates a new `struct file`, even when the underlying inode is already open.

## Removing a File

Removing a file has two parts.

First, Pintos removes the directory entry by marking the entry as not in use. This breaks the mapping from the file name to the inode.

Second, Pintos marks the inode as removed. `inode_remove()` sets a removed flag in the in-memory inode. It does not necessarily free the inode and data blocks immediately, because other processes may still have the file open.

The actual deallocation happens in `inode_close()` when the open count reaches zero. At that point, Pintos can release the inode sector and the file's data sectors back to the free map.

This delayed removal is why removing a file name and freeing the underlying inode are separate steps.
