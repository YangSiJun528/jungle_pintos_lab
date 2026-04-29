url: https://youtu.be/RbsE0EQ9_dY?si=JcKIZByXsQfrbEmk

-----------------

Intro
0:00
in second part of second project we are going to let the pins to run a
0:09
program the objective of project number two is execute use programing te pintos pintos cannot run the program currently
0:16
so basically it's not an operating system so far this is background and in pentos um
Running a program in pintos
0:25
or in any operating system uh it gets an argument uh to ex process then it it
0:32
calls a function called process execute and in process execute it creates a
0:37
thread with a certain function the problem is the body that calls process
0:44
weight is simply returns so basically the process
0:49
weight function uh this function has to wait for the child process to be created
0:55
and has to wait until it finishes however it doesn't do anything but it simply
1:01
returns so as a result the pintas works like this so in pintas uh init is a
Executing a program
1:09
process zero it's a mother of all processes in the world uh the process ID of init process is zero uh it creates a
1:17
new process and then it schedules it however
1:22
right after it schedules it it just exits so if the operating system exits
1:28
how can the other program run so in current pintas there is no way for the
1:34
process to get executed so final goal is on the right hand side of the slide uh we let the
1:41
pins to wait for the completion of the CH process and
1:47
continue it is quite uh amount of work and I'm going to explain how and what we
1:54
have to do step by step before move on let me explain the
2:00
overall structure of the pro program execution in pintas first function is called process
2:08
execute it gets a file name basically it is the name of the executable files for
2:13
example a. out or if you want to perform some kind of Ls command then LS is going
2:21
to be supplied as a fil L parameter in executing process and then a process execute calls
2:29
a function called thread create so via calling thread create uh this function
2:36
creates a new thread so there is a thread here and then it calls process
2:43
execute and then it calls process execute called a function thread create
2:48
and then at that time at that very moment new thread it created and then it
2:53
continues executing let's look at the details of
Creating a thread
2:59
thread create this is a function that creates a new thread structure so um it
3:06
creates a new thread structure and initialize it and it allocate the kernel
3:11
stack and it register the function to run register means put the value of
3:17
instruction pointer or instruction counter to the entry point of the given process and then add it to the r list
3:25
well it's it sounds conceptual so we have to look at the code
3:31
looks uh it's a simple function and I just exerpt main line of
3:38
the code uh it is function called thread create and it needs the name of the
3:44
function you need to execute and it is supplied a default priority it wants to
3:51
set and then function name and auxilary
3:57
parameters let's look at the code the first one um it allocated a single page
4:04
4 kilobyte page in the kernel space and then uh single Space 4 kiloby
4:11
single space uh other space um it initialize the thread
4:18
structure this is going to be 64 by or 128 BYT or whatever the the thread
4:24
structure size is going to be and then it is a located a thread
4:31
ID every thread has unique ID then um it also allocates a kernel
4:38
stack here is all located means ker stack and then in the kernel stack it
4:45
initializes a various field including the function it wants to execute so kernel St contains a uh address of the
4:53
function that needs to be executed and then the kernel puts the threat to to
4:59
the ridd list by unblocking it this is how a thread is
5:06
created all right then the next step is how to start a process so far thread is
Starting a process
5:13
created and then it has got put into the red list then there is another important
5:20
function called start process and in start process this is the
5:28
start process it gets a file name so it's a name of the binary file
5:35
it wants to execute the first thing it needs to do is it loads that binary file from the
5:42
disk to the memory and then um out of that binary file it obtains the instru
5:50
location of the instruction it wants to execute then also it acquires uh stack
5:59
top pointer of the user stack that's a role of load if it
6:06
succeeds then it continues to execute if it fails then the threat has to
6:13
exit so um load is very important function it pretty much does everything
6:19
from loading the binary files from the disc to memory initialize a user stack set sets of the initial pointers and
6:27
then it calls jump I'm going to go to the detail I I let me explain the details of jump instruction later
6:35
on so this is start process start process gets the name of the file it
6:41
wants to execute and then using that file name it calls load if it fails to
6:48
load then it exits so in exit it has to clean up all
6:54
the memory chunks that has been allocated to execute the process let me explain how to how the
Loading a program
7:02
load function Works let's say there is L file here this is uh a.
7:10
out and uh there's is memory if load is
7:17
called then a load kernel creates the page table for or given
7:23
threats this is past table and then it opens the file
7:30
this is a file so it opens the file and read the head elf header AED out
7:38
is uh taking elf format
7:44
elf and it has a header part
7:49
here so it first reads elf header into memory and then elf header contains an
7:57
information about how this file is organized uh from here to here this is
8:03
data and from the location of BSS sections and location of all the
8:10
binaries so using Al header um load function pars the file and then load the
8:17
data section to the data segment so it loads the data section to
8:25
the data segment and then read the text to the text segment this is
8:31
data and this is text and then also uh it creat a stack
8:38
for the process and then initialize it so um there was a thread structure
8:47
that has been created already so this uh this is truck
8:54
thread the thread structure contains a pointer to the page table this is is the
9:00
field of this name was PhD and then this P PhD contains a pointer to all this
9:07
data structures data field and text field and even stch
9:13
field so this is how a a pentas load a
9:18
file but loading a file actually do lots of things it read not only it read not
9:23
only the binary files into the memory but also it initialized data segment and it also initialized text segment for a
9:31
process to run um this is a actual load function uh
9:38
it was um to fit this function into memory I have removed a necessary code so some
9:46
quote are missing here and some codes are missing here please don't understand so
9:51
um this line of code this is the code to
9:56
initialize the user stack for a Purp and this is a code for initializing the
10:04
entry point of the stack entry point of binary variable it needs to execute uh load function contains four
10:11
parameters a file name and two variables and this variable contains
10:19
after executing this load function this this variable will contain the starting entry point of this function and this
10:26
variable will contain the stack top of the user TCH that needs to be
After loading
10:35
executed 2 one go after load function finishes the uh
10:44
operating system reads a program file onto memory and it initializes stack and
10:49
it initialized the data and VSS sections and then also it initialized the text
10:56
memory this is the end of load function I just explained the overall
11:02
organization and behavior of pintest operating system for starting process executing process and creating a threat
11:08
now it's time to look at the details of what we are supposed to do first thing is we need to we need to implement a
11:15
mechanism for passing the arguments and creating a
11:21
thread um currently in pintas um it does not have a mechanism to tokenize the command
11:28
line argument so uh it just passes entire command line
11:33
for process execution so uh we have to Mo modify the code after modification uh
11:40
we we have to tokenize individual tokens in a command line and then uh we should
11:47
be able to identify the third name and find the program with file name
11:54
Echo and then we should be able to push the arguments to the user t so in this
12:00
case there is X Y and G so we need a mechanism to push these arguments to the
12:06
user TCH so that the echo can use this step arguments to do it
12:13
job these are two functions to modify of course there are many there
Functions to modify
12:19
can be many other variety of ways for you to implement this this project but
12:25
in in this example I have modified these functions
Parse the arguments and push them to the stack
12:33
um the most important thing is Parish the arguments and push them to
12:39
the user stack so uh inside the process execute
12:46
function it receives entire command line and then it should par the string of the
12:52
file name and then for the first token as a name of the new process to run to
12:57
the thread create function then thre crate function will find try to find the file under the name of file name and try
13:05
to execute it and another function we need to modify is start process in start process
13:11
we have to parse the file name and then we tokenize individual tokens and then
13:18
pushes the parameters to the user stack of the newly created
13:23
process this is um this is a function uh that is provided by stand Library you
13:30
can use this function to tokenize the command line process executes called thread
Program Name Thread name
13:37
creates it consists of two important part first one it pass it passes the
13:43
name of the file it wants to execute as name of thread and then it creates a
13:48
thread with the executing function of start process I'm going to explain the
13:53
details of the two um in start process it allow
start_process
13:59
interrupt frame and it has to load the program and it initialize interrupt frame in user stack it initial two
14:06
things interrupt frame and user stack and then it sets up the argument at the
14:12
user stack then it jumps to the user program through
14:18
inter.it the problem is that in current pintas um it does not have a mechanism
14:26
to initialize the user stch with an argument so this is the process you have
14:31
to implement in this homework which I am going to explain from now on before I
14:37
move on uh let me explain what is getting into the kernel and what is
14:42
getting out of the kernel uh we all know that to get into
Getting into and out of kernel
14:48
the kernel we have to call the famous instruction
14:53
int if you call Int the user program traps into the operating system system
15:00
uh if you're done with something and then inside a kernel you call a instruction Cod I a and then that will
15:08
get you out of the kernel so remember interrupt and and I I'm going to explain
15:14
details of these two instruction from now on this is the layout of the address
15:21
space this is process address space this is virtual address
15:27
space virtual space of a process um V address space a process
15:35
consists of Kernel space and then user space and the user space I did not
15:40
specify that here but there is text and there is uh
15:48
data and also there is BSS well for certain operating system the location of
15:54
data segment and B segment can be switched anyway there is a stack in in
15:59
normal situation this TCH time pointer ESP points to the top of the user stack
16:05
here if you call inup instruction then it automatically
16:11
switches the ESP from user stack to the kernel stack it automat automatically
16:18
does that and then after it switches the stack from the user one to Kernel one
16:24
then it starts to save the registers that has been used by user process and
16:30
the data structure that a kernel is using to store the user's uh register to
16:37
the inter uh to the corner Tech is called interupt frame this is very important data
16:43
structure so um remember when you execute in interrupt instruction it
16:49
switches your kernel from the user to the kernel and it pushes resistance to
16:56
the interrupt frame that resides in the kernel space this is executed in a single instruction
17:02
in so int is as you see very complicated instruction and there's a lot of
17:08
important things let me explain details of the
struct intr_frame
17:14
data structure for interact frame so um there are five registers one
17:20
two three four five and there are uh 12
17:27
byte and there are a bunch of of general purpose registers so interrupt frame the
17:32
the data structure of interrupt frame is defined by operating system as well as
17:38
CPU from here to here is defined by CPU and from from here to here it is defined
17:45
by the operating system so um if you look at the other operating system that
17:51
runs on x86 R CPU architecture then this
17:56
uh part Remains the Same same but from this portion to this portion may vary
18:03
depending on what kind of operating system you're using interrupt frame as reside in the kernel stack and is store
18:10
the users process registers user process
Getting into kernel
18:17
registers let me explain getting into the kernel we call interupt n uh when
18:23
you execute a kernel function such as in tendler and system for the operating
18:29
system saves the registers of the currently executing process it is saved
18:35
in the kernel St and uh it switches the ESP from the
18:41
user stack to Corner St top and it pushes registers let me explain details of
Entering the kernel
18:47
switch pushing the inter frame this is the top of the ESP uh at the beginning
18:52
uh interrupt instruction int switches the uh tep pointer from the user St to
18:58
the kernel de and then inside interrupt instruction it pushes this five
19:05
resistance and then interrupt in instruction finishes
19:10
and then it starts to execute the interrupt Handler inside inup Handler uh
19:16
it pushes um these resistance or values so there is a 12 by
19:24
of some some field and these are General purpose
19:31
resistance so after executing interpret instruction after executing in
19:37
instruction and after saving all the registers that have been used by user
19:42
process to Kernel stack the ESP will point to the ttop
19:48
here that's uh what we have to do to get into the kernel as you see entering into
19:56
the kernel is um expensive PR process it switches the St top pointer and then
20:03
saves love instruction even though um some of the part is executed in Hardware
20:09
by single instruction still there remains a lot of uh register that needs to be saved to the kernel stack by
20:16
software this is how we enter the Cel okay so uh now let's get back to the
20:22
original business the loading load the program does a lot of things first um
20:30
the start process the start process uh passes the program name to
20:36
load and then load instruction find the executable file using the name of the file and load into the
20:44
memory okay then this is the name of the file you we want to load and this is the
20:51
function entry point the starting address of the main function that needs to be executed after the program is
20:56
loaded load function is resp responsible for initializing this fi field and also
21:02
um load function in pintas is responsible for initializing the ESP
21:10
field of a of a user so it specifies in case when the operating system starts to
21:17
execute the program um this field contains the user te top
21:24
address this is the role of the load
21:29
okay that's it um this is body of the start process and I'm going to explain
21:35
the details a little bit so first um here you load the file into
21:44
memory and then at the start process uh you start
21:49
you getting out the kernel and goes into the user mode and start executing
21:54
whatever program that has been supposed to be executed so this part there are two important
22:00
instructions three important one two three in load you load the executable
22:08
and initialize a stack um here it means initialize a user
22:14
stack and then um when you execute the user program you have to pass a set of
22:21
arguments to the user process this part is entirely missing so
22:28
in in this uh red rectangle you have to write your own code to uh to set up a
22:36
stack which I'm going to explain shortly and then here the third part it gets out
22:42
of the kernel and jump to the user program you want to
22:48
execute so this part is missing so this is the part you have to write however before writing this
22:56
part you have to understand the first one and the third one as
Getting out of the kernel
23:05
well so uh in getting out of the kernel this is
23:11
cment it consists of two basic uh assembly instruction first one is move
23:17
second one is jump so in move uh you set the es pointer to points to the current
23:26
step top of the interupt frame so it'll points the top of the interrupt frame
23:31
and then it goes to jump interrupt exit it is specified
23:38
here so it's it's a process of getting out of the chel let's look at the code
23:44
uh this says interrupt exit and then it pops all these
23:51
registers and jump another 12 byes and cause the very famous instruction I
24:00
R so um what it does is um this one
24:06
corresponds to move um zero ESP so as a as a result to
24:14
be executing this statement the the ESP points to the T top of the inter frame
24:20
and then it pops all the resistors and then it skips another 12
24:32
bite and then um after calling in exit it the ESP pointer
24:41
will point to the stack top right here and then it calls IR then as a result of
24:48
calling IR uh Hardware the CPU pops these five resists automatically and restores it to
24:55
the regist and then change the user mode change from the kernel change the mode
25:01
from the kernel to the user so as a result of uh executing IR instruction
25:08
all these five registers will be restored to the CPU and then ESP will
25:14
set to the stack toop address of the user
25:20
stack this is how we get into the kernel and getting out of the kernel there's one thing to remember uh
25:28
when you first create the thread when you first create the thread interrupt
25:34
frame is literally empty um when a process getting into the
25:40
kernel it fills the interrup frame with some
25:46
value so uh when you get into the kernel you pushes the register values using
25:52
interrupt and when you're getting out of the kernel you use I to get out of the
25:59
kernel but for the first created process it has never been uh in the user space
26:05
so inup frame is currently empty when the first process created so um for the
26:12
process uh to be created and getting out of the kernel to the user space um
26:18
operating system has to arbitrarily initialize the interrup frame with some correct values so that's what start
26:26
process do it initialize the interupt frame for the newly created
26:33
thre um the next thing uh is write a function that sets up a St well you
26:38
don't have to write a function you can just simply write the code at that region um let me go to few slide back um
26:45
sorry for for um inconvenience let me go
26:50
back to the few slides back okay all right so now I am going going
26:58
to explain this part before you go into the kernel uh sorry if before you
27:05
actually jump into the function you want to execute at line number three you have to first set up a stack so that set up a
27:12
stack at the user stack this is user St with a proper list of
27:20
parameters this is the function we are going to write so uh we're going to write a
Write a function that sets up a stack.
27:26
function that sets up a step let's consider a function um this is the sorry oops this
27:35
is the address space especially the user one there is a text and dat are data
27:44
region and there's BSS and then um this is let's let's
27:51
assume that um ESP field of interrupt frame contains the address
27:58
ston so um what you are supposed to do in start process is write a function to
28:05
set up a user stch so that when the process resumes in the its control it
28:11
pops the parameters from the user stch and runs the program so the address of
28:16
this TCH toop is currently uh saved at ESP field of the interupt
28:22
frame so uh what you are supposed to do is you have to push the parameter
28:29
um from the St top one by
28:36
one let's consider this command line it contains four
80x86 Calling Convention
28:43
arguments 1 2 3 four the index is one 0
28:50
1 2 three so uh the ARG is four and um
28:58
the array of strings that this is zero and this is index one and this is index
29:06
2 and index 3 so what we are supposed to do is um in
29:12
x86 calling convention uh you have to push arguments
29:18
um first there's there rules first you have to push character strings from left
29:23
to right so the character strings are pushed from from right to left
29:31
sorry this is right to left right to
29:37
left and each character string has to be aligned by four
29:43
byte after you are done with pushing all these character strings you push the
29:48
start address of the character strings and then you push a RGV address
29:55
starting address of the arguments and then you push the number of arguments in
30:01
the command line and then you push the address of the next instruction which is
30:06
return address this is a rules you have to follow when you pushes the arguments to the user
30:15
stack this is it so here um the number of argument is
User stack layout in function call
30:21
four and this is index zero and this is index one and this is index two and this
30:27
is index 3 assume that before pushing the parameter this represent this TP top
30:34
now we are pushing individual strings from right to
30:39
left first we pushes b a r Zer and then we pushes F and then we
30:48
pushes DL and then we pushes SL b/
30:55
LS and the import important thing
31:00
is that you have to align um you have to this align
31:10
this with 4 byte because this argument string is 19 BYT so uh you have to add
31:18
one bite pading to make it 20 bytes and make it 4 byte
31:25
aligned and then you pushes n dominate string
31:31
zero which means that this is the end of the argument string and then you pushes
31:37
the address of each character string 3 2
31:43
1 and zero and then you
31:49
stores the address starting address of starting address of this uh parameter
31:57
set this is um this is just represent here okay and
32:06
then you pushes number four which is number of arguments in this com line and
32:12
then then you pushes written ERS however uh in this case there is no
32:19
return address because this is just create newly created process so once
32:25
this is over there is no way to return just you finish the threat so in the
32:30
return address you push this zero as a fake address so I just give the answer to
32:38
this question why is return address here
32:43
zero because it's just newly created process so once you completely implement
32:51
this function then um you can check if all these Tech frames are properly set
32:56
up using the function Hexum this is the function provided by
33:02
pintas by using uh hex dump you can dump
33:08
the hex map of the inter frame and you should be able to find if
33:14
your stack has been properly set up or not okay this is the end of how you set
33:21
up uh users St good luck
