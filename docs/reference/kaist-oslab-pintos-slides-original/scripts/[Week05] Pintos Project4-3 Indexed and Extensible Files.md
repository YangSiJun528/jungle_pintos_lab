# [Week05] Pintos Project4-3 Indexed and Extensible Files

Source: https://youtu.be/4Vg66wWwkXE?si=et5lvozGrkyBRALs

## Original File Representation

The original Pintos file system gives a file a fixed size when it is created.

The on-disk inode stores a start sector and the file length. This means all sectors for the file must be placed in one consecutive region on disk.

This representation is limited. File systems commonly append data to existing files. If the sector immediately after a file is already used by another file, the original Pintos representation cannot extend the file in place.

To support growing files, the inode representation must change.

## Indexed File Representation

After modification, the inode should contain block pointers instead of only a start sector.

Each pointer can refer to a data block. If a file needs to grow, Pintos can allocate a free sector anywhere on disk and add a pointer to that new sector. The file's blocks no longer have to be consecutive.

This is the main idea behind indexed file allocation.

## Direct, Indirect, and Double-Indirect Blocks

One possible design is similar to a Unix inode.

The inode contains several direct pointers. A direct pointer points directly to a data sector.

The inode also contains an indirect pointer. The indirect pointer points to an indirect block, and the indirect block contains pointers to data sectors.

The inode can also contain a double-indirect pointer. The double-indirect pointer points to a block of pointers to indirect blocks. Each of those indirect blocks then points to data sectors.

This structure lets small files use direct pointers efficiently while still supporting larger files through indirect and double-indirect levels.

Because a Pintos on-disk inode occupies one 512-byte sector, the inode layout must fit exactly inside that sector.

## Changing `struct inode_disk`

The original `struct inode_disk` has a start sector, a length, a magic number, and unused space.

For indexed files, the start sector is replaced by pointer fields. The inode still needs the file length and magic number, but it also needs direct, indirect, and double-indirect pointer fields.

The exact number of direct pointers is a design decision, as long as the inode remains one sector in size and the file system can support the required maximum file size.

## Translating a File Offset

The file system must translate a byte offset in a file into the disk sector that contains that byte.

In the original implementation, `byte_to_sector()` can compute the sector by adding the file offset divided by the sector size to the start sector.

With indexed files, `byte_to_sector()` must inspect the inode pointers.

If the block index is in the direct-pointer range, it uses a direct pointer. If the block index is in the indirect range, it reads the indirect block and selects the appropriate pointer. If the block index is in the double-indirect range, it reads the first-level pointer block and then the second-level indirect block.

This offset-to-sector translation becomes one of the central routines in the indexed file implementation.

## Creating an Inode

`inode_create()` must also change.

The original implementation allocates a consecutive range of sectors and stores only the start sector.

After the indexed representation is added, `inode_create()` must initialize the pointer structure and allocate the sectors needed for the initial file length. The allocated data sectors should be recorded through the direct, indirect, or double-indirect pointer structure.

Newly allocated file sectors should be initialized, typically with zero bytes.

## Deleting an Inode

Deleting a file also becomes more complex.

In the original representation, the file's data sectors form one consecutive extent, so deallocation is simple.

With indexed files, Pintos must walk all pointers in the inode and release every allocated data sector. If indirect blocks or double-indirect blocks were allocated, those metadata blocks must also be released.

This deallocation logic belongs on the path that finally closes and removes the inode after no open references remain.

## Extending a File

The file system must support writes beyond the current end of a file.

When `inode_write_at()` writes past the existing file length, Pintos must allocate any newly required sectors, update the inode pointers, and update the file length.

If the write leaves a gap because the program previously used `seek()` to move beyond the end of the file, the gap should read as zero. This is the hole-punching behavior discussed in the lecture: seeking past the end does not allocate blocks by itself, but a later write should make the intermediate region behave as zero-filled data.

The extension path must be synchronized carefully, because multiple operations could try to extend the same file at the same time.

## Seek and File Size

The `seek` operation changes the current offset of an open file object.

If a program seeks beyond the current file size, the file size does not immediately change and no blocks are allocated immediately. The current offset simply moves.

The file grows only when a write actually occurs beyond the old end of file.

This distinction matters because `seek()` changes the open file object's position, while `write()` changes file contents and may change the inode.

## Transition to Subdirectories

After indexed files, the next feature is subdirectories.

The original Pintos file system has only the root directory. Project 4 changes the file system so directories can contain both regular files and other directories, forming a hierarchical tree.

Supporting subdirectories requires changes to inode metadata, directory entries, path parsing, file creation, file opening, file removal, and new directory-related system calls.
