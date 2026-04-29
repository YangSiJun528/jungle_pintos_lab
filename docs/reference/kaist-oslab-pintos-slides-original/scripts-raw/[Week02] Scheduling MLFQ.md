url: https://youtu.be/57r9OCN1EfA?si=z-hYixEB1a_zG6-d

-----------------
0:00
[Music]
0:01
next topic is
0:02
multi-level feedback queue
0:05
in
0:06
sure distraught first algorithm or
0:08
round-robin algorithm
0:09
everything is fixed in shortage of first
0:12
algorithm or random algorithm there is
0:14
only one queue of jobs in the system
0:18
now we are going to discuss the new
0:20
scheduler that learns from the past to
0:23
predict the future and that adjust the
0:25
priority of the algorithm based upon its
0:28
behavior
0:29
the objective is
0:31
for
0:32
the process that does not require lots
0:35
of cpu time it gives priority
0:38
for a process that requires lots of cpu
0:41
time it lowers the priority
0:44
it minimizes response time
0:47
basically
0:48
without the knowledge of the job's cpu
0:51
time usage behavior
0:54
here and how it works
0:56
multi-level feedback queue mlfq has a
0:59
number of distinct queues from 30 to 64.
1:03
and each queue is assigned a different
1:05
priority level so there is q
1:08
q1
1:10
2 q64 for example
1:13
and then the job is ready to run is on a
1:16
single queue so job can be anywhere in
1:19
one of the queues
1:22
and each queue is scheduled using the
1:25
round-robin scheduling algorithm
1:29
there's a few rules you need to remember
1:30
in mlfq
1:32
if priority a is greater than pre b then
1:35
a runs
1:37
and then if the
1:38
two priority two process has the same
1:41
priority then a and b runs in round
1:44
robin so
1:46
for the jobs on the same queue they have
1:48
the same priority
1:50
an mlfq varies the priority of the job
1:53
based on its observed behavior for
1:55
example if a drop repeatedly relinquish
1:58
the cpu rating for io it keeps its
2:01
priority high
2:03
if a drop uses intensively cpu for a
2:05
long period of time that reduces its
2:07
priority this is an example so there are
2:10
eight priority queues in the system and
2:12
there are four jobs uh highest priority
2:15
is q8 and the lowest priority is q1 and
2:18
a and b are in the q8 to the highest
2:21
priority queue and cr in c is in q4 and
2:25
d is in q1
2:27
there is other rules to you remember
2:30
that's the priority adjustment algorithm
2:33
when the job first centered the system
2:35
it placed at the highest priority
2:37
and if drop uses up an entire time slice
2:41
while it is running its priority is
2:43
reduced it goes on to the next priority
2:45
level
2:46
and then if a drop gives the cpu before
2:48
time slices up then it stays at the same
2:51
priority level
2:53
so let's consider the first sample this
2:56
is single
2:57
long running job
2:59
when the system and the job arrives the
3:01
system assume that there are three
3:03
priority queues it is inserted into the
3:06
highest priority q2
3:08
then assume that there are three
3:10
scheduler and time slides 10
3:11
milliseconds
3:12
in this case
3:14
the job executes first 10 milliseconds
3:17
because the cp uh the process has used
3:20
up all this time slice it
3:23
moves to the next priority level
3:25
and then executes for another 10
3:26
milliseconds once it uses also its time
3:30
slice then it goes down to the lowest
3:32
period level q0
3:34
and there it stays there and then
3:36
it keeps securing the other issue is the
3:39
time content
3:42
to react with the changes in the sea
3:43
visualization
3:45
mlfq may assign different time quantum
3:48
lengths depending upon the priority of
3:50
the queues
3:51
for pi phi high priority queue it uses
3:54
shorter time slice
3:56
and then for lower priority queue then
3:58
may use the
3:59
longer
4:00
time content size
4:02
so
4:03
this example here with the three queues
4:05
the highest priority queue has very
4:08
short time slice length however the
4:11
lower price tq has the longer time slice
4:14
length let's examine the details of
4:16
multi-level feedback implementation in
4:19
solaris
4:20
it uses time sharing
4:22
scheduling class and it has 60 queues
4:27
and uh is slowly increasing the time
4:29
length based on the priority of the
4:31
queue so highest priority queue has 20
4:34
milliseconds time length whereas the
4:36
lowest priority queue has few hundred
4:38
milliseconds time slice length
4:41
and priority of a job is boosted around
4:44
every one seconds or so
4:47
free bst scheduler is another
4:50
way of implementing mlfq
4:52
but this is entrenched implementation
4:54
because freebsd schedule implements mlfq
4:58
without the actual queue instead it uses
5:01
an equation
5:02
so it computes the priority of a process
5:04
based upon how much cpu a process has
5:08
used same as in mlf queue and it boosts
5:11
the priority by dk
5:13
and then also it takes the
5:16
user's intention to
5:18
ill cpu to other processes
5:21
but still for efficiency uses queue
5:24
in our pinterest project we are going to
5:26
implement previous schedule in detail so
5:29
detail will be will be shown later in
5:32
the later part of the class
5:35
okay let's summarize mlfq
5:38
they are a set of rules rule number one
5:42
if
5:43
a process has a higher priority then it
5:46
runs if the two process has the same
5:48
priority and they run in round-robin
5:51
passion
5:52
and then when a job enters the system it
5:54
is placed at the highest priority level
5:57
once the job uses all this time quantum
6:00
at the given level it goes down to the
6:03
next priority level
6:04
after some time or period uh move all
6:07
the jobs in the system to the timeless
6:09
queue
6:10
so finally um what is the beauty of mlfq
6:14
it does not require any prior knowledge
6:16
on the cpu setup a process
