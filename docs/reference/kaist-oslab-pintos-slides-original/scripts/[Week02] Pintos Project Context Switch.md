# [Week02] Pintos Project Context Switch

Source: https://youtu.be/mtX-bj1Fu6M?si=vCRv4Xu8zDLXLfbX

## Intro

In this class, I am going to explain the details of context switch in Pintos operating system.

This is an overview of the talk. We will talk about process structure, process state, process context, and scheduling and switching the two threads.

## Process Structure: `struct thread`

The first topic we are going to deal with is `struct thread`, or the structure that represents a process. This is one of the most important data structures in the operating system kernel.

This is `struct thread`. It contains various attributes that are required to represent a thread. The first one is thread id, the status of the thread, thread priority, and then it contains a pointer to the kernel stack. I will explain it later. Then it contains the pointer that points to the page table it uses.

## Process State

There are four process states in Pintos operating system. There is running, ready, blocked, and those are three basic process states. Then there is another state called dying. This is the state where a process stays after it calls exit and before the operating system completely removes all the data structures that have been allocated for the process.

The most important function for a thread is creating a thread. This is the prototype of `thread_create()`. It has four attributes: name, thread priority, the function it wants to execute when the thread is called, and the parameter list that needs to be supplied to the function you want to execute.

This is the detail of creating a thread. Inside `thread_create()`, it calls a function called `init_thread()`. In `memset()`, it allocates memory for the `struct thread` and sets it to zero. After allocating the structure, the operating system sets the initial state of the thread to blocked. It puts the just-created thread structure to a list called all list that I am going to explain later.

## Process List

In Pintos, there are two lists of processes or lists of threads. The first one is ready list, and the second one is all list.

Ready list is a set of threads that are ready for execution. All list is a set of all threads in the system. In `thread_create()`, it is first inserted into the list called all list, and then all the threads that are ready for CPU execution are inserted into the ready list.

## Creating a Thread

Creating a thread first inserts the created thread structure to all list. After putting it into all list, the operating system initializes the various fields in the thread structure, sets the state of the thread to ready, and then puts it in the ready list.

In the current Pintos operating system, whenever the operating system inserts the thread structure to the ready list or all list, it calls `list_push_back()`. It means that the operating system always places the just-created thread at the very end of the list.

Let me explain the function `schedule()`. It is a very important function. Let me put three exclamation marks. It is a function which the operating system calls when it needs to schedule a new thread.

Scheduling a new thread means the operating system kicks out the currently running thread, finds the next thread to run, and puts it in the CPU. Then who calls `schedule()`? The question is equivalent to saying: how does the process release CPU?

There are two reasons. The first one is voluntary release, and the other one is involuntary release. In voluntary release, the existing process calls exit, blocks, or calls yield. In involuntary switch, the currently running process is involuntarily replaced with the other process. That happens when the higher priority process has arrived or when the time quantum it has been using has expired.

Before calling `schedule()`, we have to disable interrupts, and then we have to change the state of the running thread from running to something else. Something else means that when `schedule()` is called in block, then the status goes to blocked.

There are three reasons where `schedule()` is called: when the process exits, when the process waits for an event or blocks, or when the process calls yield.

In the current Pintos operating system, there is no way for an operating system to switch out the existing process unless the process wants to. In current Pintos operating system, there is no preemption. This is non-preemption. In the course of doing the first project, we are going to turn this operating system into a preemptive operating system.

In preemptible Pintos, you are going to implement a scheduling module that kicks out the existing process if the newly incoming process has a higher priority.

This is actual code of Pintos. This is `thread_block()`. This function is called when a thread is asking for I/O completion, and then it calls `thread_block()`. Inside `thread_block()`, it puts the state of the process in `THREAD_BLOCKED`, and then it calls `schedule()`. We are going to deal with the scheduling detail, so please bear with me for now.

