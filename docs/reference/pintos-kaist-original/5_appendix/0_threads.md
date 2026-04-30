# Threads

## `struct thread`

The main Pintos data structure for threads is `struct thread`, declared in `threads/thread.h`.

* * *
    
    
    struct thread;
    

Represents a thread or a user process. In the projects, you will have to add your own members to `struct thread`. You may also change or delete the definitions of existing members. Every `struct thread` occupies the beginning of its own page of memory. The rest of the page is used for the thread's stack, which grows downward from the end of the page. It looks like this:
    
    
                          4 kB +---------------------------------+
                               |         kernel stack            |
                               |               |                 |
                               |               |                 |
                               |               V                 |
                               |        grows downward           |
                               |                                 |
                               |                                 |
                               |                                 |
                               |                                 |
                               |                                 |
                               |                                 |
                               |                                 |
                               |                                 |
        sizeof (struct thread) +---------------------------------+
                               |             magic               |
                               |          intr_frame             |
                               |               :                 |
                               |               :                 |
                               |             status              |
                               |              tid                |
                          0 kB +---------------------------------+
    

This has two consequences. First, `struct thread` must not be allowed to grow too big. If it does, then there will not be enough room for the kernel stack. The base `struct thread` is only a few bytes in size. It probably should stay well under 1 kB. Second, kernel stacks must not be allowed to grow too large. If a stack overflows, it will corrupt the thread state. Thus, kernel functions should not allocate large structures or arrays as non-static local variables. Use dynamic allocation with `malloc()` or `palloc_get_page()` instead (see [Memory Allocation](https://casys-kaist.github.io/pintos-kaist/appendix/memoryAllocation.md)).

* * *
    
    
    tid_t tid;
    

> The thread's thread identifier or tid. Every thread must have a tid that is unique over the entire lifetime of the kernel. By default, `tid_t` is a `typedef` for `int` and each new thread receives the numerically next higher tid, starting from 1 for the initial process. You can change the type and the numbering scheme if you like.

* * *
    
    
    enum thread_status status;
    

> The thread's state, one of the following:
> 
>   - `THREAD_RUNNING`
> 
> The thread is running. Exactly one thread is running at a given time. `thread_current()` returns the running thread.
> 
>   - `THREAD_READY`
> 
> The thread is ready to run, but it's not running right now. The thread could be selected to run the next time the scheduler is invoked. Ready threads are kept in a doubly linked list called `ready_list`.
> 
>   - `THREAD_BLOCKED`
> 
> The thread is waiting for something, e.g. a lock to become available, an interrupt to be invoked. The thread won't be scheduled again until it transitions to the `THREAD_READY` state with a call to `thread_unblock()`. This is most conveniently done indirectly, using one of the Pintos synchronization primitives that block and unblock threads automatically (see [Synchronization](1_synchronization.md)). There is no *a priori* way to tell what a blocked thread is waiting for, but a backtrace can help (see [Backtraces](0_threads.md)).
> 
>   - `THREAD_DYING`
> 
> The thread will be destroyed by the scheduler after switching to the next thread.

* * *
    
    
    char name[16];
    

> The thread's name as a string, or at least the first few characters of it.

* * *
    
    
    struct intr_frame tf;
    

> Storing information for the context switching, which includes registers, and stack pointer.

* * *
    
    
    int priority;
    

> A thread priority, ranging from `PRI_MIN` (0) to `PRI_MAX` (63). Lower numbers correspond to lower priorities, so that priority 0 is the lowest priority and priority 63 is the highest. Pintos as provided ignores thread priorities, but you will implement priority scheduling in project 1 (see [Priority Scheduling](../1_project1/2_priority_scheduling.md)).

* * *
    
    
    struct list_elem elem;
    

> A "list element" used to put the thread into doubly linked lists, either `ready_list` (the list of threads ready to run) or a list of threads waiting on a semaphore in `sema_down()`. It can do double duty because a thread waiting on a semaphore is not ready, and vice versa.

* * *
    
    
    uint64_t *pml4;
    

> Only present in project 2 and later. See [Page Tables](3_virtual_address.md).

* * *
    
    
    unsigned magic
    

> Always set to `THREAD_MAGIC`, which is just an arbitrary number defined in `threads/thread.c`, and used to detect stack overflow. `thread_current()` checks that the `magic` member of the running thread's `struct thread` is set to `THREAD_MAGIC`. Stack overflow tends to change this value, triggering the assertion. For greatest benefit, as you add members to `struct thread`, leave `magic` at the end.

## Thread Functions

`threads/thread.c` implements several public functions for thread support. Let's take a look at the most useful:

* * *
    
    
    void thread_init (void);
    

> Called by `main()` to initialize the thread system. Its main purpose is to create a `struct thread` for Pintos's initial thread. This is possible because the Pintos loader puts the initial thread's stack at the top of a page, in the same position as any other Pintos thread.
> 
> Before `thread_init()` runs, `thread_current()` will fail because the running thread's `magic` value is incorrect. Lots of functions call `thread_current()` directly or indirectly, including `lock_acquire()` for locking a lock, so `thread_init()` is called early in Pintos initialization.

* * *
    
    
    void thread_start (void);
    

> Called by `main()` to start the scheduler. Creates the idle thread, that is, the thread that is scheduled when no other thread is ready. Then enables interrupts, which as a side effect enables the scheduler because the scheduler runs on return from the timer interrupt, using `intr_yield_on_return()`.

* * *
    
    
    void thread_tick (void);
    

> Called by the timer interrupt at each timer tick. It keeps track of thread statistics and triggers the scheduler when a time slice expires.

* * *
    
    
    void thread_print_stats (void);
    

> Called during Pintos shutdown to print thread statistics.

* * *
    
    
    tid_t thread_create (const char *name, int priority, thread func *func, void *aux);
    

> Creates and starts a new thread named name with the given priority, returning the new thread's tid. The thread executes func, passing aux as the function's single argument.
> 
> `thread_create()` allocates a page for the thread's `struct thread` and stack and initializes its members, then it sets up a set of fake stack frames for it. The thread is initialized in the blocked state, then unblocked just before returning, which allows the new thread to be scheduled.

* * *
    
    
    void thread_func (void *aux);
    

> This is the type of the function passed to `thread_create()`, whose aux argument is passed along as the function's argument.

* * *
    
    
    void thread_block (void);
    

> Transitions the running thread from the running state to the blocked state. The thread will not run again until `thread_unblock()` is called on it, so you'd better have some way arranged for that to happen. Because `thread_block()` is so low-level, you should prefer to use one of the synchronization primitives instead (see [Synchronization](1_synchronization.md)).

* * *
    
    
    void thread_unblock (struct thread *thread);
    

> Transitions thread, which must be in the blocked state, to the ready state, allowing it to resume running. This is called when the event that the thread is waiting for occurs, e.g. when the lock that the thread is waiting on becomes available.

* * *
    
    
    struct thread *thread_current (void);
    

> Returns the running thread.

* * *
    
    
    tid_t thread_tid (void);
    

> Returns the running thread's thread id. Equivalent to `thread_current ()->tid`.

* * *
    
    
    const char *thread_name (void);
    

> Returns the running thread's name. Equivalent to `thread_current ()->name`.

* * *
    
    
    void thread_exit (void) NO_RETURN;
    

> Causes the current thread to exit. Never returns.

* * *
    
    
    void thread_yield (void);
    

> Yields the CPU to the scheduler, which picks a new thread to run. The new threadmight be the current thread, so you can't depend on this function to keep this thread from running for any particular length of time.

* * *
    
    
    int thread_get_priority (void);
    void thread_set_priority (int new_priority);
    

> Stub to set and get thread priority. See [Priority Scheduling](https://casys-kaist.github.io/pintos-kaist/project1/priority_scheduling).

* * *
    
    
    int thread_get_nice (void);
    void thread_set_nice (int new_nice);
    int thread_get_recent_cpu (void);
    int thread_get_load_avg (void);
    

> Stubs for the advanced scheduler.
