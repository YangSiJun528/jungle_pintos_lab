# [Week05] Pintos Project4-4 Subdirectories

Source: https://youtu.be/yQqNrIiB2hU?si=D7G5jkCkcnrFNj0-

## Directory Structure

The subdirectory feature changes Pintos from a flat directory structure into a hierarchical directory structure.

In the original Pintos file system, there is only the root directory. Every file is created in that root directory.

After the change, a directory entry can point not only to a regular file but also to another directory file. This creates a tree of directories and files.

## `.` and `..`

Each directory should contain two special entries.

`.` points to the current directory itself.

`..` points to the parent directory.

When a new directory is created, it is not completely empty. It should contain at least these two entries.

For the root directory, both `.` and `..` point to the root directory itself, because the root directory has no parent.

These entries should always exist. Attempts to remove them should fail.

## Paths

Once subdirectories exist, the file system needs the concept of a path.

A path is a sequence of directory names that describes how to reach a file or directory.

An absolute path starts from the root directory. It begins with `/`, such as `/user/pintos/src`.

A relative path starts from the current directory of the process. It can include names such as `.` for the current directory and `..` for the parent directory.

Path parsing is therefore required before creating, opening, or removing a file.

## File Types

With subdirectories, Pintos must distinguish ordinary files from directory files.

A regular file is read and written as a sequence of bytes. Its current offset moves through byte positions.

A directory file is different. Its contents are directory entries, and directory operations read one directory entry at a time.

Therefore, the inode needs a field that records whether the inode represents a regular file or a directory. One bit is enough conceptually, although the actual structure may use a larger field.

## Current Directory

Each process needs a current working directory.

This means `struct thread` should contain a field that points to the current directory, or to the inode for the current directory.

When a child process is created, it inherits the current directory from the parent process.

The initial process has no ordinary parent, so its current directory should be initialized to the root directory.

Adding this field to the process or thread structure is simple, but it affects many file-system operations because relative paths now depend on the process's current directory.

## Modifying File Creation

The original `filesys_create()` creates files in the root directory.

After subdirectories are implemented, file creation must first parse the path. The path parser must identify the parent directory and the final file name.

If the path is absolute, parsing starts from the root directory. If the path is relative, parsing starts from the current directory.

After the parent directory is found, Pintos allocates a new inode, initializes it as a regular file or directory as appropriate, and adds a directory entry to the parent directory.

The implementation must handle failures carefully. If an inode sector is allocated but adding the directory entry fails, Pintos must roll back the allocation and leave the file system in a consistent state.

## Creating a Directory

Creating a directory is similar to creating a regular file, but the inode must be marked as a directory.

The new directory also needs entries for `.` and `..`.

`.` should point to the new directory's own inode.

`..` should point to the parent directory's inode.

After these entries are initialized, the parent directory receives an entry that maps the new directory name to the new directory inode.

## Opening a File or Directory

Opening also requires path parsing.

The original implementation only looks in the root directory. With subdirectories, Pintos must walk the path one component at a time.

When an intermediate component is encountered, it must refer to a directory. The final component may refer to either a regular file or a directory, depending on the operation.

If the path is absolute, lookup starts from the root. If it is relative, lookup starts from the process's current directory.

## Removing Files and Directories

Removing a regular file is similar to the original behavior, except that the target directory is found by parsing the path.

Removing a directory needs additional checks. A directory should be removed only when it is empty, ignoring the mandatory `.` and `..` entries.

The special entries `.` and `..` themselves must not be removable.

The implementation must also distinguish whether the target inode is a regular file or a directory before choosing the correct removal rules.

## Directory System Calls

Project 4 adds system calls for directory manipulation.

`chdir` changes the current working directory of the process to the directory specified by a path.

`mkdir` creates a new directory.

`readdir` reads one directory entry name from an open directory file descriptor and advances the directory position. It should not return `.` or `..`.

`isdir` returns whether a file descriptor refers to a directory.

`inumber` returns the inode number associated with a file descriptor.

These system calls require the system-call layer to understand whether a file descriptor refers to a regular file or a directory.

## Summary

Subdirectories require more than adding directory entries.

The file system must distinguish file types, maintain a current directory per process, parse absolute and relative paths, initialize `.` and `..`, modify create/open/remove operations, and expose new directory system calls.

Once those pieces are implemented together, Pintos can support a hierarchical directory tree instead of a single flat root directory.
