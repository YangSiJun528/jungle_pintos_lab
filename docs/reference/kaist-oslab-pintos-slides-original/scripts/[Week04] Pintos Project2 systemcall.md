# [Week04] Pintos Project2 systemcall

Source: https://youtu.be/sBFJwVeAwEk?si=51sBUCoYdZZs9tQ9

## Intro

The next topic is how to implement system calls in Pintos. In current Pintos, the system call handler is empty. After modification, you have to fill the system call handler and implement system calls so that user programs can use them.

The system calls that need to be implemented include `halt` and other process and file related functions. We will go into the details of those individual functions later.

A system call is a programming interface for services provided by the operating system. It allows user programs to use kernel features such as creating a process, accessing a file, and saving a file.

System calls run in kernel mode. In KAIST Pintos, user programs invoke the `syscall` instruction, and the system call number and arguments are passed in registers. The key point of a system call is that the privilege level of the execution mode is raised by generating a trap.

From a user program, if you want to use a kernel service such as creating a process or saving a file, then you call a system call and ask the operating system to do the job for you. After the system call completes, control returns to the user program. The important thing is that the privilege level goes up while the system call is executing.

## Call Process of System Call in Pintos

This is the call process of a system call in the Pintos operating system. If the user calls `write`, then there is a file access wrapper in a C file. It contains the body of `write`, and it calls something like `syscall3`.

`syscall3` means that this is a system call with three parameters. In KAIST Pintos, it places the system call number in `%rax`, places the arguments in registers, and then executes the `syscall` instruction.

The kernel entry path calls `syscall_handler()`, and this function is currently empty. To implement system calls, you have to fill this out.

Each system call has its own number. The system call numbers are defined in `include/lib/syscall-nr.h`. For example, `SYS_HALT` corresponds to system call number 0, and `SYS_EXIT` corresponds to system call number 1.

## Requirement for System Call Handler

The first requirement is that the system call handler must call the correct system call using the system call number.

The most important part of the system call handler is checking the validity of addresses supplied by the user program. Suppose a system call gets a pointer parameter. Inside the kernel, the operating system may try to access that address. If the address is invalid, we do not know what will happen.

Before calling the actual body of the system call, the operating system or system call handler has to check whether the address supplied by the user program points to a correct address.

The pointers must point to the user area, not the kernel area. The virtual address space is divided into user space and kernel space. A pointer supplied by the user to a system call should point somewhere in user space, not in kernel space.

If these pointers do not point to a valid address, a page fault can occur. Once address validation completes, the kernel copies data from user memory to the kernel when needed. After executing the system call, the return value is saved in the `%rax` register, through the `rax` member of `struct intr_frame`.

The important rule is that the user should supply valid user addresses to the kernel. The kernel should avoid directly depending on user address space while it is executing. Instead, it should copy parameters into its own address space so that it can execute inside the kernel without repeatedly accessing user space.

## Address Validation

A user can pass invalid pointers through a system call. It can pass a null pointer, a pointer to unmapped virtual memory, or a pointer that points to kernel address space. In all of these cases, the operating system has to kill the program.

When a user calls a system call, the kernel must detect invalid pointers and terminate the process without harming the kernel or other running processes.

There are two ways to detect the validity of an address.

The first method is to check the validity of the user-provided pointer explicitly. You have to check the page table and check whether every given address is mapped. In KAIST Pintos, you use functions such as `pml4_get_page()` from `threads/mmu.c` and address helpers in `include/threads/vaddr.h`.

The second method checks whether the user pointer is below `KERN_BASE`. In that case, the operating system does not check whether it refers to a mapped address. It only checks that the given user pointer is below `KERN_BASE`. If the user-supplied pointer is invalid, it causes a page fault, and you handle that by modifying `page_fault()`.

The second approach is faster because the kernel does not have to explicitly check everything. It relies on hardware memory protection to verify the validity of user addresses. This approach is widely used in real operating systems such as Linux.

## Accessing User Memory

Consider an example where a process holds a lock or allocates memory with `malloc()`, and then a page fault occurs during the work. The process dies, but the global lock may still be held by the process, or the allocated memory may not have been freed. A page fault may therefore cause resource leakage.

Before terminating the process, the operating system has to unlock or release a lock, or return allocated memory, to avoid resource leakage.

Handling this kind of leakage is different in the two validation methods. In the first method, it is straightforward because the operating system checks everything before it locks or allocates resources. In a system call, it locks or allocates memory only after verifying pointer validity.

The second method is more difficult because there is no normal error code returned from an invalid memory access. Pintos provides helper functions for these cases:

- `get_user()` reads a byte from the specified user address.
- `put_user()` writes one byte to the specified user address.

## Add System Calls: Process Related System Calls

These are the process-related system calls you have to implement.

`halt` shuts down the operating system. Pintos should not shut down except when `halt` is called.

`exit` exits the process. When a process exits, it should print the name of the process and the exit status.

`fork` creates a child process. `exec` changes the current process to the executable whose name is given in the command line, passing the arguments to it. In KAIST Pintos, `exec` itself does not create a child process.

