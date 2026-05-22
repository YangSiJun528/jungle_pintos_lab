# [Week05] Pintos Project4-4 Subdirectories

Source: https://youtu.be/yQqNrIiB2hU?si=D7G5jkCkcnrFNj0-

## Reserved Directory Entries

The second directory entry points to the inode of itself. Okay. So when you implement a subdirectory feature in the file system, in each directory entry, you have to reserve two directory entries. Reserve two directory entries in each directory.

The first one is the current directory, and the second one is parent directory. We are going to come into this concept right in the next slide.

## Directory Structure

Let's go to the next slide. These are the list of the things you have to implement.

The first one, you have to implement hierarchical directory structure. Make the directory entry point to not only the regular file, but also the directory file. So in Pintos, there are going to be two types of file: regular file and directory file.

Okay. This is a very important part. Make the directory entry point to not only the regular file but also directory file. What are the details of making this happen? I am going to explain later.

The second most important thing is add directory entries with dot and dot dot. I guess you are all familiar with the notion of dot. For example, when you perform the file list `.` and `*.c`, it means list all the files whose extension is C under the current directory.

The second thing is dot dot. I know that you all are aware that dot dot means the parent directory. If you perform, for example, `cd ..`, change the directory to dot dot, it means change the notion of current directory to its parent directory.

When you implement a subdirectory, you have to reserve the first two directory entries for every individual directory, for the current directory and the parent directory.

Then, of course, you have the data block for the directory, and it contains a number of entries. Then you have to first reserve the first. The name of the first entry is current directory, and this one points to the inode of the current directory.

The name of the second entry is dot dot. It is two bytes, and it points to the inode of the parent directory.

So when you create the new directory, the directory is going to be not entirely empty. It will contain two entries. The first one is the inode of the current directory, and the second one is inode of the parent directory. Of course, you can switch the positions of the two, but you have to be consistent all through your code.

Current directory, the notion of current directory, is global for a process. So in `struct proc` or thread structure, you have to add a field to represent what the current directory is. Of course, that is going to be the pointer to the inode for the current directory.

All right. Then you have to modify the directory-related functions. So file system create, opening a file, and deleting a file. Also, you have to create new system calls associated with a directory.

## Path

Okay. The most important part now is that there is a concept of path. By enabling a subdirectory, you are now having a concept of path.

When every file is on the same directory, or every file is on a flat directory, then there does not have to be any notion of path. But now you have, for example, `/user/pintos/source/util/.../s.c`, for example. These are all paths.

Path is a sequence of directories, each of which has a parent-child relationship. So now there is a concept of path. These are all paths.

The next thing is, there are two types of path. The first one is absolute path, and the other one is relative path.

The way to distinguish the absolute path from the relative path is the start of the path, the starting character of the path. In absolute path, it specifies the entire path starting from the root directory. So in absolute path, the path string starts with the root, `/user/src/pintos/blah blah blah`. The path string starts with the root. In that case, this is called absolute path.

The relative path is the one that starts with dot or dot dot. For example, `../src/util/something/something`. Then the first directory in this path is the parent directory wherever I am, or wherever the current directory is. This path starts from the parent directory. So the location is always relative to the current position or current directory. This type of path specification is relative directory or relative path.

## File Types

Okay. These are the things you need to do. There are five things, from one to five.

The first thing is, now there are two types of file. There is ordinary file or regular file, and there is directory file. So there are two types of inode: regular inode for regular file, and the inode for the directory.

Do we really have to distinguish the two? Yes, of course.

In a regular file data block, the data on the regular file is a sequence of bytes. So you will have the current offset, and in regular file you will move this current offset with read and write function.

For the directory file, directory file is special. It is not just a sequence of bytes. It is a sequence, array of directory entries. So instead of reading and writing certain specified bytes, you read a directory entry at a time.

The way to access, the way to read from, and the way to write to the regular file and directory files are different. So you have to specify what type of file it is.

You are going to add a flag to indicate whether the inode is regular file or directory file. Because you have to differentiate only two, you will need one bit. One bit is enough. However, there is no way to save it in our data structure, so you might end up allocating whatever the smallest data structure available, a single character for example. It is one byte.