The second topic is `thread_yield()`. When the running process wants to hand over the CPU to another process, then it calls yield. In yield, the currently running process puts itself to the end of the ready queue and hands over the existing CPU to the other process. In that case, the existing process state becomes `THREAD_READY`.

In the third case, when the process exits, after process exit it changes state to `THREAD_DYING`, and then it calls `schedule()`.

These are the three situations where `schedule()` is called. Remember, in all these states, the process voluntarily relinquishes CPU.

## `schedule(void)`

Now let us look at the details of the `schedule()` function.

It gets the pointer to the current thread, and then it gets the pointer to the next thread to run. This is where the scheduling discipline comes into play. For example, you may want to use first-come, first-served, shortest job first, shortest time to completion algorithm, or round robin.

In all these scheduling algorithms, the operating system selects what is the next thread to run. In `next_thread_to_run()`, the operating system selects the next thread to run and returns the pointer to the just-selected thread structure.

After getting the pointers to the current running thread and the next thread to run, it calls a function called `switch_threads()`. The function `switch_threads()` is responsible for saving the current context and restoring the next context of the next thread to run into the CPU registers. It returns a pointer to the thread structure into the variable called `prev`.

After it is returned to `prev`, the operating system calls `thread_schedule_tail()`, which puts the thread structure pointed by `prev` at the end of the ready list.

## Stack

Now I am going to explain the details of thread switch. As the first step, I am going to explain what the stack is.

Stack is a very primitive data structure. It contains operations push and pop. As you push the items to the stack, the stack top pointer grows. As you pop the data from the stack, the pointer grows the other end.

There is an important register. We call it `esp`, a stack pointer, and it points to the top of the stack. Usually stack grows to the lower address space.

`push abc` stores the value at the address pointed by `abc` into the stack top and increases the stack pointer by four. But increasing the stack pointer by four corresponds to decreasing the `esp` value by four bytes.

The next operation is `pop abc`. It retrieves the value pointed by the `esp` pointer and saves it to the location pointed by `abc`. Then it decreases the stack pointer by 4 bytes. That is pop. Push and pop are the two basic operations in stack.

Virtual address space of a process is partitioned into two configured regions: user space and kernel space. In Pintos, virtual address space from zero to three gigabytes forms user space, and virtual space beyond the three gigabytes range forms kernel address space.

To access the kernel address space, the process needs to change its execution mode to the kernel mode. It is achieved by increasing its privilege level.

When the process runs in user mode, it uses user stack to call a function and to define the local variables. When it switches to the kernel and executes in the kernel, it uses the kernel stack to use the function call. There is a kernel stack, and there is a user stack.

## `switch_threads(struct thread *cur, struct thread *next)`

I am going to explain the detailed steps of `switch_threads()`.

The `switch_threads()` function switches the two threads pointed by `cur` and `next`. `cur` represents the currently running thread, and `next` represents the thread to run.

It basically consists of four steps. First, it saves the registers to the kernel stack. Then it saves the location of the current stack top at the current thread's stack attribute. Then it restores the new thread's stack top into CPU stack pointer. Using the stack pointer just newly established, it restores the registers from the stack to the CPU.

## Call `switch_threads()`

This represents the current stack and current stack top register value right after it calls `switch_threads()`. There are two pointers, `cur` and `next`. They point to the thread structure of the currently running thread and the next thread to be run.

Then there are two stacks. This is the kernel stack of the current thread, and then it is the kernel stack of the next thread. Both of them are in the kernel stack region. The stack pointer `esp` points to the stack top of the current stack.

This is the state of the memory layout right as `switch_threads()` is called. It just jumps to the `switch_threads()` function.

After jumping into `switch_threads()`, it stores the four CPU registers to the kernel stack of the current thread. After pushing `ebx`, `ebp`, `esi`, and `edi` registers, the stack pointer increases to the new stack top. To push four registers, the `switch_threads()` function executes four instructions: `ebx`, `ebp`, `esi`, and `edi`.

