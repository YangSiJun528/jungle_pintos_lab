# [Week02] Scheduling MLFQ

Source: https://youtu.be/57r9OCN1EfA?si=z-hYixEB1a_zG6-d

## Intro

The next topic is the multi-level feedback queue, or MLFQ.

In shortest-job-first scheduling or round-robin scheduling, everything is fixed. There is only one queue of jobs in the system. Now we are going to discuss a new scheduler that learns from the past to predict the future, and that adjusts the priority of a job based on its behavior.

The objective is to give priority to a process that does not require lots of CPU time, and to lower the priority of a process that requires lots of CPU time. This minimizes response time without prior knowledge of a job's CPU time usage behavior.

## How MLFQ Works

A multi-level feedback queue has a number of distinct queues, for example from 30 to 64 queues. Each queue is assigned a different priority level. A job that is ready to run is on a single queue, so a job can be in any one of the queues.

Each queue is scheduled using round-robin scheduling.

There are a few rules to remember in MLFQ:

- If priority A is greater than priority B, then A runs.
- If two processes have the same priority, then they run in round-robin fashion.
- Jobs in the same queue have the same priority.

MLFQ varies the priority of a job based on its observed behavior. If a job repeatedly relinquishes the CPU while waiting for I/O, it keeps its priority high. If a job uses the CPU intensively for a long period of time, its priority is reduced.

For example, suppose there are eight priority queues in the system and four jobs. The highest priority is Q8 and the lowest priority is Q1. Jobs A and B are in Q8, job C is in Q4, and job D is in Q1. The scheduler runs the jobs in the highest non-empty priority queue first.

## Priority Adjustment

The priority adjustment algorithm has these rules:

- When a job first enters the system, it is placed at the highest priority.
- If a job uses up an entire time slice while running, its priority is reduced and it moves down to the next priority level.
- If a job gives up the CPU before the time slice is over, it stays at the same priority level.

Consider a single long-running job. Assume there are three priority queues and the time slice is 10 milliseconds. When the job arrives, it is inserted into the highest priority queue, Q2.

The job executes for the first 10 milliseconds. Because it used up its whole time slice, it moves down to the next priority level. It then executes for another 10 milliseconds. Once it uses that time slice too, it goes down to the lowest priority level, Q0. It stays there and continues executing at the lowest level.

## Time Quantum

Another issue is the time quantum. To react to changes in CPU utilization, MLFQ may assign different time quantum lengths depending on the priority of the queue.

For a high-priority queue, it uses a shorter time slice. For a lower-priority queue, it may use a longer time quantum.

In a system with three queues, the highest priority queue has a very short time slice length, while the lower priority queue has a longer time slice length.

## Solaris and FreeBSD

Solaris implements multi-level feedback using the time-sharing scheduling class. It has 60 queues, and it slowly increases the time slice length based on the priority of the queue. The highest priority queue has a 20 millisecond time slice, whereas the lowest priority queue has a time slice of a few hundred milliseconds. The priority of a job is boosted roughly every second.

FreeBSD is another way of implementing MLFQ. This is an interesting implementation because FreeBSD implements MLFQ without actual queues. Instead, it uses an equation. It computes the priority of a process based on how much CPU the process has used, the same idea as MLFQ. It boosts priority by decay, and it also takes the user's intention to yield CPU to other processes into account. For efficiency, it still uses queues.

In our Pintos project, we are going to implement the FreeBSD scheduler in detail. The details will be shown later in the class.

## Summary

MLFQ can be summarized as a set of rules:

- If a process has higher priority, it runs.
- If two processes have the same priority, they run in round-robin fashion.
- When a job enters the system, it is placed at the highest priority level.
- Once a job uses all of its time quantum at a given level, it goes down to the next priority level.
- After some period of time, move all jobs in the system to the topmost queue.

The benefit of MLFQ is that it does not require prior knowledge of a process's CPU behavior.
