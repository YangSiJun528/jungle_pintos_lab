# [Week04] Pintos Project2 file manipulation

Source: https://youtu.be/SqMD8rbmEjY?si=accwnC44dvK9bpIs

## File Manipulation

Now we are going to implement file manipulation features in the Pintos operating system.

There is a process descriptor, which is `struct thread`. Inside the thread structure, we need a pointer that points to the file descriptor table. The file descriptor table is an array of pointers, and each entry in the array points to a file object.

The first entries are used for standard streams. In KAIST Pintos, file descriptor 0 is `STDIN_FILENO`, standard input, and file descriptor 1 is `STDOUT_FILENO`, standard output. After those entries, the table contains ordinary file descriptor entries.

Current Pintos does not have a file descriptor table, so we have to implement it.

## File Descriptor Table

Each process has its own file descriptor table. We can define the maximum size as 64 entries. The file descriptor table is an array of `struct file *` pointers. Each entry points to an object called `struct file`.

A file descriptor is an index into the file descriptor table. It is allocated sequentially from the available entries. File descriptors 0 and 1 are reserved for standard input and standard output, and `open` should not return either of these file descriptors.

The `open` system call returns a file descriptor. It opens a file, scans for an empty entry in the file descriptor table, and sets that entry to point to the opened `struct file`.

The `close` system call resets the value of the corresponding table entry to zero or `NULL`.

## File Descriptor Table Layout

We are going to define a file descriptor table with 64 entries.

One approach is to embed the file descriptor table directly inside `struct thread`. Another approach is to add a pointer to the file descriptor table in `struct thread`, then allocate the actual table separately in kernel memory. With the pointer approach, two threads could share the same file descriptor table if that is needed.

In the example, thread A has its own file descriptor table, and thread B has its own file descriptor table. Each `struct thread` has a file descriptor table pointer that points to its own table.

When a thread is created, the operating system allocates a file descriptor table for the thread and initializes the pointer to that table.

One thing we should not forget is that when initializing the file descriptor table, we have to reserve file descriptor 0 for standard input and file descriptor 1 for standard output.

When a process is terminated, it has to close all files. Then the operating system deallocates the file descriptor table.

## Race Conditions

One thing we have to emphasize is race conditions.

In Pintos, we are going to use a global lock to avoid race conditions in file system operations. We define a global lock, and whenever a file-system-related system call is executed, it is protected by that lock.

This is used to avoid race conditions in file system operations.

## Page Fault for Tests

We have to modify page fault handling for the tests. Some Pintos tests check whether the kernel handles a bad process properly.

In Pintos, when a page fault occurs in a user process, the kernel needs to terminate the process and print the process name and exit status `-1`. For this purpose, we modify page fault handling to satisfy the test requirement. A simple approach is to route the failure through the same path as `exit(-1)`.

## File Manipulation System Calls

These are the system calls related to file manipulation.

`create` creates a file with an initial size. It calls `filesys_create()`.

`remove` removes the file with the given name. It calls `filesys_remove()`.

`open` opens a file. It calls `filesys_open()`, stores the returned `struct file *` in the file descriptor table, and returns the file descriptor.

These helper functions are already defined in Pintos. What you have to do is provide appropriate system calls that call these functions.

`filesize` returns the length of a file. It calls `file_length()`.

## `read`

In the `read` system call, you have to distinguish reads from standard input and reads from other file descriptors.

If the file descriptor is 0, then read from the keyboard by calling `input_getc()`.

For other file descriptors, call `file_read()`.

## `write`

The same rule applies to the `write` system call.

If the file descriptor is 1, then write the output to the console. The project document recommends writing the buffer with `putbuf()`.

For other file descriptors, call `file_write()`.

## Other File System Calls

There is a function called `seek`, which changes the current position in the file by calling `file_seek()`.

There is a function called `tell`, which returns the current position in the file by calling `file_tell()`.

There is also `close`, which closes the file with `file_close()` and releases the associated file descriptor table entry.

## Deny Writes to Executables

The last topic is denying writes to executables.

What happens if the operating system tries to execute a file while that file is being modified? The result can be unpredictable. The objective is to not allow a file to be modified while it is open for execution.

The approach is to call `file_deny_write()` when the file is loaded for execution and keep that file open as long as the process is still running. When the process exits, call `file_allow_write()` or close the file. Using this approach, Pintos can deny writes to an executable file while it is running.

Once you completely implement all these features, you should be able to pass the tests.

## Summary of Functions to Add and Modify

The process creation call flow in Pintos is as follows.

First, Pintos calls `process_create_initd()` for the initial process. Then `process_create_initd()` calls `thread_create()`. The new thread runs `initd()`, `initd()` calls `process_exec()`, and `process_exec()` calls `load()`.

At each stage, you have to provide the required feature. In `process_exec()`, parse the name of the program to run. In `thread_create()`, create a thread and add it to the ready list. A new thread is created and put into the ready list, but the executable is loaded later in `process_exec()`.

In `process_exec()`, prepare the interrupt frame so that execution can move from kernel mode to user mode and start running the user program. Then load the executable. If all of this succeeds, the user program can run.