After executing these four instructions, it pushes the four registers of the currently running thread into the stack top, into the current kernel stack, and the kernel stack is changed to point to the newly established stack top.

After storing the four registers to the kernel stack, it is time to save the kernel stack-top address to the thread structure of the currently running thread.

In this figure, this stack attribute is set to point to the stack top of kernel stack. It contains this stack top address of the `esp` register.

The first thing it needs to do is to load the offset of stack attributes to the register `edx`. Stack attribute is 24 bytes apart from the beginning of the thread structure, so we first have to identify the amount of offset it has to jump from the beginning of the `struct thread`. It is specified as this statement, and its macro `thread_stack_ofs` represents the offset of stack attributes from the beginning of this thread structure. This is 24. After executing this statement, `edx` contains the value 24.

The next step is to load the beginning address of current `struct thread` to the `eax` register. The next thing the operating system does is to save the location of current `struct thread` to `eax` register. But how do we know the current location of current `struct thread`?

If you look at the stack of `cur`, there is a field `cur` that saves the location of current thread structure, and it is 20 bytes apart from the `esp` structure. By a `switch_threads()` call, this represents 20. From this point, if you go up for 20 bytes, then that location contains the address of current `struct thread` data structure. By executing this statement, the `eax` register contains the address of current thread structure.

As a next step, we save the current thread's stack top address to the thread's stack field. We move the value of `esp`, the stack pointer, to this address, where `eax` points to the beginning of this `struct thread` data structure, and `edx` represents the offset between the beginning of the thread structure and the location of the stack. There we save the value of `esp`.

As a result, the pointer, the stack field, contains the address of the current stack top. It looks like this.

## Switch Kernel Stack

The next step is to switch the stack pointer to point to the stack top of the next thread structure.

Switching the stack pointer from the stack top of the current thread to the stack top of the next thread consists of two steps. First, it has to identify the location of the thread structure of the next thread. To do that, from this point, it goes up for 24 bytes and identifies the location of the next thread structure.

After identifying the location of next thread structure, it identifies the offset between the beginning address of the thread structure and where the stack resides. This is 24 bytes. After that, you can retrieve the location of the stack pointer from the thread structure. Then the stack top pointer has successfully changed from the current thread to the new thread.

If you look at the data structure, the first instruction to identify the location of the next thread structure goes up by 24 bytes from the stack pointer. This is the location of next thread structure, and it contains the address of the thread structure of the next thread that has been supplied by the caller to `switch_threads()`.

After loading the location of next thread structure to `ecx`, it adds the offset of stack field to this base location, and then it saves the address value located at that address to `esp`. Then `esp` points to the new stack top, which is the stack top of the next thread.

## Restore the New Context

After switching the stack pointer to the new stack top, it pops four register values and restores them to the four registers as `edi`, `esi`, `ebp`, and `ebx`. As a result of popping four register values, the stack pointer is updated to this location.

It performs four pop instructions, and then kernel stack of the next thread is now updated. This is how we switch the two threads.

After switching the current thread and the next thread, the operating system updates the state of the newly selected thread as running. If the previous thread was in the dying state, the operating system has to clean up all the pages allocated to that thread.

## Change the State of New Current

This is the detail of the code of `thread_schedule_tail()`. It updates the newly running thread as `THREAD_RUNNING`. If the caller of `switch_threads()` was in dying state, then it has to deallocate all the pages allocated to the dying state process and free it.

## Summary

In this video, we have explained the details of context switch in Pintos operating system.

The `schedule()` function is called in exit, yield, and block, and it gets a new process to the CPU.

There are four important steps in context switch. It saves the context of the currently running thread to the stack, and then saves the current stack top at the currently running `struct thread`. Then it switches the stack top register, pointing to the stack top of the next thread, and then it restores the context from the stack to the CPU.

After switching the two threads, it updates the state of the next running process and frees the memory from the dying process.
