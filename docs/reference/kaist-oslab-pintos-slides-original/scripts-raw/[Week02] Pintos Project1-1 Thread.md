url: https://youtu.be/myO2bs5LMak?si=3rbvoNu2h8X5hjnz

-----------------

Intro
0:00
this is the explanation on the first project the title of the project is threads in this project uh we're going
0:07
to implement three main topics first one is alarm clock second one is prior
0:12
scheduling the third one is Advanced scheduler the first topic is alarm
0:19
clock um we're going to modify the system called timer alarm and system call timer alarms the
Overview
0:27
system called that wakes up a process and takes amount of time time pinus currently uses busy waiting for alarm
0:34
and we're going to modify the pinus or to use sleep and wake up Paradigm for
0:40
alarm this is how timer sleep is implemented in current pinest um in
0:46
current time of sleep implementation the process switches between ready State and running states to check if the alarm
0:53
time has come or not and as a result it only uses two State and it keeps
0:59
crunching the CPU instructions waiting for CPU Cycles um from data structures point of
1:05
view the the the thread is put into running mode and if it calls alarm then
1:12
it it puts puts itself back to the end of the r list and then it executes again
1:17
so basically in timer slip implementation it s it keeps executing
1:24
this uh loop this is how the timer slip is
1:29
implemented if you look at this code then first um when the timer Sy is called it records the current time and
1:39
then within the Y Loop it keeps executing the threat yield so it releases CPU and then when it has put
1:47
into the running state it checks the time if the elapse time is less than ticks then it release release CPU again
1:56
so basically the process that calls time sleep switches between ready State and
2:03
running State let's look at the details of
2:08
thread yield it worth it is worth looking at the details first it gets the
2:14
pointer to the current thread structure and then it in in it disables
2:21
in and then it puts the current thread structure to the end of the rid
2:28
list and then it changes the state of the currently running thread to thread
2:34
ready and then it calls contact switch and then after context switch finishes
2:40
it sets the interrupt level back to its original state this is how the thread yeld is
2:48
implemented there are uh there are five essential functions inside threat yield
functions in thread_yield()
2:54
first one is thread current it Returns the current thread pointer and then it disable interrupt
3:01
and then there's another function which is counterpart of interrupt disable it is interrupt set level it restor
3:08
interrupt level and then um list push back is a function that um places the
3:15
given object to the end of the specified list so the objective of this function
3:21
is put the current thir structure to the end of the r list and then uh it calls
3:28
schedule to the context s the objective
3:34
our um the objective of this project is make the timer slip more efficient we're
3:41
going to introduce the block State and then uh when the uh we're going to
3:46
implement the timer sleep using blocked state so when a process calls timer
3:52
sleep the operating system puts itself to the block State and then operating system is responsible for wake up the
4:00
black process uh frequently checking the time
4:05
Tru using this approach the operating system can save CPU cycles and then also
4:10
more importantly it can save power consumption so um this is the design um
Design: Sleep/wakeup-based alarm clock
4:19
there is um as you see at the beginning of the code there are only two list in
4:25
current Pinos first one is all list and the other thir one is rid list
4:31
rid list a data structure that represent a set of processes that are waiting for
4:37
a CPU to be executed now we are going to introduce you are going to introduce a
4:43
new data structure this is the list of blocked
4:51
threats so um it represents a set of thread that are in the blood state so uh
5:00
when a timer this is a design when a timer sleep is called when when a
5:06
process calls Tim of sleep operating system puts the thread to the SIP
5:14
list and then operating system um keeps checking the timer and then when timer
5:22
is up then it uh wakes up wakes up means move the thread from the slep list
5:30
to rid list that's what wakeup is for so this is the design of our uh
5:38
timer slip algorithm so um this is Implement
5:45
details um first we have to define the Sleep queue so um we have to define the
5:53
slip list you can name it by yourself but um you have to Define it as a SLP list then
6:00
um you have to initialize of course every dat for every data structure you have to initialize it
6:07
properly so the point to think is where do we put the Declaration statement and
6:14
when are we going to initialize it so this is uh this part is left to you to
6:19
implement in implementing the Block Base to timer sleep you have to introduce two
Global tick vs. local tick
6:25
things first one is global take and the second one is the local take um every time the time the basic design
6:33
idea is that every time when timeup Handler is executed the conel checks if there's any threads to wake up to do
6:40
that it first to scan the it first has to scan the blocked thread list and then
6:47
find if any the local tick for the block thread is greater than the cloud tick so
6:53
um in local t e threat maintain the time to wake up
6:59
the time to wake up so um currently our thread structure does not have this
7:06
filled so we have to modify the thread structure and to store the time to wake up then also um for efficiency sake we
7:14
have to introduce one Global variable it's a it's a global TI or whatever um
7:20
just suppos can name the variable uh it stores the minimum value of the local
7:26
text of the threads every time the time interrupt Handler is executed it checks
7:33
The Tick the global tick variable and the current time is less than or equal to the tick if the current time is
7:40
greater than equal to the tick then it means that it has to wake up some uh
7:45
threads in the block list otherwise it doesn't have to scan all the block thread so the reason we introduced a
7:52
tick Global barable is to save the time to scan the slip list so in the data structure you have
Modify thread structure
8:00
to add a new field that represents the alarm time to wake
8:06
up and this is the actual implementation of the code in timer sleep instead of uh
8:14
put itself in the wild for busy waiting uh if um the time to wake up is still uh
8:22
the time has left sometime has left till it has to wake up then it puts itself into the sleep by a calling threat on
8:30
the bus sleep and then it passes the alarm time it needs to wake up as a
8:36
start plus
8:42
take um there's an interesting property here um it is possible that value of
8:49
start may become invalid at two so because there are context SES in
8:56
between and by the time it executes the St the time here it has obtained may not
9:03
valid anymore when executing the line number two so uh we're going to forget it for
9:09
now but if you are really good system programmer you may want to think about
9:14
the way how to fix it so this is the body you want to
thread_sleep
9:21
implement in the thread slip uh function uh if the current thread is
9:28
that I thread you have to change the state of the collar thread to block and store the local T to wup
9:37
through it thread structure and you have to update the global tick if necessary and then call
9:44
the schedule and you should not forget to
9:50
disable interrupt when you insert a thread structure through the thread list
9:55
so the important thing in the thread um in thread slip is changed the state of
10:00
the collar thread to blocked and put it through the Sleep
10:06
Cube time of interrupt uh implementation is hurt hurt of everything uh in time of
Implementation of Alarm Clock
10:13
interrupt uh you have to modify the time Handler and in inside the time ENT Tri
10:19
Handler the operating system should determine which threats to wake up every
10:24
time when the time occurs um when the threads to wake up um
10:31
you have to remove them from sleep queue and then insert it to the red list so you are modifying the red list and then
10:37
slip list so when you modify the slip list and red list you should not forget
10:43
to disable and interrupt disable the interrupt and enable the interrupt before and after modifying the
10:49
list and also uh you should not forget to change the state to the thread from
10:54
slip to ready when you put the threads in the ready list
10:59
uh one one thing you might remember is depending upon how you organize the uh
11:05
slip list the time to identify the threads to wake up may varies a lot for
11:11
example this is sleep list and if you organize the slip list
11:17
and you if you sort the slip list with respect to the alarm clock time so this
11:23
is 10 and this is 101 and this is 105 and so this uh block
11:31
list is sorted with ascending order of the alarm time so when you find the when
11:37
you want to find the thread to wake up you can start scanning from the beginning of the queue and you can stop
11:44
scanning until you find the last thread to wake up however if the this this list is not
11:50
sorted then you have to scan on TI list every time you need to find the thread
11:56
to wake up this is the modified code for time interupt you have to add this
12:04
part so you check the slip list and then check the global tick and you find any
12:10
thread to W up and then you move them to the ready list if necessary and then you
12:15
update to Global clock so this is simple so um in summary um you have to
Summary
12:22
modify a few functions thread in it timer slip and timer intera to make your
12:29
timer sleep function based upon sleep and wakeup protocol there are variety ways to write
Design tip for modularization
12:36
your own code um but U following the design suggestion for
12:42
modularization um you might want to add four functions first one is function that sets the thread tape to block and
12:49
then insert it to the Sleep queue the second function is uh find the
12:56
thread to wake up from the Sleep queue and wake it up wake it up means that put the threads from the Sleep Cube to the
13:03
red list and also you might want to find a right function that saves the minimum
13:09
value of tech that threats have and lastly uh you might want to write a
13:14
function that Returns the minimum volue of the text after you uh write all the
13:20
code you have to check whether you have to check if your code passes the alarm
13:25
test so this is the result and then hope you pass this
13:30
test the second topic is priority scheduling the Pinos uses five for
Outline
13:37
scheduling you are required to modify the pinto scheduler for priority scheduling this is the result of what
13:46
you are required to do first uh sort the red list by the thread priority second
13:52
uh sort a way list for the synchronization primitive such as semor condition variables and locks with the
13:58
respect of the priority and uh we also have to implement the
14:04
preemption and the preemption point is when the thread is put into the riddle
14:09
list so not uh is important that you do not have to check the preempt ability
14:15
every time when time of interrupt is called um in this schedule algorithm uh
14:21
operating system check the preemption only when the new threat arrives to the r list you have to modify two files
14:30
so this is the design um when you exam the red list and uh select the next
14:37
threads to run you get the thread with the highest priority that's one thing and then when
14:45
you uh when uh there a th thread waiting for the lock then when the lock becomes
14:52
available the operating system selects the thread with the highest priority
14:58
that's the two things you have to implement there are three things to
Three things to consider
15:04
consider first um When selecting a thread to run in the r list um we have
15:11
to select the one with the highest priority and when inserting a new thread
15:17
to the Red List uh operating system is required to compare the priority with the running thread with the existing
15:25
one and um we have to schedule a newly inserted
15:32
thread if it has the higher priority than the currently running
15:37
thread so that rules apply to the uh setable thread waiting for
15:43
a a synchronization primitive such as lucks semaphores and condition varable
15:49
when the luck becomes available or or semaphor condition variable is available
15:54
operating system selects the thread with the highest priority
Priority in pintos
16:01
let us explain the priority in pintas uh in pintas the priority ranges from 0 to
16:08
63 There are 64 priority levels and the larger the number the higher the
16:14
priority the default priority uh is set when the thread is first created and the
16:20
default priority value is 31 um there uh Pinos operating system C
16:27
provides two functions F first one is set thread priority that sets a priority
16:34
of a thread to a specified value and then get priority that gets a priority
16:39
of a given threat in uh implementing a PRI based
16:48
scheduling um we have to modify a few things the first thing is that when you
16:54
um when you call inside thread create um
16:59
we like to maintain this R list sorted with the respect to the priority of
17:05
threats in the red list so um when you insert uh thread after creating the
17:12
thread you put the thread respect to the order of the priority so um this is going to be very
17:21
expensive and the second thing is when the threat is added to the riddle list
17:26
um the operating system has to compare the priority of a newly incoming threat with the one that's being
17:33
executed and if the newly incoming threat has a higher priority then we
17:39
have to call schedule so that we can switch out the currently learning threat and push the newly incoming threat to
thread_create()
17:47
CPU so uh this is the code for thread create so we have after unblock the
17:53
thread um we have to compare the priorities of the currently running thread
18:00
compare the priy of the currently running thread and the newly inserted
18:06
one if the newly arriving threat has higher parity then the existing threat
18:14
has to give up the CPU and there are few others
Others to modily
18:23
modify when the thread is unblocked so when when the thread is unblocked from their ready State when it
18:30
puts in the ready State also inserts with the priority order and then also um when a thread
18:40
cars thread yeld also it has to put itself to the r list with respect to the
18:46
priority order and uh there's another function um
18:53
we have to modify that is set priority set priority function changes the
18:58
priority of a threat with the given prior value um in current pinest Set uh
19:05
threat set priority function just simply set the priority with a new priority
19:11
value but in the new function in the new modified algorithm um threat set
19:18
priority not only says the priority value but also adjusted location or position of the thread within the red
19:25
list because uh Red List has to be ser Ed with respect to the priority value of
19:31
the thins in the list we're going to show the details of
Hint:thread_unblock (happy holiday-!)
19:36
how we can just modify the threat unblock code when unblocking a
19:42
threat we are going to place the thread and the red list with respect to its
19:49
thread priority in the existing code uh operating system puts the unblocked
19:55
thread at the end of the ridd list like this list under B push back so we delete
20:03
this code and we put uh put the newly unblocked thread the r list with respect
20:10
to the uh priority and we have to change the
Change the synchronization primitives
20:15
synchronization primitive uh there are luck there sema4 and there are condition
20:21
variables and um when the lock or SEMA for the condition variables becomes
20:26
available we have to make up the waking waiting threat we have to wake up the waiting threat with respect to the
20:33
threat priority this view graph shows how the luck is maintained in pintas Pinas
FIFO lock/unlock in priority-less Pintos
20:41
operating system uses first and first served in um determining the lock
20:47
holder there are assume let's say there are four threats a b c and d and a is
20:56
currently holding a lck and condition of excuting until this point while a is holding a lock B has made a request
21:04
first and then D has make request next and then C has to make request third
21:13
after a release a lock at this time point then operating system decides the
21:18
thread to acquire the lock there are three by this time there are three threads waiting for the lock there is B
21:27
and this is D and this is C among these three
21:33
threads D has the highest priority however even though the D even
21:40
though D has highest priority the operating system just removes that uh
21:46
first threat in the list and assigns a lck so the order in which the lck is
21:52
acquired is from B and D and C so
21:57
between B and D there is priority inversion the the process with the
22:05
higher priority is waiting for the process with lower priority that's because pinus this happens because pinus
22:12
uses five uh lock unlock mechanism in priority based lock unlock
Priority-based lock/unlock
22:19
mechanism the waiters acquir the lock based on this priority um this is the
22:27
same example um there are three there are four threat A B C and D and B has
22:36
made a request and then D has made a request and then C has made a request
22:41
for the lock but after a has released the lock at this point there are three
22:48
threats and uh different from the previous Slide the the waiting list here
22:56
is ordered with respect to the the priority with respect to the priority so
23:03
uh when thread a release the lock the thread with the highest priority thread
23:09
D gets the lock and then after d release the lock the thread with the next
23:16
highest priority thread C gets a lock and B is the L to get a lock we are
Semaphore in pintos
23:23
going to briefly introduce the basic functions in semaphor and condition variable and going to uh point out what
23:31
kind of functions we have to modify in SEMA 4 and condition variable in sema4
23:36
there are three functions first one is semi init which initialize a semi 4 to given value the second one is sem down
23:44
it request for SEMA 4 and if it is required acquired then the the process
23:51
proces but if the process fails to acquire semaphor then it has to block
23:56
and SEMA up releases the semap four so here you have to modify SEMA
24:02
down and SEMA up the luck there are three functions lock init lock acquire and lck release
24:10
as you can see lock is implemented by the semaphore so um uh in order to
24:18
modify the lock primitive based on priority uh you are it is sufficient to
24:24
modify semapo a third function is condition variable um there are four important function in
Condition variable in pintos
24:31
condition for first one is init which initialize the condition variable data structure second one is cond weight uh
24:39
once a process calls cond weight uh the process is put to the block State and it
24:44
waits for Signal by the condition variable and third one is signal it
24:49
sends a signal to a thread of the highest priority waiting in their condition variable and then there's
24:55
another function broadcast isent the signal to all threads waiting in the condition variable this is the functions
25:02
to modify there are two functions you have to modify first one is uh sem down
25:07
and the other is C weight so inside this code you are required to modify the code
25:13
so that when the process is put into the weight list they have to be sorted with respect to the priority and also you
25:21
have to modify semi up and cons signal as well as set priority so that all the
25:27
list can be uh order with respect to the priority there are important issue you
Priority Inversion
25:34
have to consider um first one is priority inversion priority inversion is the one that uh a higher priority
25:42
process is waiting for the process of the low priority consider this situation
25:49
um there is uh three threat threat a and there threat B and threat C and A is
25:57
executing and it has acquired lock at this point then it continues executing
26:03
and C is asking for luck but it is being held by a so a is executing and C is
26:10
blocked from this point but at this point uh a have been executed until this
26:17
point and then B has arrived because B has a higher priority than a uh b gets
26:24
executed while B A gets into the red list and then there's interesting thing
26:29
happens because um currently C is being blocked because waiting for a however a
26:37
uh hands over the CPU to B Because B is uh B has higher priority than a the
26:45
problem is the relationship between c and b c has the higher priority than b
26:51
but uh it turns out that c is waiting for the B to finish this is called priority inversion so so um in our uh
27:00
priority scheduling we have to fix this problem uh in 1997 Pathfinder on MAR has
27:08
stopped because OS operating systems crashed due to the priy inversion the
27:14
engineers in NASA has downloaded their source code of Pathfinder and they
27:20
identify that the crash was ured due to the pride inversion and then they find
27:27
the PA fix the system and upload the code to the Pathfinder and make it work
27:35
this is uh the interesting story and importance of the priority inversion
27:40
there are one thing you will use to fix the prior inversion problem this is called priority donation uh priority
27:48
donation is um the action of inheriting
27:54
the priority of a process to the lockh holder uh let's say there are three
27:59
threads again thread a and thread B and thread c um a is uh executing while holding a
28:08
lock at this point and then a is executing and at this point the
28:15
C is asking for a lock but lock is being held by threat
28:23
a at the time if you compare the priority of C and A then a has low
28:30
priority so when C is asking for a lock
28:35
and finds that a is holding a lock then C inherits its priority to a so that A's
28:43
priority gets boosted to C's level right after a A's priority has been boosted
28:50
from lower level to C's level you can see that B has arrived the system in the
28:56
original scheduling algorithm b suppos b is supposed to preempt the process
29:01
process a however in this case process A's priority has been boosted to C Level
29:08
so there is no way for B to preempt process a so the process a continues executing
29:16
at the C's priority level and once it's over then um the lck is hand over to
29:24
process C so process C gets executed and then after process C finishes
29:30
execution it releases the lock and then process B finally gets an execution
29:37
opportunity so via donating priority to the lock holder we avoid priority
29:44
inversion this technique is priority donation um in the system without
29:51
priority donation there is a lock and this is L lock is currently allocated to
29:56
threat one with priority 10 and after thread one acquires the lock uh three
30:04
thread has arrived thread one uh thread two thread three and thread four each of
30:10
these thread has priority 9 12 and 8 without priority donation the priority
30:16
of thread one remains as 10 but if we employ priority donation then um the
30:25
lockh holders priority becomes a priority of the highest priority threats
30:32
so here there are two three threats T2 T3 and T4 and T2 has the priority 9 and
30:40
T3 has priority 12 and T4 has priority 8 so among these thread
30:47
priority thread three inherits its priority to thread
30:54
one this is priation there are few issues to consider first
31:00
one is nested donation consider this
31:05
scenario um there are three threads thread two oh Sor thread one thread two
31:11
and thread three and thread one has priority 10 and thread two has priority
31:17
9 and thread three has priority 7 somehow first uh thread one is holding a
31:25
l a and then thread two has made a request
31:31
for thread lock a and it's got blocked however uh thread two is holding
31:39
a lck b and um thread three is waiting
31:45
for a lock b to be released while holding lock C so there is a chained
31:51
lock holding relationships what if there comes thread four it has friend thread
31:56
14 and IT issues luxi okay and then um it thread four is
32:05
make a request for luxi and then because of the priority donation it will donate
32:11
this priority to its lock holder T3 and make the priority of itself
32:17
24 7 to4 however here T3 again donates its
32:25
priority to this to its lock holder so the priority of T2 is updated from 9 to
32:34
14 again um T2 inherits it Priority to
32:39
its lock holder and it inherits priority 14 to its lock holder so the priority
32:46
level of T1 is updated from 10 to 14 this is called nested donation so in
32:54
your uh priority in your priority donation in in implementation you have
32:59
to implement the N donation feature in your priority schedule algorithm next
33:05
topic is multiple donation thread one is holding three
33:11
locks lock a lock b and lock C the
33:16
original priority of thread one is 10 T2 makes request for lock a its priority is
33:24
12 so the PRI thread T1 is donated the priy of 12 so 10 changes
33:34
to 12 and then T3 is as make request for
33:39
lock b and then it has a priority 11 so it does not donate its priority to each
33:45
lock holder because the current uh priority that has been donated by T2 is
33:51
12 now T4 make a request for C and it T4 priority is 13
33:58
so uh priority 13 that has been held by T4 is larger than priority 12 so finally
34:07
um the priority of T1 becomes 13 let's assume that T1 has
34:13
unlocked the lock C and then as a result of unlock uh lock C the lock is Rel uh
34:22
allocated to T4 but um then after
34:29
releasing uh Lux C t1s priority should
34:34
not become its original priority 10 but the priority of T1 has to be
34:41
updated to the largest priority that donated the priority to T1 so here um
34:50
priority of T1 now become 12 that is
34:55
multiple donation so in your uh priority donation mechanism you have to implement
35:03
a nested donation and multiple donation the idea is simple for supporting
Data Structure for Multiple Donation
35:10
multiple donation uh thread has to maintain the list of donors so every
35:18
time it releases luck um it searches the donors and get the highest priority of
35:24
the remaining donors for nest donate uh you have to maintain the luck that it
Data Structure for nested donation
35:30
waits for and then once you inherited a priority you have to check if you need
35:37
to inherit the current priority to your child that's the way you implement a
35:43
nesty donation this is the functions to modify in PRI donation you have to
Implementation of Priority Donation
35:49
modify the init data structure and you also have to modify luck
35:56
acquire in lock release and set priority in lock acquire if the lock is not
36:03
available you have to store the address of the lock uh you have to store the
36:09
current priority and maintain the Donate threads in the list and then you'll have
36:14
to donate the priority and when the lock is released you have to remove the threads that hold the lock on the
36:20
donation list and you have to update the priority properly and also when you set
36:26
the priority you have have to uh set the priority considering the donation but
36:33
that's the issues you have to modify and this is the result of the test

