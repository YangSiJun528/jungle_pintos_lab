url: https://youtu.be/sBFJwVeAwEk?si=51sBUCoYdZZs9tQ9

-----------------

Intro
0:00
the next topic is how to implement
0:02
system course incorrect pin test system
0:09
handler is under tables empty after
0:12
modification you have to feel the system
0:14
handler of Pintas and then you have to
0:17
implement the system first so that the
0:19
users can use it the system cores that
0:22
need to be implemented includes helped
0:24
and all these functions and these
0:27
functions you're going to get into
0:30
details with integer functions later
0:33
system call is programming interface for
0:36
services provide the operating system it
0:39
allows the user word programs to use
0:41
kernel features such as creating process
0:43
accessing a file saving a file and stuff
0:46
like that system calls run on the kernel
0:50
mode we have we have dealt with the
0:54
instruction int in detail and what kind
0:57
of parameters are passed from user mode
0:59
kernel mode the key point of system call
1:03
is that the priority of execution mode
1:05
is raised to the special
1:07
[Music]
1:10
by generating hydrating trucks so from
1:14
the user program if you want to use the
1:17
kernel service such as creating a
1:18
process or saving a file then you called
1:22
a system call and ask for operating
1:24
system to do the jobs for you and then
1:27
after system calls complete it returns
1:29
to the user
1:30
the important thing is from here to here
1:32
the privilege level goes up
1:40
this is this call process of system
Call process of System call (Pintos)
1:43
co-inventors operating system if the
1:45
user curls calls a right then there is a
1:49
file access called dot C then it
1:53
contains a body of write and it returns
1:58
via calling since call 3 and in Sisco 3
2:04
it means that it is a system call with
2:07
three parameters so it pushes three
2:11
parameters and then pushes the permit
2:14
numbers and then it generates interrupt
2:18
with there are six three zero in Pintas
2:21
there are six three zero corresponds to
2:23
the intro number that represent system
2:26
core and here if you look at okay then
2:30
interrupt there are six three zero there
2:33
is an interrupt handler for this
2:36
interrupt and then there is a body
2:40
called cisco on the behavior and the
2:45
function is currently empty
2:49
so to implement a system call you have
2:54
to fill this out eventually you have to
2:57
fill the system calendar with this
2:59
function like this then each system call
3:04
has its own number the system call
3:06
number is defined in this call number
3:09
dot H for example in our system for
3:12
handler table sis health the health
3:14
system corresponds to system called mo 0
3:17
exit system car is cars has system Qin
3:21
umber 1 and stuff like that that may
Requirement for System Call handler
3:25
explain the requirement for system call
3:27
Hendler first you have to make the
3:31
system call handler to call system call
3:34
using system car number and the most
3:37
important part of the system call
3:39
handler is check the validation of the
3:42
address the user program has supplied
3:44
let's say this is system core and it has
3:48
it gets a parameter void this is system
3:52
core and in the user program for example
3:55
user program has called a system car
3:59
with zero then in the system core inside
4:10
the kernel the operating system tries to
4:13
access this address then we don't know
4:16
we won't know what is going to happen so
4:20
before calling actual the body of the
4:24
system call the operating system or
4:26
system call handler has to check if the
4:29
other supplied by the user program
4:31
points to the correct address or not so
4:35
the pointers must point to the user area
4:38
and at the kernel area so this is user
4:41
virtual tour space
4:42
this is kernel and this is user and the
4:46
pointer supplied by the user to the
4:49
system core should point somewhere in
4:52
the user interface not in the current or
4:55
space and if these pointers don't point
5:00
to the valid address
5:02
the page for stalker and then once the
5:06
address validation completes then it
5:08
copies the argument in the user stack to
5:10
the kernel and then after that each save
5:14
the return value is 6:10 core at EAX
5:17
register so the important thing again
5:21
the important thing is that user should
5:25
supply the valid user address to the
5:27
kernel and then the kernel should not
5:31
access users address space while it is
5:33
executing instead it has to copy
5:36
whatever the parameters are to its own
5:38
address space from here to here so that
5:45
kernel can execute the program inside
5:48
the kernel without accessing the user
5:50
space
5:52
user can pass in valid pointers through
Address Validation
5:55
the system core it can pass null pointer
5:59
or pointer to the unmapped virtual
6:01
memory or it can pass the pointer that
6:04
points to the corners for Childress pace
6:06
which is about five days in both cases
6:09
the operating system has to kill the
6:11
program so when a user consistent korkin
6:17
only to detect in velocity in validity a
6:19
pointers in terminating process we have
6:22
harm to the kernel or the other running
6:23
processes there are two ways to detect
6:26
the validity of the address first one it
6:30
checks the validity of the user provide
6:33
pointer so you have to check the page
6:35
table and you have to check whether all
6:38
given address is mapped or not so you
6:40
have to use the functions provided in
6:42
page dr to see and we eighty dr dot h
6:45
the other method check if the user
6:48
points to the address below of Ivins in
6:53
that case operating system does not
6:56
check whether it refers to the map the
6:58
dress or not it just checked that the
7:00
given user point is below five days so
7:03
in case user supplied pointer is invalid
7:07
then it causes page fault so you can
7:10
handle by modifying the code page fault
7:13
this is faster because you don't have to
7:16
check anything and it relies on hardware
7:19
a menu to verify and verify the ability
7:24
of the user ders so the second approach
7:28
is widely used in the real operating
7:29
system such as Linux let's look at this
Accessing User Memory (cont.)
7:34
example a process can hold the lock or
7:39
ask for malloc and then the course of
7:44
performing malloc and page fault occurs
7:48
then the process dies but the global
7:53
variable lock is still hold by the
7:55
process or this process got an malloc
7:59
but it did not freed so as wizard page
8:04
fault may may incur the linkage of the
8:08
resources so before terminating process
8:11
the operating system has to unlock or
8:13
release a lock or return the allocated
8:16
memory space to avoid any kind of
8:18
resource leakage handling this kind of
8:25
issues leakage is different in method
8:27
one and Method two in method one it is
8:31
straightforward because the operating
8:33
system take care of everything
8:36
so in system corner it luck or allocate
8:44
the page only after verifying the
8:46
validity of the pointers but second
8:49
approach is more difficult to the second
8:52
method is more difficult because there
8:54
is no way to return an error code from a
8:56
memory access so you can use the
8:59
following functions to handle these
9:01
cases there are two functions first one
9:05
is get under by user and the second one
9:07
is put under a user get under by user
9:10
writes reads a byte from the specified
9:14
address and put and about user writes
9:17
one byte to the user specified address
Add system calls: Process related system calls
9:23
these are the system course you have to
9:26
implement in this page there are process
9:28
related system course the health
9:30
shutdown the Windows operating system
9:33
printers should not shut down and it
9:36
should be shortened only when healthy
9:38
scored the second function is exit it x2
9:41
the process when X is a process it
9:44
should print out the name of the process
9:46
and exit status and then exact is
9:50
command that create a child process and
9:53
execute a program it is important to
9:56
note that this exact is not exact in
10:01
UNIX here exact in Pentos is similar to
10:10
fork and exec
10:13
the combination of fork and exec well
10:21
together and wait system core is wait
10:26
for a termination of child process whose
10:28
process ID is PID and in this project
Process Hierarchy
10:35
you have to introduce a notion of
10:37
process hierarchy you have to specify
10:40
the parent process and you have to
10:43
specify the child process it has created
10:44
and you should have a pointer to points
10:47
to siblings so there are two concepts
10:49
you have to implement the first one is
10:51
parent you have to add this field to the
10:54
thread structure first one it pointer to
10:56
the parent process and second one is
10:59
pointers to the siblings so it goes to
11:02
the doubly linked list and then it
11:05
contains a pointers to the children you
11:09
don't have to there are many ways to
11:11
implement the pointers to the children
11:14
but it is not reasonable to instill all
11:18
the pointers for all child usually you
11:21
can you can maintain pointer to the tail
11:26
of the siblings list and then head of
11:29
the sibling list
11:30
and you can arbitrarily order the
11:34
changes but list of siblings can be
11:37
maintained in the order it was created
11:39
from the oldest to the youngest let me
11:43
explain how we implement a system core
11:45
way system Co way for child process to
11:47
accent and it received the child access
11:50
status if PID our process is live it
11:55
waits till it in terminates and then it
11:57
returns the status that PID passes to
11:59
the exit
12:01
if the PID did not collects it but was
12:03
terminated by the kernel it returns -1
12:07
it is possible that parent process can
12:10
call wait for a ready charlie terminated
12:13
process and then it returns exit status
12:16
of the termination process the important
12:20
thing is that after the child terminates
12:22
the parent should be allocated its
12:24
process descriptor wait fails if the
12:30
process ID does not refer to a direct
12:33
child of the calling process the
12:35
operating system limits the ability for
12:37
process to wait it should wait for all
12:40
its child and the process that calls
12:45
wait has already called wait on PID then
12:47
the process waits fails so this is
12:53
current form or process wait so you have
12:56
to implement it but implementing it is
12:59
not easy so for now youyou put it an
13:03
infinite loop here
13:09
so that is what you are supposed to do
13:11
for this project but this is not the
13:15
correct way of implementing process wait
Correct implementation process_wait()
13:20
let me explain the correct
13:23
implementation of process wait process
13:27
wait it searches the descriptor of the
13:29
child process using the parameter and
13:32
the caller blocks until the trial
13:34
process exits and the ones the child
13:37
exits it D look the allocated descriptor
13:39
which all process and returns access to
13:41
the solution process so how can we
13:44
synchronize the cholera process wait and
13:47
the execution of the child thread it
13:50
waits for determination it has it as
13:54
semaphore weight to the thread structure
13:57
and this semaphore is initialized to
13:59
zero when a thread first created an in
14:02
weight system core the caller cords a
14:06
simmer down for the semaphore third ID
14:08
in the exit process of PID it costs Emma
14:12
up it increases some of a value by one
14:14
and the simmer down decreases the
14:16
semaphore value by 1 because the smurfer
14:19
values initialized zero when the first
14:21
thread is created the call for a system
14:24
car by calling way system core the
14:27
caller is blocked and when the exit on
14:32
the child process calls exit because it
14:34
could increases the value by semaphore
14:35
bank one the waiting process can be can
14:40
be woken up from the slip status and go
14:44
ahead and also in exit the process has
14:49
to return its exit status so we need to
14:52
add a field to the structure structure
14:55
thread to denote the exit status of the
14:58
thread structure so this is the block
Flow of parent calling wait and child
15:02
diagram control flow diagram of how we
15:04
add the semaphore so we in the process
15:07
wait we call semi down and then in exit
15:11
we call semaphore up so by calling
15:15
semaphore down the caller blocks at this
15:18
point waiting for the child process to
15:21
finish when a user
15:22
process calls exit it calls semaphore up
15:25
so that any process that has been
15:28
waiting for the semaphore value to be
15:30
increased is unblocked from the sleep
15:34
status now it's the exact system core in
exec() system call
15:41
Pentos exact system core is the
15:44
combination of fork and exec in unix so
15:51
exec system call in Pentos it creates a
15:55
thread and executes the binary so exec
15:59
system call in Pintas get six
16:01
command-line argument and it runs a
16:04
program which succeeds mail line so it
16:07
created thread and run exact in Pintas
16:10
is equivalent to fork and exec in unix
16:14
it passed arguments the program to be
16:17
executed and it returns process ID of
16:21
the neutron process the parent process
16:25
calling Zac should wait till the partial
16:28
process is created and load the excu
16:31
doubles completely
16:32
that's what exact system core is
16:36
supposed to do of course exec system
16:39
core does not wait for the trial process
16:43
to finish it just creates and lows the
16:46
excu doubles completely and then the
16:49
existing stem call can return let me
16:52
explain how we implement process execute
16:56
process execute is a call function for
16:58
exact so parents should wait till it
17:02
knows the child processor successfully
Kernel function for exec(): process_execute()
17:04
created and the binder file is
17:07
successfully loaded so you're going to
17:10
use semaphore to synchronize the caller
17:12
and the child process that has been
17:13
created as a result of calling process
17:16
on the base cute I'm going to use
17:18
semaphore we add a semaphore for exact
17:22
to the thread structure so now this is a
17:25
second semaphore for the thread thread
17:27
structure this semaphore value is
17:29
initialized to zero when the first
17:31
thread is created
17:33
in exec system call in-process excuse
17:37
system core it calls simmered down to
17:40
wait for that successful load of the
17:42
askew table file of the child process
17:45
when executable file is successfully
17:48
loaded it calls Emma up and we need
17:53
another variable that represent the load
17:55
status in the ER structure we need a new
17:58
field to her present whether the file
18:00
has been law successfully loaded or not
18:02
so as a as a result of this's parent
Current flow of the parent calling exec and the child
18:10
process and it called process execute
18:14
and then in the start process it
18:20
allocated thread structure and it
18:25
creates a new process and then it load
18:27
the binder file for a program to execute
18:30
so when it becomes child process here
18:33
and then parent process has to wait
18:37
until it ensures that the binary file it
18:41
wants to execute is completely and
18:43
successfully loaded to the memory then
18:47
it starts to execute the program so if
18:52
you look at the control flow parent
18:54
process continues executing and then it
18:58
calls process executes and then in the
19:02
part of process executes it called start
19:04
process the sub process at some point
19:07
new thread is created and then new
19:11
threat continues execution and then it
19:14
loads the file and after it finishes
19:18
loading the file it reduces the
19:21
semaphore down then both of these two
19:24
process can continue in parallel
19:29
so this is how you implement correctly
19:33
how we implement exec functions so exact
19:38
after it calls process execute inside
19:43
process executes hence wait for symmetry
19:45
on then in the start process after it
19:49
finishes loading it calls semaphore up
19:54
so there's the important issue here
19:57
where are we going to put this image on
19:59
function is it going to be right next to
20:02
the process execute or are we going to
20:05
put this image down inside process
20:07
execute the next function we're gonna
20:14
cover is exit in exit you have to return
20:18
the excess tightness of a given process
20:21
so if you if a process called terming
20:26
exit the operating system has to
20:29
terminate the chronic user program and
20:31
then returns the Thetas to the criminal
20:34
and if the process parent waits for it
20:38
then it will read the access status of
20:42
the given thread so in the existing exit
20:48
body you have to add the code to save
20:51
the exit status at the process
20:53
descriptor okay this is strong structure
20:57
threat then also you have to print the
21:00
exit status and then name up the threat
21:04
and then you call thread exit so this is
21:08
common function for exact which is
Kernel function for exit():thread exit
21:10
thread exit and here you have to store
21:15
the status to the status of a process
21:17
and then you go sign up so that a
21:20
process that has been waiting for this
21:22
thread to finish can pass it so this is
21:26
body of the exam and then let's let's
21:31
look at the details of the code this is
21:33
thread exit then it deserves interrupt
21:38
you have to disable the interrupt
21:41
whenever you manipulate
21:43
read list so then you list you remove
21:48
the node from the threat list and then
21:52
you change the status of a threat from
21:55
whatever to thread dying and then you
21:58
ill the CPU to order process and then
22:01
this function is not reached
