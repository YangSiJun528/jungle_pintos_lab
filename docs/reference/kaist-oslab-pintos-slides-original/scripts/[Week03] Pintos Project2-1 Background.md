# [Week03] Pintos Project2-1 Background

Source: https://youtu.be/RbsE0EQ9_dY?si=JcKIZByXsQfrbEmk

## Intro

In the second part of the second project, we are going to let Pintos run a program. The objective of Project 2 is to execute user programs. Pintos currently cannot run a user program, so basically it is not an operating system yet. This lecture is background for that project.

## Running a Program in Pintos

In Pintos, or in any operating system, a command line is passed as an argument to execute a process. In the KAIST Pintos tree, the first user process is created through `process_create_initd()` and `thread_create()`, and the executable is loaded by `process_exec()`.

The problem is that the body that calls `process_wait()` simply returns. The `process_wait()` function has to wait for the child process to be created and has to wait until it finishes. However, it currently does not do anything and simply returns.

As a result, Pintos works like this. In Pintos, `init` is process zero. It is the mother of all processes, and its process ID is zero. It creates a new process and schedules it, but right after it schedules the process, it exits. If the operating system exits, then the other program cannot run. In current Pintos, there is no way for the process to get executed.

The final goal is to let Pintos wait for the completion of the child process and then continue. That requires quite a bit of work, and we will go through it step by step.

## Executing a Program

The first important function for execution is `process_exec()`. It gets a file name or command line, which contains the name of the executable file. For example, it could be `a.out`; if you want to perform an `ls` command, then `ls` is supplied as the file-name parameter.

For the initial user process, `process_create_initd()` calls `thread_create()`. By calling `thread_create()`, Pintos creates a new thread. At that moment, the new thread is created and execution continues.

## Creating a Thread

`thread_create()` creates a new `struct thread`, initializes it, allocates the kernel stack, registers the function to run, and adds the thread to the ready list. Registering the function means putting the instruction pointer or instruction counter at the entry point of the given process.

The code is simple at the high level. `thread_create()` needs the name of the thread, the default priority to set, the function name, and auxiliary parameters.

First, it allocates a single 4 KB page in kernel space. Then it initializes the thread structure inside that page. The thread structure may take 64 bytes, 128 bytes, or whatever the structure size is. Then it allocates a thread ID, because every thread has a unique ID.

It also allocates a kernel stack. In the kernel stack, it initializes various fields, including the function it wants to execute. The kernel stack contains the address of the function that needs to be executed. Then the kernel puts the thread into the ready list by unblocking it. This is how a thread is created.

## Starting a Process

So far, a thread has been created and put into the ready list. The raw lecture transcript describes this step with the older name `start_process`. In the current KAIST Pintos tree used by this repository, there is no separate `start_process()` function; the corresponding path is `process_create_initd()` -> `thread_create(..., initd, ...)` -> `initd()` -> `process_exec()`.

`process_exec()` gets a file name, which is the name of the binary file it wants to execute. The first thing it needs to do is load that binary file from disk to memory. Out of that binary file, it obtains the location of the instruction it wants to execute. It also obtains the stack-top pointer of the user stack. That is the role of `load()`.

If loading succeeds, the process continues to execute. If loading fails, the thread has to exit. On exit, it has to clean up all memory chunks that have been allocated to execute the process.

`load()` is a very important function. It pretty much does everything from loading the binary file from disk to memory, initializing the user stack, setting the initial pointers, and then jumping to the user program.

## Loading a Program

Suppose there is an executable file, such as `a.out`, and memory. If `load()` is called, the kernel creates the page table for the given thread. Then it opens the file and reads the ELF header.

The executable is in ELF format. The ELF header contains information about how the file is organized, including locations of the data section, BSS section, and binary code. Using the ELF header, `load()` parses the file and loads the data section into the data segment. It also reads the text section into the text segment.

Then it creates and initializes a stack for the process. The thread structure has already been created, and the thread structure contains a pointer to the page table, `pml4` in KAIST Pintos. That page table points to all these data structures: data, text, and stack.

Loading a file does many things. It reads the binary file into memory, initializes the data segment, and initializes the text segment for a process to run.

