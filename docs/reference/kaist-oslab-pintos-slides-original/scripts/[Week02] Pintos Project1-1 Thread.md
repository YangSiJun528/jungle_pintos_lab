# [Week02] Pintos Project1-1 Thread

Source: https://youtu.be/myO2bs5LMak?si=3rbvoNu2h8X5hjnz

## Intro

This is the explanation on the first project. The title of the project is Threads. In this project, we are going to implement three main topics. The first one is Alarm Clock, the second one is Priority Scheduling, and the third one is Advanced Scheduler.

## Overview

The first topic is Alarm Clock. We are going to modify the system called timer alarm. Timer alarm is the system that wakes up a process after a certain amount of time.

Pintos currently uses busy waiting for alarm, and we are going to modify Pintos to use the sleep and wakeup paradigm for alarm. This is how `timer_sleep()` is implemented in current Pintos. In the current `timer_sleep()` implementation, the process switches between ready state and running state to check if the alarm time has come or not. As a result, it only uses two states, and it keeps crunching CPU instructions waiting for CPU cycles.

From a data structures point of view, the thread is put into running mode, and if it calls alarm, then it puts itself back to the end of the ready list and then executes again. Basically, in the `timer_sleep()` implementation, it keeps executing this loop.

This is how `timer_sleep()` is implemented. If you look at this code, first, when `timer_sleep()` is called, it records the current time. Then within the while loop, it keeps executing `thread_yield()`. So it releases CPU, and then when it has been put into the running state, it checks the time. If the elapsed time is less than ticks, then it releases CPU again. Basically, the process that calls `timer_sleep()` switches between ready state and running state.

Let us look at the details of `thread_yield()`. It is worth looking at the details. First, it gets the pointer to the current thread structure, and then it disables interrupts. Then it puts the current thread structure to the end of the ready list. Then it changes the state of the currently running thread to `THREAD_READY`, and then it calls context switch. After context switch finishes, it sets the interrupt level back to its original state.

## Functions in `thread_yield()`

There are five essential functions inside `thread_yield()`.

First one is `thread_current()`. It returns the current thread pointer. Then it disables interrupts. There is another function which is the counterpart of interrupt disable: `intr_set_level()`. It restores interrupt level.

Then `list_push_back()` is a function that places the given object to the end of the specified list. The objective of this function is to put the current thread structure to the end of the ready list. Then it calls `schedule()` to do the context switch.

The objective of this project is to make `timer_sleep()` more efficient. We are going to introduce the blocked state. We are going to implement `timer_sleep()` using blocked state. When a process calls `timer_sleep()`, the operating system puts itself to the blocked state, and then the operating system is responsible for waking up the blocked process, frequently checking the time. Through this approach, the operating system can save CPU cycles and, more importantly, it can save power consumption.

## Design: Sleep/Wakeup-Based Alarm Clock

This is the design. As you see at the beginning of the code, there are only two lists in current Pintos. The first one is all list, and the other one is ready list. Ready list is a data structure that represents a set of processes that are waiting for CPU to be executed.

Now we are going to introduce a new data structure. This is the list of blocked threads. It represents a set of threads that are in the blocked state. When `timer_sleep()` is called, when a process calls `timer_sleep()`, the operating system puts the thread to the sleep list. Then the operating system keeps checking the timer, and when the timer is up, it wakes it up. Wakeup means moving the thread from the sleep list to ready list.

This is the design of our `timer_sleep()` algorithm. These are implementation details. First, we have to define the sleep queue. We have to define the sleep list. You can name it by yourself, but you have to define it as a sleep list. Then you have to initialize it. Of course, for every data structure, you have to initialize it properly. The point to think about is where we put the declaration statement and when we are going to initialize it. This part is left to you to implement.

## Global Tick vs. Local Tick

In implementing the blocked-state `timer_sleep()`, you have to introduce two things. The first one is global tick, and the second one is local tick.

