url: https://youtu.be/4-OjMqyygss?si=uy9Q-H-kDi0U7rD1

-----------------
0:00
in this video i'm going to explain
0:03
uh another
0:04
topics on implementing speed scheduler
0:07
the title of this talk is about 4.4 bh
0:10
like scheduler
0:12
our main goal is implement
0:14
4.4 bs scheduler which is mlfq
0:17
multi-level feed crew light scheduler
0:20
without the qs um
0:22
bsd 4.4 scheduler gives priority to the
0:26
processes with interactive nature and it
0:29
is basically priority based scheduler it
0:32
uses equation to compute the priority
0:38
the most important concept in bhd
0:40
scheduler is the concept of nice it's
0:43
integer value and it represents the
0:45
niceness of a thread
0:47
if a thread is nicer
0:49
it means that the thread is willing to
0:52
give up some of its cpu time
0:54
so in pentas nice value ranges from
0:57
negative 20 to 20.
0:59
a nice value of 0 is default and it does
1:03
not influence
1:05
and priority
1:07
if nice is positive then
1:11
it decreases the priority if nice is
1:13
negative then it increases the priority
1:15
of a process there are two functions for
1:18
adjusting and getting nice value first
1:20
only thread get nice and second one is
1:23
thread set nice
1:27
in pentos the priority of process ranges
1:30
from 0 to 63 it is
1:33
unsigned integer
1:35
minimum being 0 with maximum being 63.
1:40
the larger the number the higher the
1:42
priority when the thread is initialized
1:44
the thread is
1:46
the priority of the thread is set to 31.
1:51
this is all
1:52
so um this is a priority of a process
1:57
and it is computed as a priority
2:02
minus
2:03
recent cpu divided by four
2:06
minus nice times 2. so i'm going to
2:09
explain the details of this equation
2:11
this equation is very simple but it is
2:14
profound
2:18
there are four principles in behind this
2:21
equation first if the thread is nicer it
2:25
lowers its priority
2:30
if the thread has been using
2:33
lots of cpu recently
2:35
lowered priority
2:37
so there is an important
2:40
word here recently and how
2:43
cpu scheduler take into account how the
2:45
cpu has been using the process has been
2:47
used recently
2:49
later for all thread priority is
2:52
recalculated
2:55
once in every fourth clock tick
2:59
and the result is truncated to nearest
3:02
integer um this statement
3:05
sounds pretty obvious but it is not the
3:08
reason is that um
3:10
foreign and priority max this is initial
3:14
and then recent cpu
3:16
divided by four and nice is this is not
3:20
an integer number it's floating one
3:22
number so we have to have some some
3:25
rules to map the floating floating point
3:28
number to the integer value and the
3:30
result is truncated to its nearest
3:32
integer
3:36
first
3:37
we're going to talk about the concept of
3:39
recent cpu
3:42
it represents how much of the cpu cycles
3:45
the process has been using
3:48
cpu scheduler
3:50
sorry um timer interrupt increases
3:52
recent cpu
3:54
of the currently running process by one
3:57
in every time i interrupt
4:01
and
4:02
in the previous slide
4:06
lower the priority
4:07
if the process has been using cpu for
4:12
uh for recently
4:15
so
4:16
even though the process has been using
4:18
cp a lot of times long before then we
4:21
have to have a mechanism to
4:24
discourage
4:25
that fact comes into play so we
4:29
bring the concept of dk
4:32
um
4:33
dk decreases the amount of recent cpu
4:36
value by a certain amount of time dk
4:38
factor in every second
4:40
so of course the dk is less than one
4:43
we are just a recent cpu by nice in
4:45
every seconds so in every seconds we
4:49
add nice value to recent cpu and set it
4:52
to the new value of recent cpu so
4:54
putting it all together
4:56
uh recent cpu is computed by dk times
5:00
repeats recent cpu plus nice value in
5:04
every second
5:06
every second
5:11
in system 5 release 3 the dk factor was
5:16
uh 0.5
5:18
in bst point point 4.4 it incorporated
5:22
more sophisticated
5:24
mechanism um in heavy load
5:27
the cpu schedulers to make the dk factor
5:30
nearly one
5:32
in light load dk factor
5:34
converges to zero
5:36
so to achieve this objective um
5:39
it comes with this very interesting
5:42
uh formula
5:44
which means that um the decay factor is
5:47
2 times load average divided by two
5:51
times load average plus one
5:54
if load average
5:57
is large then this value converges to z
6:00
one
6:01
and if load average is
6:04
um nearly zero
6:06
then this value goes to zero converges
6:09
to zero
6:12
and uh there's another way uh uh what
6:15
then what is load average
6:17
um load average represent how busy the
6:21
system is
6:22
so uh at the booting time load average
6:24
is initially set to zero
6:26
and load average is a
6:30
weighted average of load average
6:34
and ready threads and ready thread is a
6:37
number of threads in the ready list
6:40
and plus threads in the executing at the
6:41
time of an update
6:43
so that's
6:45
that represents a number of registers in
6:48
the system so load average again
6:50
computed by um this value of 59 divided
6:54
by 60 times load average
6:57
plus 1 over 60 times radius threads
7:00
all of these values
7:02
plays a very critical role in
7:05
determining the fairness efficiency
7:07
performance through the space cpu
7:09
schedule algorithm
7:12
but we're not going to get into details
7:14
about how to set this value
7:18
so in summary
7:20
we can
7:21
obtain the following rules first
7:24
in every fourth tick
7:27
we need to recompute the priority of all
7:29
threads
7:31
as follows
7:33
in every fourth clocktick
7:36
and in every clock tick we increase the
7:38
running threads a recent cpu by one
7:42
in every second update to every threat
7:45
recent cpus follows
7:48
recent cpu
7:50
becomes
7:51
dk factor times recent cpu plus nice
7:55
and decay factor and load average is
7:59
computed as follows
8:06
let us provide an example
8:08
there are three processes p1 p2 and p3
8:12
and initial value of nice is 0
8:15
an initial value of load average is
8:18
also 0.
8:20
let's start
8:22
at the first clock tick the priority of
8:25
four threads are all 63 so process one
8:29
is picked up as a sleeper scheduler
8:33
and um
8:35
it as
8:36
ad clock ticks one zero one 2 3 the
8:40
recent cpu value increases by 1
8:44
2
8:46
3.
8:47
and then at fourth clock tick it
8:50
the priority of p1 p2 p3 is recalculated
8:55
so
8:57
because the recent cpu becomes 4
9:00
so priority of process
9:03
1 becomes 62 from 63.
9:07
then
9:08
it compares the priority of the two
9:10
other processes p2 and p3 the rest of
9:12
the two processes gets priority 63 and
9:16
it has the higher priority so due to
9:19
that reason priority process 2 is picked
9:21
up and then it gets executed from also
9:24
the recent cpu value increases four
9:27
times from zero to one one two two three
9:29
to four
9:31
through four and at this point um the
9:34
priority of p2 is update is 62.
9:37
therefore the priority of p3 is 63 and
9:40
it gets the highest priority so it gets
9:43
the cpu
9:45
at the same time let's look at how
9:47
recent cpu gets reset
9:50
here
9:54
regen's recent cpu
9:58
is
10:00
computed like this two times load
10:02
average divided by two times load
10:05
average plus one
10:06
times resistance u plus nice basically
10:09
if you consider all these then recent
10:11
cpu values the process will become zero
10:14
to four at this time period
10:18
so as a result
10:20
according to
10:21
this
10:22
priority mechanism p1 and p2 and p3 gets
10:26
executed
10:28
in roundup manner if they all require
10:30
cpu
10:33
there is one important things to do
10:36
you need to implement fixed point
10:38
arithmetic
10:39
the reason is inside the kernel
10:42
you can do only integer arithmetics
10:46
kernel doesn't have shaped floating
10:48
point
10:49
register when switching the thread
10:51
treating the context
10:53
so
10:54
you need to implement fixed point
10:55
arithmetic using integer
10:58
arithmetic priority
11:01
nice and ready thread value
11:04
are integers
11:06
however the recent cpu load average
11:09
value is real
11:11
so
11:12
we're going to use the 17.14 50 point
11:15
number representation
11:18
using in this representation
11:21
decimal point says 14 right most bits
11:25
and integer is 17 next bits to the left
11:29
and the last of the left bit one bit is
11:32
sign bit
11:36
so this is how it looked like so this is
11:38
total 32 byte sorry 32 bit
11:43
and
11:44
from
11:45
0 to 13 this is decimal point
11:51
and
11:53
left leftmost speed represents sign
11:56
and then this one represents
11:59
the fixed numbers
12:06
so this is integer part
12:09
this is the small and this assignment
12:12
so this is a rule
12:14
we need these functions convert n to
12:17
fixed point
12:19
and convert fixed point to integer
12:22
and then convert x to integer and add
12:26
two values
12:27
where x and y is fixed point numbers and
12:30
n is integer
12:31
and then subtraction
12:33
addition
12:35
and
12:36
the addition between the fixed point
12:38
numbers and integer numbers and then
12:41
subtraction multiplication
12:43
and division
12:45
so uh you need to implement all these
12:48
functions by yourself and then
12:51
use proper functions
12:53
to do the arithmetics
12:57
so this is basic
13:00
of
13:01
implementation first thing you have to
13:03
do is
13:05
you have to
13:06
add
13:07
nice
13:08
and recent cpu fields to the structure
13:12
thread
13:14
and then you need all these functions
13:16
first
13:17
you need a function that calculates
13:20
priority using recent cpu and nice
13:24
and then you you also need a function to
13:26
calculate recent cpu load average
13:29
and you need a function to increase in
13:31
cp by one
13:32
and then also you have to
13:35
recalculate the priority and recent cpu
13:37
of all threads
13:40
so
13:41
if you use this simple equation
13:44
then you may not
13:47
need multiple cues to implement
13:50
multi-level feedback
13:51
so
13:52
this simple equation
13:54
based cpu scheduler achieves the same
13:58
objective as multi-level feedback you
14:01
does
14:03
it it gives priority to the interactive
14:06
jobs and it gives priority to the i o
14:09
intensive jobs
14:14
so these are functions to modify
14:16
um
14:18
in any thread
14:21
you have to initialize your nice value
14:23
and recent cpu
14:25
and also um
14:27
in
14:27
in set priority
14:31
you disable the friday setting when
14:33
using advanced scheduler
14:35
and
14:37
you have to adjust
14:39
time inter function
14:42
in time enter function you recalculate
14:45
load leverage recent cpu of all threads
14:49
and also priority in every one second
14:53
and
14:55
you have to recalculate a priority of
14:57
all threads in every fourth tick
15:05
and
15:06
please disable priority donation
15:10
when using advanced scheduler
15:13
also both in luck acquire and lock
15:16
release
15:19
these are the functions you need to
15:20
modify to implement
15:22
base like scheduler
15:25
there is a function called thread set
15:28
nice
15:29
it sets nice value of the current thread
15:32
and it also has a function thread get
15:35
nice
15:37
it returns nice value of the current
15:38
thread
15:40
and
15:42
you implement thread get load average
15:46
it returns load average multiplied by
15:47
100
15:49
and then also you write thread get
15:52
recent cpu it returned a recent cpu
15:54
multiplied by 100.
15:57
so once you completely have implemented
16:01
these features then you should be able
16:03
to pass all 30 to 27 tests