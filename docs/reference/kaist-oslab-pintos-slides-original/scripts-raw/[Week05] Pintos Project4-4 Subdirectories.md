url: https://youtu.be/yQqNrIiB2hU?si=D7G5jkCkcnrFNj0-

-----------------

0:00
the second director entry points to the inode of itself okay so when you
0:09
implement a sub-directory feature under in the file system each directory entry
0:17
you have to reserve two directory entries reserved two directory entries
0:33
in each directory okay first one is the
0:45
current directory and second one is parent directory so we're gonna come
0:52
into this concept right in the next slide let's go to the next slide okay so
Directory structure
1:01
these are the list of the things you have to implement it's the first one you
1:08
have to implement hierarchical directory structure make the directory entry can
1:13
point to not only the regular file but also the directory file so in Pintas
1:21
there's going to be two types of file regular file and directory file
1:36
okay and okay so this very important
1:44
part make the director and three point to not only the regular file but also directory file but what's the details of
1:51
making this to happen so this is I'm going to explain later the second most important thing is at directory entry
1:58
with dot and dot dot I guess you are all
2:04
familiar with the notion of that okay for example when you perform the file
2:12
list dot and Abba start at C it means
2:18
that list all the files whose extension is C under the current directory and the
2:28
second thing is that dot that I know that you all are aware that data means
2:36
the parent directory so if you perform for example CD change the directory to
2:43
dot dot means change the notion of current change the current directory to
2:48
its parent directory okay so when you implement a sub directory you have to
2:55
reserve the first two directory entry for every individual directory for the
3:04
current directory and the parent directory then of course you have the
3:12
citizens data block for the directory and it can it contains number of entries
3:18
then you have to first resolve the first to the name of the first century is
3:24
current directory and this one points to the inode of the current directory inode
3:32
of current directory
3:39
and the name of the second entry is that
3:45
so it's 2-2 bite and it points to the inode of the parent directory parent
3:57
directory so when you create the new
4:02
directory the directory is going to be not entirely empty it will contain two
4:11
entries first one is dyno'd of the current directory the second one is inode of the parent directory of course
4:17
you can switch the position of the two when you have to be consistent all through your code and then current
4:27
directory in the notion of current directory is global for a process so
4:34
struct proc or thread structure you have to add a field to represent what the
4:42
current directory is of course that's going to be the pointer to the inode for
4:47
the current directory all right then you have to modify the directory related
4:54
functions so file system create opening a file and deleting a file and also you
5:01
have to create a new system cores associated with a directory okay so the
5:10
most important part now there is a concept of path there is a concept of
5:19
pests by enabling a subdirectory you are now having a concept of path when every
5:30
file is on the same directory or every file is on the flat a directory then
5:36
there's no there doesn't have to be any notion of concept but now you have for example slash user slash pintas
5:47
/ source / util /sq I asked that see for
5:59
example then these are all so path is a
6:06
sequence of directories that is can that each of which has a parent-child
6:12
relationship so now there is a concept of path so these are all paths okay so
6:23
the next thing is there are two type of path first one is absolute path and the
6:31
other one is a relative path the way to distinguish the absolute path from the
6:39
relative path is the start of the path the starting character of the past in
6:48
absolute path it specified the entire path starting from the root directory so
6:54
in absolute path the path string start with de bruit / user / sr c / Pintas /
7:07
blah blah blah then thus the path string starts with the root so in that case
7:14
this is called absolute path the relative path is the one start with the
7:22
dot or dot dot for example dot dot slash
7:28
SRC / you till / something something
7:33
something then the first directory in this path is the parent directory where
7:40
am I wherever I am or wherever the current directory is this path starts
7:46
from the parent directory so the location is always relative to the
7:51
current position or current directory so this type of path specification is relative directory or relative path
8:00
okay so this is the this is the things
File types
8:07
you need to do there are five things so
8:12
from one to five the first thing is now
8:18
there are two types of file there is ordinary file or regular file and there
8:24
is directory file so there are two types of inode regular inode for regular file
8:31
the inode for the directory do we really have to distinguish the two yes of
8:38
course because in regular file data block the
8:44
data on the regular file is sequence by strings so you will have the current
8:50
offset and in regular file you will
8:55
model you will move this current offset with read and write function for the
9:03
directory file directory file is special it is not just a sequence of byte it is
9:11
a sequence array of directory entries so instead of reading and writing a certain
9:17
specified bytes you read a directory and three at a time so the way to access the
9:26
way to read and read from in the way to write to the regular file and directory
9:31
files are different so you have to specify that what type of the file it is
9:37
so you will going to add a flag to indicate whether the inode is regular
9:44
file or director file because you have to differentiate only two you will need
9:49
one byte sorry one bit one bit is enough however
9:55
there is no way to save it in in our data structure so you might end up
10:02
allocating whatever the smallest data structure available a single character
10:08
for example it's one bite and in in the data structure construct
10:15
thread you have to define the notion of current directory okay this is pointer
10:22
to the inode of the current directory and and then when creating a new file
10:30
creating a file or opening or removing a file you have to modify so that each
10:39
create opened delete can handle the directory as well as the regular file
10:44
and then you have to the system calls for tracker manipulation you have to add
10:49
system calls for directory manipulation for example for example system course
10:56
for change directory or you will have to
11:02
implement system code for creating a directory or system core for removing a
11:09
directory or stuff like that okay and then you will have to add a special
11:15
directory entry escort dot and dot dot ok let's start our journey first one you
I know structure
11:26
will have to modify the I know structure of on disk so you'll have to add a flag
11:35
indicating if it is a file sorry it is a it is a file of course directory is also
11:41
a file it's a it is a regular file or if
11:48
it is directory file ok so now we'll add
11:56
a regular file for like a director flag so when a file is created
12:01
you should set this flag properly depending on whether it is a regular file or is a directory file ok no oh
Data structure
12:12
before we are moving on to the next topic the remember the most essential
12:19
thing essential
12:24
tasks yes
12:30
designing or modifying data structure so
12:44
that is the most essential part in writing a software you first have to define or design or modifying data
12:51
structure okay this is object and you
12:58
have to define what kind of films it has to have and then next thing you define
13:06
or writing the code or method you can
13:11
it's method when you use it as a function or you use a term procedure or
13:18
even you use the term operation okay so you first defined the object definition
13:26
very clearly and then you define the Associated operation or procedure function or method whatever so currently
13:33
in the first or second step you redefine or modify the data structure to
13:39
accommodate the concept of directory in the first step we modify the data
13:47
structure for on this guy node and then we modify the data structure for the
13:53
thread to accommodate the concept of thread okay in the second part we are
13:58
adding the notion of current directory in the concept of thread of course the
14:06
the modification is very simple and straightforward in the first field in
14:13
the first step we just add a flag and on this kind of structure to represent
14:19
whether a current directory or current file is directory or regular file in a
14:26
struct thread you simply add a single field to denote what the current
14:33
directory is but as as in the most other software
14:42
design modifying a data structure how
14:48
even though it the modification is simple and insignificant it may have
14:55
profound implication implication on the obverse of your design so we're not the
15:03
ready it were to create for the first time when a thread thread is created
15:08
created means that every thread has a
15:13
parent it's a thread is created by the
15:19
parent so in a thread is created it inherits the current directory from the
15:26
parent it inherits the current director from the parents so when a thread is
15:34
created for the first time the current directory of the child process or chart
15:39
thread is the same as the current directory of the parent threads of course there is well there is one thread
15:47
that does not have a parent what is the
15:54
thread that does not have
16:06
parent parent thread this is something
16:21
like Genesis in a creation of the word at the beginning okay what is the thread that doesn't have
16:27
parent thread this is card in it process
16:34
so when the first for the first time when a computer is enabled an
16:40
electricity okay the motherboard is initialized then
16:46
power is engaged in all the hardware and then you know the fan starts to running
16:51
and you hear some mechanical sounds for cooling fans and Nesser cetera and then
16:57
a hard disk is initialized cpricci initialize DRM is initialized and then the software is loaded from the hardest
17:06
drive to the memory and the operating system starts to run and then the very
17:12
first threat which is called init process is created so at that time the
17:19
init process does not have a parent thread it it created by itself for your
17:27
fun so then what yes the process ID what
17:38
is the process ID of the first
17:45
threat in the pintas look at the code
17:55
and find it out for fun okay what is the
18:03
process idea of the first threat in the Pintas when the system is booted okay you find it out for yourself and write
18:12
it to the PS a-- okay let's on to let's
18:18
move on to the next topic so we have modified on disk structure to academic
18:23
accommodate a concept of subdirectory and we have changed the thread structure
18:29
slightly and you added a current director pointer to a thread
Algorithm of file creation
18:35
okay now it's time to modify the algorithm okay so now we have when we
18:46
create the file we first have to parish the path and we have to parse the path
18:53
and then we should create the
18:58
appropriate directory entry at the target directory and also we have to
19:04
create I note so first thing is we have to distinguish it is absolute path and
19:11
relative path and find directory target
19:18
directory where you have to create the file and create a file this is detail
19:24
steps of creating a file well of course we already have a module to create a
19:32
file in Pintas except that it just create a file on the read root directory
19:38
so all you have to do in changing the file creation is to find the right
19:45
directory where you need to create a file and create a file on the path except instead of creating a file on the
19:53
right directory so this is the one you have to modify so examine the path
19:58
and open directory of the path and allocate new inode and initialize the
20:08
inode with the newly created file and add the directory entry to the target
20:14
directory okay but you know it is important that operating system should
20:22
be bulletproof and should be written against any failure or exception what if
20:32
there is no disk space in the course of creating a file so you successfully allocate a new inode you create a new
20:41
inode for a file but you just found that after creating all these you find that
20:46
directory is full then you're creating a new file should fail in that case you
20:53
have to deallocate all the I nodes and bitmaps you have just modified so if you succeed if you
21:01
succeed then you close the directory and
21:06
return success and if you fail if you
21:14
fail then you have to deal locate inode and return success success means that
21:21
return the return the value of variable
21:27
success this is the name of the variable it doesn't mean then you have succeeded the cranium file you may have failed in
21:33
creating file so you have to modify the
21:38
algorithm of file creation of course then if you modify the algorithm of the
21:43
file creation then you're going to modify the algorithm of file deletion as well okay so this is a details of our
Creating a file
21:54
creating a file so this is the name of the file and then in Pintas you specify
22:00
the size of the initial file now initialize of the file so in reality and
22:06
in pentas you always have to create a file in the root directory but after modification
22:11
first you have to parse the path and then create the file on that directory
22:18
so of course as I mentioned before you have to distinguish taps path and
22:23
relative path and parse when creating a
22:28
file you have to add the code to set a flag its directory this is the name of
22:34
the flag you may have added to denote that whether a current file is directory or not and set the path to zero if it is
22:42
regular file and of course you have to add new directory entry to directory
22:47
path and also you have to modify opening
Opening a file
22:54
a file so when you open a file okay
23:00
currently you always find the file on the root directory but now because there
23:08
is a subdirectory and there is notion of path you have to parse the path and find
23:15
a file on that directory and open it so just opening the file itself is the
23:22
same as opening a file as before however there is edit step parse the path and
23:30
find a fire on that directory so uhm window path is absolute you find it from
23:37
the root directory find from the root directory and the path is relative then
23:43
find it from the current directory okay
23:48
that's opening a file and another step is removing the file
Removing files
23:57
it's the same as before in original pin tests we always remember
24:02
the file from the red directory now after modification you remove the file
24:09
from the director specified by the path and if in lieu a file in memory of the
24:18
target file is for directory regular file if it's a directory check there's
24:23
it if it's a regular file you can just delete it but if it is a directory then
24:31
you have to check if the directory is empty or not and here we remove the
24:39
directory file only when the directory is empty and you have to add a number of
24:46
system cores about manipulating the directory files this is change directory and creating a directory reading a
24:54
directory check if the current file is directory and returns a number of then I
25:00
know that's a shaded so first one is changing the directory we get the path
25:06
and parses the path and change the current working directory of the process to the target directory specified as
25:13
parameter to a function and return true if successful and it written follows on
25:18
failure the second thing is creating a direct making a directory sorry there is France right parenthesis
25:27
missing so it creates a directory named dir and return true if you're successful
25:35
and Falls a failure so read the directory it accepts safari descriptor
25:42
name and name let me explain the concept
25:47
of read dir it requires a little bit of
25:52
elaboration in reading of regular file we use a function read and passes the
26:01
parameter buffer and size in reading a file this file is represented by
26:07
descriptor ft and there is current offset and then when our system core
26:13
read is called the operating system or file system specifically reads size
26:21
amount of data starting from the current offset position this is read read dir is
26:32
similar but different
26:37
so in read II I are of course the directory is pointed by F G and in read
26:44
dir party in a directory file is partitioned into directory entry in
26:53
directory file also has a pointer that points to the current directory entry to
27:00
read so if you issue read dir this
27:05
function reads a single directory entry and copies the name of the file in this
27:12
directory entry to the before specified by the name and process the current
27:19
offset to the next directory entry that's what we T I R is for so that's
27:27
the reason why we specify the flag to distinguish the regular file in the
27:33
directory file so when application
27:39
issues a read system core application
27:45
does not know whether a given file descriptor is associated with the director or not so if the application
27:51
issues a read for the regular file the file system passes it and cores just
27:59
regularly read if the application calls a read on the regular file then it calls
28:05
read but if the application is just read on the directory file then we core the
28:11
fire system of course read dir but he should not be returned by the
28:19
media here the current directory dot and the parent directory - not be returned by the region there is a function is dir
28:28
this denotes that whether are a current file associated with the file descriptor
28:33
is a regular file or director file it returns true if the FD represented
28:41
directory and the function I number it
28:46
returns the inode number associated with the fire descriptor okay
Directory entries
28:56
so the last part we have to add special interest directory entries the special
29:02
directory entries dot means it were present itself and dot that represent
29:10
the parent directory so when our directory is created the special entries
29:18
should be added there is interesting property for the root directory this
29:24
let's say this is data block for the with treachery this is inode for the
29:30
root directory and inode has a pointer that points to the data block associated
29:36
itself and the associated data blocks will contain the directory entries for
29:41
directory entries
29:46
for root and the current directory in
29:53
the first century liberal present dying it for the current directory and then the second will be the parent entry
30:01
however for the root directory these two
30:06
points to the same place because root directory does not have its parent
30:12
directory so please be alert and if this
30:22
these two entries should be there always so if a user tried to remove them the
30:31
system core should return fails and it should fail so in the directory entry the first century and the second entry
30:39
should not be removable that's the end of the functions you need to implement
30:46
good luck