In the data structure `struct thread`, you have to define the notion of current directory. This is pointer to the inode of the current directory.

When creating a new file, opening, or removing a file, you have to modify so that each create, open, delete can handle the directory as well as the regular file.

For directory manipulation, you have to add system calls for directory manipulation. For example, system calls for change directory, or you will have to implement system call for creating a directory, or system call for removing a directory, or stuff like that.

Then you will have to add special directory entries called dot and dot dot.

## Inode Structure

Okay. Let's start our journey.

First one, you will have to modify the inode structure of on-disk. You will have to add a flag indicating if it is a file. Sorry, it is a file of course, because directory is also a file. It indicates if it is a regular file or if it is directory file.

Now we will add a regular file flag or a directory flag. When a file is created, you should set this flag properly depending on whether it is a regular file or a directory file.

## Data Structure Design

Before we move on to the next topic, remember the most essential thing, essential task, is designing or modifying data structure. That is the most essential part in writing software.

You first have to define or design or modify data structure. This is object, and you have to define what kind of fields it has to have. Then next thing, you define or write the code, or method. You can say it is method when you use it as a function, or you use the term procedure, or even you use the term operation.

You first define the object definition very clearly, and then you define the associated operation, procedure, function, method, whatever.

Currently in the first or second step, you redefine or modify the data structure to accommodate the concept of directory.

In the first step, we modify the data structure for on-disk inode. Then we modify the data structure for the thread to accommodate the concept of directory. In the second part, we are adding the notion of current directory in the concept of thread.

Of course, the modification is very simple and straightforward. In the first step, we just add a flag in the on-disk inode structure to represent whether a current file is directory or regular file. In `struct thread`, you simply add a single field to denote what the current directory is.

But as in most other software design, modifying a data structure, even though the modification is simple and insignificant, may have profound implications on the rest of your design.

## Current Directory Inheritance

So when a thread is created for the first time, every thread has a parent. A thread is created by the parent. So when a thread is created, it inherits the current directory from the parent. It inherits the current directory from the parent.

When a thread is created for the first time, the current directory of the child process or child thread is the same as the current directory of the parent thread.

Of course, there is one thread that does not have a parent. What is the thread that does not have a parent thread? This is something like Genesis in creation of the world at the beginning. What is the thread that does not have a parent thread? This is called init process.

When a computer is enabled for the first time and electricity is engaged, the motherboard is initialized, power is engaged in all the hardware, the fan starts running, and you hear some mechanical sounds for cooling fans, etc. Then hard disk is initialized, CPU is initialized, DRAM is initialized, and the software is loaded from the hard disk drive to the memory. The operating system starts to run, and then the very first thread, which is called init process, is created.

At that time, the init process does not have a parent thread. It is created by itself, for your fun.

Then what is the process ID of the first thread in Pintos? Look at the code and find it out for fun. What is the process ID of the first thread in Pintos when the system is booted? Find it out for yourself and write it to Piazza.

Let's move on to the next topic.

We have modified the on-disk structure to accommodate the concept of subdirectory, and we have changed the thread structure slightly and added a current directory pointer to a thread.

## Algorithm of File Creation

Now it is time to modify the algorithm.

When we create the file, we first have to parse the path. We have to parse the path, and then we should create the appropriate directory entry at the target directory, and also we have to create inode.

The first thing is, we have to distinguish whether it is absolute path or relative path, and find target directory where you have to create the file, and create a file.

These are detailed steps of creating a file. Of course, we already have a module to create a file in Pintos, except that it just creates a file on the root directory. So all you have to do in changing the file creation is to find the right directory where you need to create a file and create a file on the path, instead of creating a file on the root directory.

So this is the one you have to modify. Examine the path and open directory of the path, allocate new inode and initialize the inode with the newly created file, and add the directory entry to the target directory.

Okay. But you know it is important that operating system should be bulletproof and should be written against any failure or exception. What if there is no disk space in the course of creating a file? You successfully allocate a new inode, you create a new inode for a file, but after creating all these you find that directory is full. Then creating a new file should fail.