The basic design idea is that every time the timer interrupt handler is executed, the kernel checks if there are any threads to wake up. To do that, it first has to scan the blocked thread list and find if any local tick for the blocked thread is greater than the global tick. In local tick, the thread maintains the time to wake up.

Currently our thread structure does not have this field, so we have to modify the thread structure to store the time to wake up. Then also, for efficiency's sake, we have to introduce one global variable. It is a global tick, or whatever you name the variable. It stores the minimum value of the local ticks of the threads.

Every time the timer interrupt handler is executed, it checks the tick, the global tick variable. If the current time is less than or equal to the tick, if the current time is greater than or equal to the tick, then it means that it has to wake up some threads in the blocked list. Otherwise, it does not have to scan all the blocked threads. The reason we introduced a global tick variable is to save the time to scan the sleep list.

## Modify Thread Structure

In the data structure, you have to add a new field that represents the alarm time to wake up.

This is the actual implementation of the code in `timer_sleep()`. Instead of putting itself in the while loop for busy waiting, if the time to wake up still has some time left until it has to wake up, then it puts itself into the sleep list by calling `thread_sleep()`. It passes the alarm time it needs to wake up as start plus ticks.

There is an interesting property here. It is possible that the value of start may become invalid at line two, because there are context switches in between. By the time it executes the statement, the time it has obtained may not be valid anymore when executing line number two. We are going to forget it for now, but if you are a really good system programmer, you may want to think about the way how to fix it.

## `thread_sleep()`

This is the body you want to implement in the `thread_sleep()` function. If the current thread is the idle thread, you have to change the state of the caller thread to blocked and store the local tick to wake up through its thread structure. You have to update the global tick if necessary, and then call `schedule()`. You should not forget to disable interrupts when you insert a thread structure through the thread list.

The important thing in `thread_sleep()` is changing the state of the caller thread to blocked and putting it into the sleep queue.

## Implementation of Alarm Clock

Timer interrupt implementation is the heart of everything. In timer interrupt, you have to modify the timer handler. Inside the timer interrupt handler, the operating system should determine which threads to wake up every time when the timer occurs.

When there are threads to wake up, you have to remove them from sleep queue and then insert them to the ready list. You are modifying the ready list and the sleep list. When you modify the sleep list and ready list, you should not forget to disable interrupts and enable interrupts before and after modifying the list. Also, you should not forget to change the state of the thread from sleep to ready when you put the threads in the ready list.

One thing you might remember is that depending upon how you organize the sleep list, the time to identify the threads to wake up may vary a lot. For example, this is sleep list. If you organize the sleep list, and if you sort the sleep list with respect to the alarm clock time, this is 10 and this is 101 and this is 105. This blocked list is sorted with ascending order of the alarm time. When you want to find the thread to wake up, you can start scanning from the beginning of the queue, and you can stop scanning until you find the last thread to wake up. However, if this list is not sorted, then you have to scan the entire list every time you need to find the thread to wake up.

This is the modified code for timer interrupt. You have to add this part. You check the sleep list, then check the global tick, and find any thread to wake up. Then you move them to the ready list if necessary, and then you update the global clock.

## Summary

In summary, you have to modify a few functions: `thread_init()`, `timer_sleep()`, and timer interrupt, to make your `timer_sleep()` function based upon the sleep and wakeup protocol.

## Design Tip for Modularization

There are a variety of ways to write your own code, but following the design suggestion for modularization, you might want to add four functions.

The first one is a function that sets the thread state to blocked and then inserts it to the sleep queue. The second function finds the thread to wake up from the sleep queue and wakes it up. Wake it up means putting the threads from the sleep queue to the ready list. Also, you might want to write a function that saves the minimum value of ticks that threads have. Lastly, you might want to write a function that returns the minimum value of the ticks.

After you write all the code, you have to check whether your code passes the alarm test. This is the result, and I hope you pass this test.

## Outline

The second topic is Priority Scheduling. Pintos uses FIFO scheduling. You are required to modify the Pintos scheduler for Priority Scheduling.

