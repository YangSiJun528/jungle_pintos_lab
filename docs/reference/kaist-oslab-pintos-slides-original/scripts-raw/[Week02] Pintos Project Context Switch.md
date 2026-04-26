url: https://youtu.be/mtX-bj1Fu6M?si=vCRv4Xu8zDLXLfbX

-----------------

Intro
0:00
this class i'm going to explain the
0:01
details of context switch in pentas
0:03
operating system this is an overview of
0:06
the talk
0:07
we'll apply top process structure
0:09
process date
0:11
process context and schedule and
0:13
switching the two thread
0:15
the first topic we are going to deal
Process structure: struct thread
0:16
with is the structure thread or
0:19
structure that represent a process
0:22
this is one of the most important data
0:25
structure in operating system corner
0:28
this is struct thread
0:31
it contains a various attribute that is
0:34
required to represent a thread the first
0:37
one is thread id the status of the
0:39
thread
0:40
thread priority
0:42
and then it contains a pointer to the
0:44
kernel stack and i will explain it later
0:48
then it contains the pointer that points
0:51
to the page table it uses
Process state
0:56
there are four process state in in
0:58
printer's operating system that is
1:00
running
1:01
ready
1:03
blocked and that's our three basic
1:05
process tape
1:07
and then there is another state called
1:09
dime
1:10
this is the state where uh process stays
1:13
after it calls exit and before the
1:16
operating system completely
1:18
removes all the data structure that has
1:20
been allocated for the process
1:23
most important function for
1:25
a thread is
1:27
creating a thread this is the per type
1:29
of the thread on the back create it has
1:32
four attributes name
1:35
and thread priority
1:37
and the function it wants to execute
1:39
when the thread is called and the
1:42
parameter list then needs to be supplied
1:44
to the function you want to execute this
1:47
is a detailed creating a thread inside
1:49
thread.create it calls a function called
1:54
init thread
1:55
in memset it allocates
1:58
memory for the stress structure and set
2:01
it to zero and after allocating the
2:04
structure it says the operating system
2:06
sets the state initial state of the
2:08
thread to blocked it puts just create a
2:11
thread structure to some list called all
2:14
list that i am going to explain later
2:17
in pintas there are two
Process list
2:19
list of processes or lists of threads
2:22
first one is ready list
2:24
and the second one is all list
2:26
ready list is a set of threads that are
2:29
ready
2:30
for execution
2:32
and all list is a set of all threads in
2:34
the system
2:35
so in the thread created it is first
2:38
inserted into the list called all list
2:41
and then all the threads that are ready
2:42
for cpu execution are inserted into the
2:46
ready list
Creating a thread
2:50
creating a thread it first inserts the
2:53
created thread structure to a list
2:58
and after putting it into our list
3:02
the operating system initializes the
3:04
various fields in the thread structure
3:06
and setting the state of the thread to
3:08
ready list and then
3:10
put it in the red list
3:13
in
3:14
in in current pinterest operating system
3:16
whenever the operating system inserts
3:19
the thresh structure to the right list
3:21
or o list it calls list and the push
3:24
back on the right back it means that
3:27
operating system always places to just
3:30
create a thread at the very end of the
3:32
list let me explain the function
3:34
schedule it is very important function
3:37
let me put three
3:39
exclamation mark
3:42
it is a function which the operating
3:44
system cores when it needs to schedule a
3:47
new thread
3:48
new thread means that scheduling a new
3:50
thread means the operating system
3:54
kicks out the currently learning running
3:56
thread and then find the next thresh to
3:58
run and put it in the cpu then who
4:02
called schedule
4:04
the question is equivalent to saying
4:06
that
4:08
how
4:08
the process releases cpu
4:11
there are two reasons first one is
4:14
voluntary loop
4:17
voluntary release and the other one is
4:20
involuntary release
4:23
involuntary release
4:25
the existing process cards exit or it
4:28
blocks or it calls yield or
4:32
in involuntary switch
4:35
the currently running process is
4:37
invulnerably
4:38
replaced with the other process
4:41
that happens when the higher priority
4:43
process has arrived or when the time
4:46
quantum it has been using has expired
4:49
before calling schedule
4:50
we have to disable interrupt
4:53
and then we have to change the states of
4:55
running thread from running to something
4:57
else
4:58
something else means that when
5:00
a schedule is called in the block and
5:04
then the status goes to blocked there
5:06
are three reasons
5:08
where the schedule is called when the
5:10
process exit and in the process with an
5:13
event or blocked or when the process
5:16
calls yield
5:17
so
5:18
in current pinterest operating system
5:22
there is no way for an operating system
5:25
to
5:26
switch out the existing process unless
5:30
the process wants to
5:32
so in current pinterest operating system
5:33
there is no preemption this is done
5:36
preemption
5:38
so in the course of doing the first
5:40
project we are going to turn this
5:42
operating system into preemptive
5:44
operating system
5:45
in preemptible printing system pintos
5:48
you are going to implement a scheduling
5:51
module that kicks out the existing
5:53
process if the newly incoming process
5:56
has a higher priority
6:00
so this is actual code of pintus so this
6:03
is a thread block this thread love
6:05
function is called when a thread asking
6:08
for a ayah completion and then it calls
6:11
a thread block and inside third block
6:13
it puts the state of the process in the
6:16
thread blocked
6:17
and then it calls schedule
6:19
we are going to deal with the scheduling
6:21
detail so please bear with me for now
6:24
and then second topic is thread yield
6:27
when the
6:29
the running process wants to
6:32
hands over the cpu to other process then
6:35
it cause yield
6:37
in yield the currently running process
6:40
puts itself to the end of the ready
6:42
queue and
6:43
hands over the existing cpu the other
6:45
process
6:47
so
6:48
in that case the existing process state
6:50
becomes thread ready
6:53
and
6:54
in third case when the process exits
6:57
after process exits is um it changes
7:01
states to thread dying
7:03
and then he called schedule
7:06
this is the three situations where the
7:09
schedule is called remember all in all
7:12
this state uh process
7:14
voluntarily
7:17
relinquish
7:18
cpu
schedule (void)
7:22
okay so now let's look at the details of
7:26
the schedule function
7:30
it gets the pointer to the current
7:31
thread
7:33
and then it gets the pointer to the next
7:37
thread to run so this is where the
7:40
scheduling discipline
7:42
comes into play so for example um you
7:46
may want to use first come first serve
7:50
or you want to use shortage of first or
7:53
you want you may want to use shortest
7:56
time
7:57
to completion algorithm or you may want
8:00
to use round robin
8:03
in all this scheduling algorithm
8:06
operating system selects what is the
8:09
next thresh to run
8:11
so in this next thread to run operating
8:13
system selects the next thread to run
8:16
and returns the pointer to the
8:19
just selected thread structure
8:22
after getting the pointers to the
8:24
current running thread and the next
8:26
thread to run it calls a function called
8:28
switch thread so the function switch
8:31
thread is responsible for saving the
8:33
current context and restoring the next
8:36
context of the next thread to run into
8:38
the cpu registers and
8:41
it returns a pointer to the thresh
8:42
structure
8:44
into the variable called prv
8:47
after it is returned preview the
8:49
operating system calls thread schedule
8:52
tail which puts the
8:55
peer the thread structure pointed by
8:57
prev at the end of the ready list
9:01
now
9:02
i am going to explain the details of
9:05
thread switch
9:07
as the first step
9:09
i'm going to explain what the stack is
9:11
stack is a very primitive data structure
9:14
it contains an operation push and pop
9:17
and as you push the items to the stack
9:20
the stack top pointer grows
9:23
as you pop the data from the stack the
9:26
pointer grows the other end
9:29
so there's important register
9:32
we call it esp as tag pointer and it
9:35
points to the top of the stack
9:39
and usually stack grows to the lower
9:42
lower address space push abc
9:46
stores the value at the address pointed
9:48
by abc into the stack top and increases
9:51
the stack pointer by four
9:53
but increases the stack pointer by four
9:56
increasing the stack pointer by four
9:57
corresponds to decreasing the esp value
10:00
by four
10:02
byte
10:03
the next operation is pop abc
10:08
it retrieves value
10:11
which is
10:12
pointed by the esp pointer and saves it
10:16
to the location pointed by abc
10:19
and then it decreases the stack pointer
10:21
by 4 byte
10:23
that is pop
10:24
so push and pop is the two basic
10:28
operation in stack
10:30
virtual address space of a process
10:32
partitioned into two configured regions
10:34
user space and kernel space in pintus
10:37
virtual displays from zero to three
10:39
gigabytes forms a user space and virtual
10:42
space beyond three gigabytes range forms
10:45
a kernel address space
10:46
to access the chrome over space the
10:48
process needs to change its execution
10:50
mode to the kernel mode
10:52
it is achieved by increasing its private
10:54
level
10:55
when the process runs in the user mode
10:58
it uses user stack to cause a function
11:01
and to define the local variables when
11:04
it switches the kernel and it executes
11:06
in the kernel it uses the kernel stack
11:08
to access and
11:11
use the function call there's a conor
11:13
stack and there's the user stack
11:16
i'm going to explain the detailed steps
switch_threads (struct thread cur, struct thread *next)
11:18
of switch thread
11:19
switch thread function switches the two
11:21
threads pointed by cur and next curl
11:24
represents the currently running thread
11:26
and next we present the
11:28
uh
11:29
that threads to run
11:32
it basically consists of four steps
11:34
first it saves the registers
11:37
to the kernel stack
11:38
and then it says the location of the
11:41
current stack top at the current threads
11:44
stack attribute
11:47
this one and this one
11:49
and then it restores the new threads
11:51
tech top into cpu stack pointer
11:54
and then using the stack pointer just
11:56
newly established it restored the
11:58
registers from the stack to the cpu
Cal switch_threads
12:02
this represents the stack
12:04
current stack and current stack type
12:07
register value
12:09
right after it calls switch thread
12:13
and there are two pointers
12:15
see you are core and next
12:18
it points to the thread structure of the
12:20
currently running thread and the next
12:22
thread to be run
12:24
and then there are two stacks this is
12:26
the corner stack of current thread and
12:28
then it's a kernel stack of the next
12:30
thread
12:31
and both of them are in the kernel stack
12:34
connectors region
12:36
and
12:37
the stack pointer esp points to the
12:40
stack top of the current stack
12:43
this is the state of the memory layout
12:46
right at the switch that is
12:50
called so it
12:53
it just jumps
12:55
jump to the switch thread function
12:59
and then
13:00
after jumping into the switch thread it
13:03
stores the four registered four cpu
13:06
registers to the corner stack of current
13:09
threads
13:10
so after pushing ebx ebp esi and edm
13:15
registers the stack pointer increases to
13:18
the new stack tab
13:20
to push for registers the switch thread
13:23
function executes four instructions ebx
13:26
ebp
13:27
esi and edi
13:29
so after executing these four
13:32
instructions it pushes the full
13:34
registers of the currently running
13:36
thread into the stack top
13:39
into the current kernel stack and kernel
13:41
stack is changes to point to the newly
13:43
stabilized stack time
13:46
after
13:47
storing the full registers to the kernel
13:49
stack it's time to save the corner
13:52
stacktop address to the thread structure
13:56
of currently running thread
13:58
so in this figure this stack attribute
14:01
set to point to the stack type of corner
14:04
stack
14:06
so it contains this tech type address of
14:09
the
14:10
esp registers
14:13
the first thing it needs to do is to
14:15
load the offset of
14:17
stack attributes to the register edx
14:20
stack attributes is 24 byte apart from
14:24
the beginning of thread structure
14:26
so we first have to identify the amount
14:29
of offset it has to jump from the
14:32
beginning of the stress structure and it
14:34
is specified as this statement
14:37
and its macro thread on the bar stack
14:39
under offset represents the offset of
14:42
stack attributes from the beginning of
14:44
this
14:45
thresh structure this is 24.
14:50
so after executing this statement edx
14:53
becomes to contain the
14:55
value 24.
14:58
next step is to load the beginning
switch_threads (struct thread cur, struct theead *next)
15:01
address of current struct thread to the
15:04
ex register
15:05
the next thing the operating system
15:08
does is to save the location of current
15:11
struct thread to eax register but how do
15:14
we know the current location of current
15:17
struct thread
15:18
if you look at the stack of
15:21
the cur and then there is a field cur
15:26
that
15:26
saves the location of current thresh
15:29
structure
15:30
and it is from 20 byte
15:33
apart from
15:35
the esp structure
15:37
so by a switch car this represent
15:41
20
15:42
and then from
15:44
step 0.0 if you go up for 20 byte then
15:48
that location contains the address of
15:51
current
15:53
struck through a data structure
15:55
so by executing this statement the eax
15:58
register becomes contained the address
16:01
of current thresh structure
16:05
and then as a next step we save the
16:09
current threads tech type address to
16:11
thus track threads text field
16:13
so uh we move the value of esp the stack
16:18
pointer to
16:19
this address which is the eax points to
16:23
the beginning of this tracked thread
16:25
data structure and edx represent offset
16:28
between
16:30
uh
16:31
just between the beginning of the thresh
16:33
structure until the location of the
16:35
stack
16:36
and then there
16:38
we save the value of esp
16:41
and then as a result
16:43
let's go back to the few slides back
16:45
as a result the pointer the stack field
16:49
contains the address of the current
16:51
stack tab
16:52
and
16:53
it looks like this
16:55
the next step is to switch the stack
Switch kernel stack
16:58
pointer point to the stack type of the
17:00
next thread structure
17:02
switching the step dot pointer from the
17:05
set top of the current thread to the
17:07
stack type of next thread consists of
17:09
two steps basically first one it has to
17:12
identify the location of the thread
17:14
structure of the next thread first to do
17:17
that from this point
17:19
goes up for 24 bytes and then identify
17:22
the location of the
17:24
next thresh structure
17:26
and then after identifying the location
17:28
of next thread structure it identified
17:31
offsets between the stack
17:33
between the beginning address of the
17:35
thread structure till the stack resides
17:37
this is 24 byte so after that you can
17:41
retrieve the location of the selector
17:43
pointer from here to the thresh
17:45
structure then the stacked up pointer
17:48
has successfully changed from the
17:50
current thread to the new thread
17:53
if you look at the data structure the
17:55
first one to identify the location of
17:58
the next
17:59
thread structure is this instruction so
18:02
from sector pointer
18:05
it goes up by 24 byte
18:08
this is the location of next thread
18:11
structure and then it contains the
18:13
address of the thresh structure of the
18:15
next restaurant that has been supplied
18:17
by the caller to the switched thread
18:20
and then after loading the location of
18:24
next thread structure to ecx and then it
18:28
adds the offset of
18:30
stack field to this base location and
18:33
then it saves the address value located
18:37
at that
18:38
address to esp
18:40
then the esp points to the news tech tab
18:43
which is the stack type of the next
18:46
thread
18:47
after
Restore the new context
18:48
switching
18:49
the stack pointer to the new stack top
18:52
it pops four register values and then
18:56
restore it to the four registers as edi
19:00
esi ebp and ebx and then as a result of
19:03
popping four register values the setup
19:06
pointer is points think uh has updated
19:09
to this location
19:13
so it performs
19:14
four pop instruction
19:18
and then kernel stack of the next thread
19:21
is now updated as
19:23
this this is how we switch the two
19:25
threads
19:27
after switching the current thread and
19:29
the next thread
19:31
the next operating system updates the
19:33
state of the newly selected thread as
19:35
running
19:37
and if the previous thread was in the
19:40
dying state the operating system has to
19:42
clear up all the pages are located to
19:44
that thread so this is the
Change the state of new current
19:47
details of the code of thread schedule
19:51
tail so
19:53
it updates the new leak running thread
19:57
as the thread running
19:59
and then if the collar of the switch
20:02
thread
20:03
was
20:04
in dying state then it has to deal
20:06
locate all the pages allocate to the
20:08
dying state process and then freeze it
Summary
20:12
in this video we have
20:14
we have explained the details of
20:16
context switch in pinterest operating
20:18
system
20:19
schedule function is called in exit
20:21
yield and block and it gets a new
20:24
process to the cpu
20:26
there are four important steps in
20:28
context switch it saves the context of
20:30
the currently running thread to the
20:31
stack and then save the current stack
20:34
top at the currently running struct
20:37
thread
20:38
and then
20:39
it switches just ticked up register
20:42
pointing to the stack type of the next
20:44
thread and then it restores the context
20:47
from the stack
20:48
to the cpu
20:51
and after switching the two thread it
20:53
updates the state of the next running
20:55
process and then frees the memory from
20:57
the dying process