`wait` waits for the termination of a child process whose process ID is `pid`.

## Process Hierarchy

In this project, you have to introduce the notion of process hierarchy. You have to specify the parent process, the child processes that it has created, and pointers to siblings.

There are two important concepts to implement. First, add a pointer to the parent process to the thread structure. Second, add pointers for siblings so that child processes can be maintained in a doubly linked list.

There are many ways to implement pointers to children, but it is not reasonable to store a separate pointer for every child directly inside the parent. Usually you can maintain the head and tail of the child or sibling list. The list of siblings can be maintained in creation order, from the oldest to the youngest.

## `wait`

The `wait` system call waits for a child process to exit and receives the child's exit status. If the process identified by `pid` is still alive, `wait` blocks until it terminates. Then it returns the status that the child passed to `exit`.

If the process identified by `pid` did not call `exit` but was terminated by the kernel, `wait` returns `-1`.

It is possible for a parent process to call `wait` for an already terminated child process. In that case, it returns the exit status of the terminated process. The important thing is that after the child terminates, the parent should deallocate its process descriptor.

`wait` fails if the process ID does not refer to a direct child of the calling process. It also fails if the calling process has already called `wait` on the same `pid`.

The current form of `process_wait()` has to be implemented. Implementing it is not easy, so for an early step you may put an infinite loop there, but that is not the correct implementation.

## Correct Implementation of `process_wait()`

The correct implementation of `process_wait()` searches the descriptor of the child process using the parameter. The caller blocks until the child process exits. Once the child exits, the parent deallocates the descriptor for the child process and returns the exit status.

How can we synchronize the caller of `process_wait()` and the execution of the child thread? Add a wait semaphore to the thread structure. This semaphore is initialized to zero when a thread is first created.

In the `wait` system call, the caller calls `sema_down()` on the child's wait semaphore. In the exit path of the child process, the child calls `sema_up()`. `sema_up()` increases the semaphore value by one, and `sema_down()` decreases the semaphore value by one.

Because the semaphore value is initialized to zero when the thread is created, the caller blocks when it calls `wait`. When the child process calls `exit`, it increases the semaphore value by one, and the waiting process can wake up and continue.

In `exit`, the process also has to return its exit status. Therefore, we need to add a field to `struct thread` to store the exit status of the thread.

## Flow of Parent Calling `wait` and Child

The control flow uses the semaphore as follows. In `process_wait()`, the parent calls `sema_down()`. In `exit`, the child calls `sema_up()`.

By calling `sema_down()`, the caller blocks while waiting for the child process to finish. When a user process calls `exit`, it calls `sema_up()`, so any process that has been waiting for the semaphore value to increase is unblocked.

## `exec()` System Call

In KAIST Pintos, `exec` changes the current process to a new executable and executes that binary.

The `exec` system call gets a command-line argument and runs the requested program. It passes arguments to the program to be executed. If successful, it never returns; if loading fails, the process terminates with exit status `-1`.

For `fork`, the parent should not return until it knows whether the child has successfully duplicated resources. For `exec`, the current process loads the executable and switches to it.

## Kernel Function for `exec()`: `process_exec()`

`process_exec()` is the kernel function for `exec`. It loads the binary file for the current process and switches execution to the loaded program.

For child creation through `fork`, you can use a semaphore to synchronize the caller and the child process. Add a semaphore to the thread structure if you need to wait for child setup to complete.

In child setup, the parent can call `sema_down()` to wait for successful setup, and the child can call `sema_up()` when setup is complete.

We also need another variable representing load status in the thread structure. This field records whether the file was loaded successfully.

## Current Flow of Parent Calling `exec` and Child

For child creation, the parent process calls the creation path and a new thread starts running. Inside the child path, the child loads or duplicates the resources it needs. The parent process has to wait until it is sure that setup has completed. Then both parent and child can continue.

The control flow is as follows. The parent process continues executing and creates a new thread. The new thread continues execution and performs setup. After setup finishes, it calls `sema_up()`. After the parent's `sema_down()` returns, both processes can continue in parallel.

This is how to implement parent-child synchronization correctly: the parent waits on a semaphore, and after child setup finishes, the child calls `sema_up()`.

An important design issue is where to put `sema_down()`: directly next to the creation call, or inside the creation helper itself.

## `exit`

In `exit`, you have to return the exit status of a given process. If a process calls `exit`, the operating system has to terminate the current user program and return the status to the kernel. If the parent process waits for it, the parent will read the exit status of the given thread.

In the existing `exit` body, you have to add code to save the exit status in the process descriptor or `struct thread`. You also have to print the exit status and the name of the thread, and then call `thread_exit()`.

## Kernel Function for `exit()`: `thread_exit()`

`thread_exit()` is the common kernel function for exit. Here, you have to store the status in the process status field and signal the semaphore so that a process waiting for this thread to finish can continue.

In `thread_exit()`, interrupts are disabled. You have to disable interrupts whenever you manipulate thread lists. Then Pintos removes the node from the thread list, changes the status of the thread to `THREAD_DYING`, yields the CPU to another process, and this function is not reached again.