This is the result of what you are required to do. First, sort the ready list by the thread priority. Second, sort a wait list for the synchronization primitives such as semaphores, condition variables, and locks with respect to priority. We also have to implement preemption. The preemption point is when the thread is put into the ready list. It is important that you do not have to check preemptibility every time when timer interrupt is called. In this scheduler algorithm, the operating system checks preemption only when the new thread arrives to the ready list.

You have to modify two files. This is the design. When you examine the ready list and select the next thread to run, you get the thread with the highest priority. That is one thing. When there is a thread waiting for the lock, then when the lock becomes available, the operating system selects the thread with the highest priority. Those are the two things you have to implement.

## Three Things to Consider

There are three things to consider.

First, when selecting a thread to run in the ready list, we have to select the one with the highest priority.

Second, when inserting a new thread to the ready list, the operating system is required to compare the priority of the running thread with the existing one, and we have to schedule a newly inserted thread if it has a higher priority than the currently running thread.

Third, the same rule applies to the set of threads waiting for a synchronization primitive such as locks, semaphores, and condition variables. When the lock becomes available, or semaphore or condition variable is available, the operating system selects the thread with the highest priority.

## Priority in Pintos

Let us explain the priority in Pintos. In Pintos, the priority ranges from 0 to 63. There are 64 priority levels, and the larger the number, the higher the priority. The default priority is set when the thread is first created, and the default priority value is 31.

Pintos operating system provides two functions. The first one is `thread_set_priority()`, which sets a priority of a thread to a specified value, and the second one is `thread_get_priority()`, which gets a priority of a given thread.

In implementing a priority-based scheduling, we have to modify a few things. The first thing is that, inside `thread_create()`, we would like to maintain this ready list sorted with respect to the priority of threads in the ready list. When you insert a thread after creating the thread, you put the thread with respect to the order of the priority. This is going to be very expensive.

The second thing is, when the thread is added to the ready list, the operating system has to compare the priority of a newly incoming thread with the one that is being executed. If the newly incoming thread has a higher priority, then we have to call `schedule()` so that we can switch out the currently running thread and push the newly incoming thread to CPU.

## `thread_create()`

This is the code for `thread_create()`. After unblocking the thread, we have to compare the priorities of the currently running thread and the newly inserted one. If the newly arriving thread has higher priority, then the existing thread has to give up the CPU.

## Others to Modify

There are a few others to modify.

When the thread is unblocked from the ready state, when it is put in the ready state, it also inserts with the priority order. Also, when a thread calls `thread_yield()`, it also has to put itself to the ready list with respect to the priority order.

There is another function we have to modify. That is `thread_set_priority()`. `thread_set_priority()` changes the priority of a thread with the given priority value. In current Pintos, `thread_set_priority()` just simply sets the priority with a new priority value. But in the new modified algorithm, `thread_set_priority()` not only sets the priority value but also adjusts the location or position of the thread within the ready list, because ready list has to be sorted with respect to the priority value of the threads in the list.

## Hint: `thread_unblock()`

We are going to show the details of how we can modify the `thread_unblock()` code. When unblocking a thread, we are going to place the thread in the ready list with respect to its thread priority. In the existing code, the operating system puts the unblocked thread at the end of the ready list like this, `list_push_back()`. So we delete this code and put the newly unblocked thread in the ready list with respect to priority.

## Change the Synchronization Primitives

We have to change the synchronization primitives. There are locks, semaphores, and condition variables. When the lock, semaphore, or condition variable becomes available, we have to wake up the waiting thread with respect to the thread priority.

## FIFO Lock/Unlock in Priority-Less Pintos

This view graph shows how the lock is maintained in Pintos. Pintos operating system uses first-in, first-served in determining the lock holder.

Assume there are four threads A, B, C, and D. A is currently holding a lock and continues executing until this point. While A is holding a lock, B has made a request first, D has made a request next, and C has made a request third.