The actual `load()` function takes four parameters: a file name and two output variables among them. After `load()` executes, one variable contains the starting entry point of the program, and the other variable contains the stack top of the user stack that needs to be executed.

## After Loading

After `load()` finishes, the operating system has read the program file into memory, initialized the stack, initialized the data and BSS sections, and initialized the text memory.

Now it is time to look at what we are supposed to do. First, we need to implement a mechanism for passing arguments and creating a thread.

Currently Pintos does not have a mechanism to tokenize command-line arguments. It just passes the entire command line to process execution. After modification, we have to tokenize the individual tokens in a command line. Then we should be able to identify the program name, find the executable with the file name, and pass arguments to the user stack and registers.

For example, for an `echo` command, arguments such as `x`, `y`, and `z` should be pushed to the user stack so that `echo` can use those arguments to do its job.

## Functions to Modify

There are multiple possible ways to implement this project. The raw lecture mentions `start_process`, but in this KAIST Pintos tree the main function to extend for argument passing is `process_exec()`, as described in the KAIST project document.

## Parse the Arguments and Push Them to the Stack

The most important task is to parse the arguments and push them to the user stack.

Inside `process_exec()`, Pintos receives the command line. It should parse the string, extract the file name, and use the first token as the program name. Then `load()` will try to find the file under that file name and execute it.

In `process_exec()`, we have to parse the file name, tokenize the individual tokens, and place the parameters for the user program. The standard library provides a function, `strtok_r()`, that can be used to tokenize the command line.

## Program Name and Thread Name

`process_create_initd()` calls `thread_create()`. It has two important parts. First, it passes the name of the file it wants to execute as the name of the thread. Second, it creates a thread with `initd()` as the function to execute; `initd()` then calls `process_exec()`.

## `process_exec()`

In the raw lecture this section is introduced as `start_process`. In this repository's KAIST Pintos code, the corresponding function is `process_exec()`. In `process_exec()`, Pintos allocates an interrupt frame, loads the program, and initializes both the interrupt frame and the user stack. Then it sets up the arguments and jumps to the user program through `do_iret()`.

The problem is that current Pintos does not have a mechanism to initialize the user stack with arguments. This is the part you have to implement in this homework.

Before that, we need to understand what it means to get into the kernel and get out of the kernel.

## Getting Into and Out of Kernel

To get into the kernel for a system call in KAIST Pintos, the user program executes the `syscall` instruction. The user program traps into the operating system. When the kernel is done with its work, Pintos returns to user mode through `do_iret()` and the `iretq` instruction.

The virtual address space of a process consists of kernel space and user space. In user space there are text, data, BSS, and stack regions. In normal execution on x86-64, the stack pointer `rsp` points to the top of the user stack.

When the CPU enters the kernel, the entry path switches from the user stack to the kernel stack. After it switches the stack from the user one to the kernel one, it saves registers that have been used by the user process. The data structure that the kernel uses to store the user's registers on the kernel stack is called the interrupt frame.

When the CPU traps into the kernel, it switches from user mode to kernel mode and the entry code saves registers into the interrupt frame that resides in kernel space. In the system call path, the important instruction is `syscall`.

## `struct intr_frame`

The `struct intr_frame` data structure contains five registers, twelve bytes of extra fields, and many general-purpose registers.

Part of the interrupt frame is defined by the CPU, and part of it is defined by the operating system. If you look at another operating system running on the x86 CPU architecture, the CPU-defined part remains the same, but the operating-system-defined part may vary depending on the operating system.

The interrupt frame resides in the kernel stack and stores the user process registers.

## Getting Into Kernel

When a user program enters the kernel through an interrupt, exception, or system call, the operating system saves the registers of the currently executing process in the kernel stack. It switches `rsp` from the user stack to the kernel stack top and pushes registers.

At the beginning, the `rsp` register points to the top of the user stack. The entry path switches the stack pointer from the user stack to the kernel stack. Then execution starts in the interrupt or system call handler.

Inside the interrupt or system call entry path, Pintos saves general-purpose registers and related fields. After saving the registers used by the user process to the kernel stack, `rsp` points to the top of the interrupt frame.

