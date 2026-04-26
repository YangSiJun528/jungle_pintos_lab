# [Week02] Pintos Project1-2 BSD

Source: https://youtu.be/4-OjMqyygss?si=uy9Q-H-kDi0U7rD1

## Intro

In this video, I am going to explain another topic on implementing the scheduler. The title of this talk is about a 4.4BSD-like scheduler.

Our main goal is to implement the 4.4BSD scheduler, which is MLFQS, a multilevel feedback queue scheduler. The 4.4BSD scheduler gives priority to the processes with interactive nature. It is basically a priority-based scheduler. It uses equations to compute the priority.

## Nice

The most important concept in the BSD scheduler is the concept of nice. It is an integer value, and it represents the niceness of a thread. If a thread is nicer, it means that the thread is willing to give up some of its CPU time.

In Pintos, nice value ranges from -20 to 20. A nice value of 0 is default, and it does not influence priority. If nice is positive, then it decreases the priority. If nice is negative, then it increases the priority of a process.

There are two functions for adjusting and getting nice value. The first one is `thread_get_nice()`, and the second one is `thread_set_nice()`.

## Priority

In Pintos, the priority of a process ranges from 0 to 63. It is an unsigned integer, minimum being 0 and maximum being 63. The larger the number, the higher the priority. When the thread is initialized, the priority of the thread is set to 31.

The priority of a process is computed as:

```text
priority = PRI_MAX - (recent_cpu / 4) - (nice * 2)
```

I am going to explain the details of this equation. This equation is very simple, but it is profound.

There are four principles behind this equation. First, if the thread is nicer, it lowers its priority. If the thread has been using lots of CPU recently, it lowers its priority. There is an important word here: recently. How does the CPU scheduler take into account how the CPU has been used by the process recently?

Later, for all threads, priority is recalculated once in every fourth clock tick, and the result is truncated to the nearest integer. This statement sounds pretty obvious, but it is not. The reason is that `PRI_MAX` is an integer, and then `recent_cpu / 4` and `nice` are not integer numbers. They are floating-point numbers. So we have to have some rules to map the floating-point number to the integer value, and the result is truncated to its nearest integer.

## `recent_cpu`

First, we are going to talk about the concept of `recent_cpu`. It represents how much of the CPU cycles the process has been using. Timer interrupt increases `recent_cpu` of the currently running process by one in every timer interrupt.

In the previous slide, using CPU recently lowers the priority. Even though the process has been using CPU a lot of times long before, then we have to have a mechanism to discount that fact. So we bring the concept of decay.

Decay decreases the amount of `recent_cpu` value by a certain decay factor every second. Of course, the decay is less than one. We adjust `recent_cpu` by nice every second. So in every second, we add nice value to `recent_cpu` and set it to the new value of `recent_cpu`.

Putting it all together, `recent_cpu` is computed by decay times previous `recent_cpu` plus nice value every second:

```text
recent_cpu = decay * recent_cpu + nice
```

In System V Release 3, the decay factor was 0.5. In 4.4BSD, it incorporated a more sophisticated mechanism. In heavy load, the CPU scheduler makes the decay factor nearly one. In light load, decay factor converges to zero.

To achieve this objective, it comes with this very interesting formula:

```text
decay = (2 * load_avg) / (2 * load_avg + 1)
```

If `load_avg` is large, then this value converges to one. If `load_avg` is nearly zero, then this value converges to zero.

## `load_avg`

Then what is `load_avg`? `load_avg` represents how busy the system is. At the booting time, `load_avg` is initially set to zero.

`load_avg` is a weighted average of `load_avg` and ready threads. Ready threads is the number of threads in the ready list, plus threads executing at the time of an update. That represents the number of threads in the system.

`load_avg` is computed by:

```text
load_avg = (59 / 60) * load_avg + (1 / 60) * ready_threads
```

All of these values play a very critical role in determining the fairness, efficiency, and performance through the BSD CPU scheduler algorithm. But we are not going to get into details about how to set this value.

## Summary of Rules

In summary, we can obtain the following rules.

First, in every fourth tick, we need to recompute the priority of all threads as follows:

```text
priority = PRI_MAX - (recent_cpu / 4) - (nice * 2)
```

In every clock tick, we increase the running thread's `recent_cpu` by one.

In every second, update every thread's `recent_cpu` as follows:

```text
recent_cpu = decay * recent_cpu + nice
```

The decay factor and `load_avg` are computed as follows:

```text
decay = (2 * load_avg) / (2 * load_avg + 1)
load_avg = (59 / 60) * load_avg + (1 / 60) * ready_threads
```

## Example

Let us provide an example. There are three processes, P1, P2, and P3. The initial value of nice is 0, and the initial value of `load_avg` is also 0.

At the first clock tick, the priority of all threads is 63, so process one is picked up by the scheduler. At clock ticks one, two, and three, the `recent_cpu` value increases by 1, 2, and 3.

At the fourth clock tick, the priority of P1, P2, and P3 is recalculated. Because `recent_cpu` becomes 4, the priority of process 1 becomes 62 from 63. Then it compares the priority of the two other processes, P2 and P3. The rest of the two processes get priority 63, and they have the higher priority. Due to that reason, process 2 is picked up, and then it gets executed. Its `recent_cpu` value also increases four times from zero to one, two, three, and four.

At this point, the priority of P2 is updated to 62. Therefore, the priority of P3 is 63, and it gets the highest priority, so it gets the CPU.

At the same time, let us look at how `recent_cpu` gets reset. `recent_cpu` is computed like this:

```text
recent_cpu = (2 * load_avg) / (2 * load_avg + 1) * recent_cpu + nice
```

Basically, if you consider all these, the `recent_cpu` values of the processes will become zero to four at this time period. As a result, according to this priority mechanism, P1, P2, and P3 get executed in a round-robin manner if they all require CPU.

## Fixed-Point Arithmetic

There is one important thing to do. You need to implement fixed-point arithmetic. The reason is that inside the kernel, you can do only integer arithmetic. The kernel does not have floating-point registers when switching the thread context. So you need to implement fixed-point arithmetic using integer arithmetic.

Priority, nice, and ready thread values are integers. However, the `recent_cpu` and `load_avg` values are real numbers.

We are going to use the 17.14 fixed-point number representation. In this representation, the decimal point uses the 14 rightmost bits, and integer uses the next 17 bits to the left. The last left bit, one bit, is the sign bit.

This is how it looks. This is total 32 bits. From bit 0 to 13, this is the fractional part. The leftmost bit represents sign. This one represents the fixed numbers. This is integer part, this is the fractional part, and this is sign.

These are the rules. We need these functions:

- Convert `n` to fixed point.
- Convert fixed point to integer.
- Convert `x` to integer.
- Add two values where `x` and `y` are fixed-point numbers and `n` is an integer.
- Subtraction.
- Addition.
- Addition between fixed-point numbers and integer numbers.
- Subtraction, multiplication, and division.

You need to implement all these functions by yourself, and then use proper functions to do the arithmetic.

## Basic Implementation

This is the basic implementation. The first thing you have to do is add nice and `recent_cpu` fields to `struct thread`.

Then you need all these functions. First, you need a function that calculates priority using `recent_cpu` and nice. You also need a function to calculate `recent_cpu` and `load_avg`. You need a function to increase `recent_cpu` by one. Also, you have to recalculate the priority and `recent_cpu` of all threads.

If you use this simple equation, then you may not need multiple queues to implement multilevel feedback. This simple equation-based CPU scheduler achieves the same objective as multilevel feedback queues. It gives priority to the interactive jobs, and it gives priority to the I/O-intensive jobs.

## Functions to Modify

These are functions to modify.

In `init_thread()`, you have to initialize your nice value and `recent_cpu`. In `thread_set_priority()`, you disable the priority setting when using advanced scheduler.

You have to adjust the timer interrupt function. In timer interrupt function, you recalculate `load_avg`, `recent_cpu` of all threads, and also priority every one second. You have to recalculate priority of all threads in every fourth tick.

Please disable priority donation when using advanced scheduler, both in `lock_acquire()` and `lock_release()`.

These are the functions you need to modify to implement the BSD-like scheduler.

There is a function called `thread_set_nice()`. It sets the nice value of the current thread. There is also a function `thread_get_nice()`. It returns nice value of the current thread.

You implement `thread_get_load_avg()`. It returns `load_avg` multiplied by 100. Also, you write `thread_get_recent_cpu()`. It returns `recent_cpu` multiplied by 100.

Once you have completely implemented these features, then you should be able to pass the tests.