After A releases a lock at this time point, the operating system decides the thread to acquire the lock. By this time there are three threads waiting for the lock: B, D, and C. Among these three threads, D has the highest priority. However, even though D has highest priority, the operating system just removes the first thread in the list and assigns a lock. So the order in which the lock is acquired is B, D, and C.

Between B and D, there is priority inversion. The process with the higher priority is waiting for the process with lower priority. This happens because Pintos uses FIFO lock/unlock mechanism.

## Priority-Based Lock/Unlock

In a priority-based lock/unlock mechanism, the waiters acquire the lock based on priority. This is the same example. There are four threads A, B, C, and D. B has made a request, D has made a request, and C has made a request for the lock.

After A has released the lock, at this point there are three threads. Different from the previous slide, the waiting list here is ordered with respect to priority. When thread A releases the lock, the thread with the highest priority, thread D, gets the lock. After D releases the lock, the thread with the next highest priority, thread C, gets a lock, and B is the last to get a lock.

## Semaphore in Pintos

We are going to briefly introduce the basic functions in semaphore and condition variable, and point out what kind of functions we have to modify in semaphore and condition variable.

In semaphore, there are three functions. The first one is `sema_init()`, which initializes a semaphore to a given value. The second one is `sema_down()`. It requests for semaphore, and if it is acquired, then the process proceeds. But if the process fails to acquire semaphore, then it has to block. `sema_up()` releases the semaphore. Here you have to modify `sema_down()` and `sema_up()`.

For locks, there are three functions: `lock_init()`, `lock_acquire()`, and `lock_release()`. As you can see, lock is implemented by the semaphore. In order to modify the lock primitive based on priority, it is sufficient to modify semaphore.

## Condition Variable in Pintos

A third function is condition variable. There are four important functions in condition variable. The first one is `cond_init()`, which initializes the condition variable data structure. The second one is `cond_wait()`. Once a process calls `cond_wait()`, the process is put to the blocked state and waits for a signal by the condition variable. The third one is `cond_signal()`. It sends a signal to a thread of the highest priority waiting in the condition variable. Another function, `cond_broadcast()`, sends the signal to all threads waiting in the condition variable.

These are the functions to modify. There are two functions you have to modify. The first one is `sema_down()`, and the other is `cond_wait()`. Inside this code, you are required to modify the code so that when the process is put into the wait list, they have to be sorted with respect to priority. Also, you have to modify `sema_up()` and `cond_signal()`, as well as `thread_set_priority()`, so that all the lists can be ordered with respect to priority.

## Priority Inversion

There is an important issue you have to consider. The first one is priority inversion. Priority inversion is the one where a higher priority process is waiting for the process of low priority.

Consider this situation. There are three threads: thread A, thread B, and thread C. A is executing and has acquired a lock at this point. Then it continues executing. C is asking for the lock, but it is being held by A, so A is executing and C is blocked from this point. At this point, A has been executed until this point, and then B has arrived. Because B has a higher priority than A, B gets executed while A gets into the ready list.

Then there is an interesting thing that happens. Currently C is being blocked because it is waiting for A. However, A hands over the CPU to B because B has higher priority than A. The problem is the relationship between C and B. C has higher priority than B, but it turns out that C is waiting for B to finish. This is called priority inversion. In our priority scheduling, we have to fix this problem.

In 1997, Pathfinder on Mars stopped because operating systems crashed due to priority inversion. The engineers in NASA downloaded the source code of Pathfinder, and they identified that the crash was due to priority inversion. Then they found the patch, fixed the system, uploaded the code to Pathfinder, and made it work. This is the interesting story and importance of priority inversion.

There is one thing you will use to fix the priority inversion problem. This is called priority donation. Priority donation is the action of inheriting the priority of a process to the lock holder.

Let us say there are three threads again, thread A, thread B, and thread C. A is executing while holding a lock at this point. A is executing, and at this point C is asking for a lock, but the lock is being held by thread A. At that time, if you compare the priority of C and A, then A has low priority. When C is asking for a lock and finds that A is holding a lock, then C donates its priority to A so that A's priority gets boosted to C's level.

