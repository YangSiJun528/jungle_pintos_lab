url: https://youtu.be/SqMD8rbmEjY?si=accwnC44dvK9bpIs

-----------------

0:00
now we are going to implement a file
0:02
manipulation features in the Pinterest
0:04
operating system there is a process dish
0:09
cook table and this is process
0:11
descriptor
0:12
it's a struct thread and in in a thread
0:16
it has to have a pointer that points to
0:19
the process descriptor table process the
0:22
script Abel is an array of pointers and
0:25
each entry in the array points to the
0:30
file object so first one is standard
0:33
input element 2 0 contains a pointer to
0:36
the send an input file structure and
0:40
element 1 points to this first structure
0:43
for sent an output and the index to the
0:48
second second element the process file
0:50
disco table points to the standard error
0:53
and then there is file descriptor
0:57
entries here so current Pintas does not
1:03
have file descriptor table so we have to
1:06
implement it this is the details of the
1:09
implementing from this view table its
1:11
processes has its own file to store
1:13
table now we can redefine maximum size
1:16
of 64 entry for this to table is an
1:19
array of pointer to the struct file
1:22
pointer to the struct file so there is
1:24
going to be 64 entries and each entry
1:26
points to a object called struct file
1:32
this is 0 and this is 63 and 50 in this
1:38
file descriptor is an index of the for
1:41
dessert table and it is allocated
1:43
sequentially from 0 to and and fire
1:47
distress 0 and 1 allocated for standard
1:50
input and standard opera spectively open
1:54
system core returns or far descriptor so
1:58
open system call allocates a struct file
2:00
and it scans whatever empty entries on
2:04
the fire disco table and then sets the
2:07
Associated entry points to the struct
2:09
file that is open closed system core
2:13
resets the value of our entry with 0 so
2:16
it is as a pointer this is the sample
2:21
layout of the file disputable we are
2:24
going to define the far discreet table
2:27
of 64 entries the first approach is
2:31
directly embedded of our disco table at
2:33
the thread structure 64 oh we can add a
2:39
pointer to the fire descriptor table and
2:42
then we can allocate separate file this
2:44
your table at the kernel memory area
2:46
like this then in this case the two
2:51
threads can share the same file as your
2:52
table
2:58
or here in this example thread a has a
3:02
Fire District Point Fire District table
3:05
here and anything in tain is a pointer
3:08
that points to its own party's table and
3:10
thread B has its own fire dessert table
3:12
defined outside of thread structure and
3:14
point to a fired extra table pointer of
3:17
thread B structure points to its own
3:19
Fire District table when the thread is
3:23
created the operating system I'll locate
3:25
the file descriptor table for a thread
3:27
and then it initialize a pointer to fire
3:30
dessert table one thing we should not
3:33
forget is when initializing fartistry
3:35
table we have to reserve our discrete
3:38
zero and for our district one for
3:40
sandwich input and standard output when
3:42
a process is terminated it has to close
3:45
all files and then the operating system
3:47
deallocate the fire district table one
3:50
thing we have to emphasize is the race
3:54
condition in Pintas we are going to use
3:57
the global lock to avoid race condition
4:00
in a file so we are going to define a
4:03
global lock and then whenever fire
4:06
system core fire system related with the
4:08
system query is executed it is protected
4:11
by the lock this is to avoid the race
4:16
condition of the fire system operation
4:20
we have to modify the page fault for the
4:23
test in Pintas some test checks whether
4:28
you Crowell handles the bad process
4:30
properly in Pentos it needs to kill the
4:34
process and print the thread name and
4:38
exit status - run and page fault occurs
4:42
for this purpose we have to modify the
4:44
page fault to satisfy test requirement
4:48
and simply you can call exit -1 inside
4:52
page fault these are the system course
4:55
relate to file manipulation first one is
4:58
create it creates a file of the sizes
5:04
the initial size and there is system
5:08
where we move we move the file whose
5:10
name is file and open and it calls far
5:17
sister open and remove it calls fastest
5:21
remove and create it calls parses create
5:25
so all these functions are already
5:28
defined in the Pintas so what you have
5:31
to do is just provide a certain
5:33
appropriate system call that calls this
5:37
function there is a file size function
5:42
also and it calls a function file length
5:46
in rich system core in rich system core
5:50
you have to distinguish the read from
5:54
the farthest as a row and read from
5:57
other file descriptors in case the file
6:00
descriptor is zero then you have to call
6:03
the input get see in that raiser
6:06
characters from the keyboard and for
6:08
other file descriptors other than 0
6:10
you call file read same things applies
6:15
for right system core when the far
6:18
descriptor is 1 then you have to write
6:20
the output to the console otherwise I
6:23
have to call file right to the right and
6:29
there is a function called tell and it
6:32
calls file on the bottom and they're
6:34
sitting close which close the fire and
6:37
the backgrounds the last topic you have
6:42
to take care of is that denies the rise
6:46
to the executables what if the operating
6:49
system tries to execute a file that is
6:51
being modified then the research cannot
6:54
be a unpredicted can be unpredictable so
6:57
the objective of this topic is do not
7:02
allow the file to be modified when it is
7:04
open for execution there are two
7:06
approaches now the approach is that when
7:11
the file is loaded for execution call
7:13
filed in I write when a file finishes
7:16
crucian file called file alright so at
7:20
the beginning and end of load executable
7:23
you call fired and I write when process
7:26
exits you add file alright using this
7:30
approach you can simply enable the
7:33
Pinter's to deny rise to the excludable
7:35
file once you completely implement all
7:41
these features then you to be able to
7:43
pass the test
7:45
I'd like to finish this presentation by
7:49
summarizing what functions you have to
7:51
add and what functions you have to
7:53
modify these are the call flows of the
7:56
Pinterest operating system in creating a
7:58
process first it calls process execute
8:01
and then process execute cards thread
8:04
create and then it course sub process
8:07
and it calls load so at each process you
8:10
have to provide feature to parse the
8:13
name of the program to run and then here
8:15
you have to create a thread and add it
8:17
to the red list so new thread is created
8:19
here and it's pretty clear already list
8:21
but interesting thing is until this
8:23
point you don't know which function or
8:25
which program you need to execute then
8:28
at the start process it presents
8:31
prepares an interrupt frame and it
8:34
prepares interrupts frame so that it can
8:36
get into the user mode from the kernel
8:39
mode and start executing and then and
8:41
then it loads executable and then it's
8:44
if we all this succeeds then you get a
8:48
user program
8:53
good luck