In that case, you have to deallocate all the inodes and bitmaps you have just modified. So if you succeed, then you close the directory and return success. If you fail, then you have to deallocate inode and return `success`. `success` means return the value of the variable `success`. This is the name of the variable. It does not mean that you have succeeded in creating the file. You may have failed in creating file.

So you have to modify the algorithm of file creation. Of course, if you modify the algorithm of file creation, then you are going to modify the algorithm of file deletion as well.

## Creating a File

These are the details of creating a file. This is the name of the file, and in Pintos you specify the size of the initial file.

In reality and in Pintos, you always have to create a file in the root directory. But after modification, first you have to parse the path, and then create the file on that directory.

Of course, as I mentioned before, you have to distinguish absolute path and relative path, and parse. When creating a file, you have to add the code to set a flag `is_directory`. This is the name of the flag you may have added to denote whether a current file is directory or not. Set the flag to zero if it is regular file.

Of course, you have to add new directory entry to directory path.

## Opening a File

Also you have to modify opening a file.

When you open a file, currently you always find the file on the root directory. But now, because there is a subdirectory and there is a notion of path, you have to parse the path, find a file on that directory, and open it.

Opening the file itself is the same as opening a file as before. However, there is an added step: parse the path and find a file on that directory.

When the path is absolute, you find it from the root directory. Find from the root directory. When the path is relative, then find it from the current directory. Okay. That is opening a file.

## Removing Files

Another step is removing the file. It is the same as before.

In original Pintos, we always remove the file from the root directory. Now after modification, you remove the file from the directory specified by the path.

If the inode of the target file is for a regular file, you can just delete it. But if it is a directory, then you have to check if the directory is empty or not. Here, we remove the directory file only when the directory is empty.

You have to add a number of system calls about manipulating the directory files. This is change directory, creating a directory, reading a directory, checking if the current file is directory, and returning the number of the inode associated.

The first one is changing the directory. We get the path, parse the path, and change the current working directory of the process to the target directory specified as parameter to a function, and return true if successful, and return false on failure.

The second thing is making a directory. Sorry, there is parenthesis missing. It creates a directory named `dir` and returns true if successful and false on failure.

## `readdir`

Read the directory accepts file descriptor and name. Let me explain the concept of `readdir`. It requires a little bit of elaboration.

In reading a regular file, we use a function `read` and pass the parameters buffer and size. In reading a file, this file is represented by descriptor `fd`, and there is current offset. When a system call read is called, the operating system, or file system specifically, reads size amount of data starting from the current offset position. This is read.

`readdir` is similar but different.

In `readdir`, of course, the directory is pointed by `fd`. In a directory file, the directory file is partitioned into directory entries. Directory file also has a pointer that points to the current directory entry to read.

If you issue `readdir`, this function reads a single directory entry and copies the name of the file in this directory entry to the buffer specified by `name`, and moves the current offset to the next directory entry. That is what `readdir` is for.

That is the reason why we specify the flag to distinguish the regular file and the directory file. When application issues a read system call, application does not know whether a given file descriptor is associated with the directory or not.

If the application issues a read for the regular file, the file system passes it and calls regular read. If the application calls read on the regular file, then it calls read. But if the application issues read on the directory file, then we call the file system `readdir`.

But the current directory dot and the parent directory dot dot should not be returned by `readdir`.

There is a function `isdir`. This denotes whether a current file associated with the file descriptor is a regular file or directory file. It returns true if the `fd` represents directory.

The function `inumber` returns the inode number associated with the file descriptor.

## Directory Entries

The last part: we have to add special directory entries. The special directory entry dot means it represents itself, and dot dot represents the parent directory. So when a directory is created, the special entries should be added.

There is an interesting property for the root directory. Let's say this is data block for the root directory. This is inode for the root directory, and inode has a pointer that points to the data block associated with itself. The associated data blocks will contain the directory entries for root.

The current directory in the first entry will represent the inode for the current directory, and then the second will be the parent entry. However, for the root directory, these two point to the same place, because root directory does not have its parent directory.

Please be alert. These two entries should be there always. If a user tries to remove them, the system call should return failure, and it should fail. In the directory entry, the first entry and the second entry should not be removable.

That is the end of the functions you need to implement. Good luck.