Entering the kernel is expensive. It switches the stack pointer and saves many registers. Some parts are executed by hardware in a single instruction, but many registers still need to be saved to the kernel stack by software.

## Back to Loading

`process_exec()` passes the program name to `load()`. Then `load()` finds the executable file using that file name and loads it into memory.

The file name is the name of the file we want to load. The entry point is the starting address of the main function that needs to be executed after the program is loaded. `load()` is responsible for initializing this entry-point field. It is also responsible for initializing the `rsp` field of the user stack. This field contains the user stack-top address when the operating system starts executing the program.

The body of `process_exec()` has three important steps. First, it loads the executable and initializes the user stack. Second, before executing the user program, it passes a set of arguments to the user process. This part is what argument passing extends. Third, it gets out of the kernel and jumps to the user program that should be executed.

Before writing the missing stack setup code, you have to understand how loading and getting out of the kernel work.

## Getting Out of the Kernel

Getting out of the kernel consists of two basic assembly instructions: `mov` and `jmp`.

With `movq`, Pintos sets the stack pointer to point to the current stack top of the interrupt frame. Then it calls `do_iret()`.

In `do_iret()`, Pintos restores registers and calls the `iretq` instruction. As a result, `rsp` points to the top of the interrupt frame. The code restores the saved registers, and then `iretq` restores the CPU-defined state and changes the execution mode from kernel mode to user mode.

After executing `iretq`, the saved state is restored to the CPU, and `rsp` is set to the stack-top address of the user stack. This is how Pintos gets into and out of the kernel.

There is one thing to remember. When a thread is first created, its interrupt frame is empty. When a process enters the kernel, it fills the interrupt frame with values. When entering the kernel, registers are saved through the entry path. When getting out of the kernel, `iretq` is used.

However, a newly created process has never been in user space. Its interrupt frame is empty. For the process to be created and then leave the kernel for user space, the operating system has to initialize the interrupt frame with correct values. This is what `process_exec()` does before calling `do_iret()`.

## Write a Function That Sets Up a Stack

Before actually jumping into the function you want to execute, you first have to set up the user stack with a proper list of parameters. This is the function we are going to write.

Consider the user address space. There is a text region, data region, BSS region, and stack. Assume that the `rsp` field of the interrupt frame contains the stack-top address. In `process_exec()`, you write code to set up the user stack so that when the process resumes its control, it can read parameters and run the program.

The stack-top address is currently saved in the `rsp` field of the interrupt frame. What you are supposed to do is place the parameters from the stack top one by one.

## x86-64 Calling Convention

Consider a command line containing four arguments, indexed from zero to three. In the x86-64 calling convention, you have to place arguments in a specific order.

First, place the character strings on the stack. Then push the address of each string plus a null pointer sentinel in right-to-left order. Before the first push, round the stack pointer down to a multiple of 8. Then point `%rsi` to `argv`, set `%rdi` to `argc`, and finally push the fake return address.

These are the rules you have to follow when pushing arguments to the user stack.

## User Stack Layout in Function Call

Assume the number of arguments is four, with indexes zero through three. Before pushing parameters, the stack pointer represents the stack top.

Now we push the individual strings from right to left. For example, we push `bar\0`, then `foo\0`, then other argument strings, and then the program name such as `/bin/ls\0`.

The important thing is that the stack has to be aligned. In KAIST Pintos, round the stack pointer down to a multiple of 8 before pushing the `argv` pointers.

Then push a null pointer, zero, which means this is the end of the argument string pointer array. After that, push the address of each character string in reverse order: argument 3, argument 2, argument 1, and argument 0.

Then store the starting address of this parameter set. This is the `argv` address. Next, set `%rsi` to the `argv` address and `%rdi` to the number of arguments, such as 4. Then push the fake return address.

In this case, there is no real return address because this is a newly created process. Once it is over, there is nowhere to return; the thread just finishes. Therefore, push zero as a fake return address.

Once you completely implement this function, you can check whether all stack frames are set up properly by using the `hex_dump()` function provided by Pintos. With `hex_dump()`, you can dump the hex map of the interrupt frame and determine whether your stack has been properly set up.

This is the end of setting up the user stack.