Right after A's priority has been boosted from lower level to C's level, you can see that B has arrived. In the original scheduling algorithm, B is supposed to preempt process A. However, in this case, process A's priority has been boosted to C's level, so there is no way for B to preempt process A. Process A continues executing at C's priority level, and once it is over, the lock is handed over to process C. Process C gets executed, and then after process C finishes execution, it releases the lock. Then process B finally gets an execution opportunity. Via donating priority to the lock holder, we avoid priority inversion. This technique is priority donation.

In the system without priority donation, there is a lock, L. Lock is currently allocated to thread one with priority 10. After thread one acquires the lock, three threads have arrived: thread two, thread three, and thread four. Each of these threads has priority 9, 12, and 8. Without priority donation, the priority of thread one remains as 10. But if we employ priority donation, then the lock holder's priority becomes the priority of the highest priority thread. Here there are three threads T2, T3, and T4. T2 has priority 9, T3 has priority 12, and T4 has priority 8. Among these threads, thread three donates its priority to thread one. This is priority donation.

## Nested Donation

There are a few issues to consider. The first one is nested donation.

Consider this scenario. There are three threads: thread one, thread two, and thread three. Thread one has priority 10, thread two has priority 9, and thread three has priority 7. Somehow, first thread one is holding lock A, and then thread two has made a request for lock A and it got blocked. However, thread two is holding lock B, and thread three is waiting for lock B to be released while holding lock C. There is a chained lock-holding relationship.

What if there comes thread four? It has priority 14 and it issues lock C. Thread four makes a request for lock C, and because of priority donation it will donate this priority to its lock holder T3 and make the priority of T3 from 7 to 14. However, here T3 again donates its priority to its lock holder, so the priority of T2 is updated from 9 to 14. Again, T2 donates its priority to its lock holder, and it donates priority 14 to its lock holder, so the priority level of T1 is updated from 10 to 14. This is called nested donation.

In your priority donation implementation, you have to implement the nested donation feature in your priority scheduling algorithm.

## Multiple Donation

The next topic is multiple donation. Thread one is holding three locks: lock A, lock B, and lock C. The original priority of thread one is 10. T2 makes a request for lock A. Its priority is 12, so thread T1 is donated priority 12. Then T3 makes a request for lock B, and it has priority 11, so it does not donate its priority to the lock holder because the current priority that has been donated by T2 is 12.

Now T4 makes a request for C, and T4's priority is 13. Priority 13 that is held by T4 is larger than priority 12, so finally the priority of T1 becomes 13.

Let us assume that T1 has unlocked lock C. As a result of unlocking lock C, the lock is allocated to T4. After releasing lock C, T1's priority should not become its original priority 10. The priority of T1 has to be updated to the largest priority that donated the priority to T1. Here, priority of T1 now becomes 12. That is multiple donation.

In your priority donation mechanism, you have to implement nested donation and multiple donation.

## Data Structure for Multiple Donation

The idea is simple. For supporting multiple donation, a thread has to maintain the list of donors. Every time it releases a lock, it searches the donors and gets the highest priority of the remaining donors.

## Data Structure for Nested Donation

For nested donation, you have to maintain the lock that it waits for. Once you inherit a priority, you have to check if you need to inherit the current priority to your child. That is the way you implement nested donation.

## Implementation of Priority Donation

These are the functions to modify in priority donation. You have to modify the data structure initialization, and you also have to modify `lock_acquire()`, `lock_release()`, and `thread_set_priority()`.

In `lock_acquire()`, if the lock is not available, you have to store the address of the lock. You have to store the current priority and maintain the donating threads in the list, and then you have to donate the priority.

When the lock is released, you have to remove the threads that hold the lock on the donation list, and you have to update the priority properly. Also, when you set the priority, you have to set the priority considering the donation. Those are the issues you have to modify. This is the result of the test.
