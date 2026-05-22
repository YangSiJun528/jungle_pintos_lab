url: https://youtu.be/4Vg66wWwkXE?si=et5lvozGrkyBRALs

-----------------

0:00
I'm going to explain how we can build a
0:03
indexed file in Pintas fire system in
0:09
original Pintas the fire size has fixed
0:13
when it is created it is somewhat very
0:16
limited so in this project will modify
0:19
pinches file system to change the fire
0:21
size dynamically in maximum file size
0:23
will be 8 megabyte they say as far as
0:26
you're going to modify so how this is
0:33
how we allocate the block when Pinter's
0:35
creates a file when creating a file say
0:41
file a is saved the start block address
0:45
and length to the inode and then when
0:51
you create another file again you say
0:54
the start block address and length to
0:56
the inode so this is disk this is disk
1:04
so but in fire system we perform append
1:10
very frequently we pend a block to the
1:14
end of its new file append another file
1:17
and append another file ok but in
1:21
painters file system we cannot do that
1:24
in the current fire system structure
1:26
because a file is represented by the
1:32
start address the size this means that
1:39
all the blocks associated we the file
1:42
should be placed at the consecutive
1:44
region on the disk
1:50
consecutive region however in in this
1:54
configuration there is file a however
1:57
there is we if you want to extend a file
2:03
but the next address has been occupied
2:05
already so we cannot extend the file so
2:09
we have to fundamentally change the way
2:10
the inode represent a file so after
2:18
modification after Mari you're going to
2:20
modify the file structure in painters
2:24
operating system how
2:25
let me show show you in in after
2:30
modification an inode is going to have a
2:33
bunch of pointers and each pointer will
2:37
point to the data blocks at this at the
2:42
creation of the first file you allocate
2:46
1 2 3 4 5 6 7 8 lakhs and with 8
2:50
pointers at the creation of the next
2:54
file file B you create another file and
2:58
with 8 blocks and there are 8 files
3:02
sorry 8 pointers and if you want to
3:06
expand a file a then you allocate
3:09
another block wherever there is a free
3:13
block and set the ninth pointer point to
3:18
the newly allocated block then we can
3:23
extend a file so that's what you are
3:27
required to do okay so let me go into
3:35
the detail a little bit the C
3:38
cooperation C cooperation
3:42
it changes the current offset of the
3:44
file then let's consider this case this
3:52
is the size of the current file let's
3:54
say this is a 5 by 5 to a byte times 8
4:00
blocks this is 4 kilobyte
4:03
so the current current file size is a 4
4:06
kilobyte but the application calls seek
4:12
system core and sets it to 10 kilo byte
4:17
for example then current offset is
4:20
folklore bytes
4:21
I'm sorry I think it's too large let's
4:23
say let's it's a 6 6 Club i'ts okay and
4:28
then current offset pointer is updated
4:32
to the position where there is no 5
4:36
lakhs updated okay so when you modify
4:44
the fire up fire system operation and
4:47
you when you modify the file to the
4:49
indexed organization you have to check
4:52
you have to modify the seek fire system
4:56
core or so and when the seek actually
5:00
six beyond the size of the fire it does
5:02
not change the fire size nor does it
5:03
allocate the blocks but it just updates
5:05
the offset and when a write is cord and
5:13
offset and when the actually write is
5:16
called ok at this position this block is
5:20
still free and it has not been allocated
5:22
to any of the fires but offset is update
5:26
to this point at this point if you if
5:29
you or application calls right system
5:32
core then it places a contents at this
5:38
point and initializes the intermediate
5:46
block that hasn't been accessed anyway
5:49
with some initial value okay this is
5:55
called punch hole and Panchal is
5:57
initialized with the value 0 so this is
6:02
very interesting part of the sake
6:04
operation
6:07
there are three things to do first you
6:10
have to modify the undisguised structure
6:13
and then you have to modify the code
6:15
that uses under sky node that include
6:19
changing fell officer to the block
6:21
address and creating a new inode and
6:25
deleting an inode and also you have to
6:27
modify and create the function that
6:30
handles extension of a file okay so um
6:36
this is an underscore in Carpenter's so
6:40
um this is sector start address and then
6:46
this is length of the file so in current
6:49
time in current Pintas the file class
6:51
file block a file is represented by the
6:56
cert address and length so we are going
7:00
to modify that so um we are going to
7:07
modify on this kind of structure for the
7:10
file so that we can extend file so this
7:15
is sample same point note this I note
7:22
there are some number of direct index
7:27
and the indirect block and then double
7:31
indirect block so in the single indirect
7:34
single direct block okay
7:37
the number there are direct blog entries
7:40
number of direct pointers so that points
7:44
directly point to data blocks and then
7:48
indirect block points to the point of la
7:52
that points to the data block now third
7:55
one is double indirect block so it
7:58
points to the pointer block that points
8:01
to the another pointer block and then
8:04
that points to the data blocks so this
8:06
is two level interaction
8:11
okay so this is the layout of this is
8:16
the actual layout of the file that
8:19
belongs to a single file
8:21
because in Pentos each block each inode
8:26
occupies a single I a single block so
8:29
the inode can be very large so we don't
8:33
want to waste any of the space so for
8:35
that reason we are going to allocate as
8:38
many as direct pointed as possible so
8:43
here there are from 0 to 124 this is a
8:47
gorgeous example I know a structure
8:51
simple I know so if you want to use
8:55
different idea for the fire organization
8:58
you're welcome to use your own idea and
9:01
implement it anyhow there is 128
9:05
sorry 124 direct blocks and then there
9:11
is single indirect and there is another
9:13
double ender this is total number of
9:15
pointers in the inode okay so in the new
9:24
I know structure there is file length
9:26
and there is magic number and then there
9:31
is 126 pointers this is total of 126
9:39
pointers okay and then here from here to
9:44
here there are 128 integers and each
9:48
integer is 4 byte though 128 integers
9:52
times 4 byte is 512 byte so here a
9:57
single inode single on this kind of
10:00
occupies entire sector
10:07
okay and then next thing to do is
10:10
compute the sector number from the fire
10:13
offset so we have to compute we should
10:17
be able to compute the sector number
10:19
from the fire offset so Pinter's already
10:24
finds a function block sector two by
10:27
block sector T this is the region valid
10:31
type of return value and this is byte
10:35
two sector so it provides an offset and
10:39
it returns the sector number associated
10:41
for that upset we have to change that so
10:44
this is byte 2 sector function and it
10:48
converts the position to a sector number
10:50
so that you can you have to change this
10:53
function properly and another function
11:02
if the change is the code for creating
11:03
an inode so there is function called I
11:06
know it create in are not create you
11:09
have to there I will create functions
11:12
supplied is supplied two parameters
11:15
first one is sector yes there is length
11:19
so sector is location where the inode
11:21
should be created and the length is the
11:24
size of the file when at the time it is
11:26
created so in originally Pinter's
11:32
allocates contiguous blocks and save its
11:34
start address however here now we modify
11:37
the code of the block address all blocks
11:39
are located it's simple okay and then
11:47
deleting and I knows it should be
11:49
changed slightly when we delete an inode
11:53
we have to do look don't have sorry you
11:59
know I know it has so we have to add
12:02
block do locating code at the I entered
12:04
Clause because this says inode
12:06
and then there are bunch of pointers
12:12
and then there is a blocks associated it
12:15
so when you delete it I note you have to
12:19
do locate all these blocks that means
12:23
that you have to set the free block
12:25
bitmap for the these data blocks to zero
12:34
now it's time to handle an extension of
12:38
a file when the first size changes it
12:41
allocate a new block and update a Dana
12:43
block pointer in the inode and then you
12:47
have to fill the allocate blocks with
12:49
zero I think that's the basic steps you
12:52
have to do
12:54
there's function inode right at okay and
12:59
then here of course we have taught locks
13:05
you have to acquire appropriate locks
13:08
and then when you write beyond the size
13:12
of the existing file then you have to
13:14
update some field the next topic is
13:20
subdirectory original pintos has only a
13:25
root directory but not the other sub
13:29
directories so we'll implement the
13:32
concept of super subdirectory feature to
13:35
make the fire system more more
13:37
reasonable and you will bring in the
13:42
concept of hierarchical tree structure
13:45
this is the list of fires you need to
13:48
modify the course of creating the
13:50
subdirectory concept in the painters
13:52
file system ok so this is the structure
13:57
of the directory in the original Pintas
14:01
there is only root directory there is
14:04
only birth directory and so directory is
14:06
flat and root directory of course has
14:11
inode okay and then I know it has the
14:15
associate data block the data block has
14:19
a directory entry
14:22
and every directory entry in pintos file
14:27
system is just a file okay
14:30
so file means it's a it is a file name
14:36
and inode number pale now based upon
14:43
these it allocates a file inode and then
14:48
it contains a data plus associated with
14:51
the inode okay now we are going to
14:55
change the concept of this flat
14:58
directory structure into a hierarchical
15:01
directory structure so here the root
15:06
directory and can contain the file a
15:12
regular file as well as another
15:16
directory that's the important part
15:24
okay so a directory contains a regular
15:28
file as well as a directory file the
15:33
important thing is that in when there's
15:38
concept a higher-up a directory
15:39
structure there is two pre-allocated
15:44
directory entry in every directory
15:46
structure which one is which is current
15:48
directory and then parent directory so
15:52
let's look at the details this is let's
15:55
say this is root directory and then
15:58
there is i know'd okay and then i know'd
16:04
contains points to the data blocks of
16:07
the directory and each of these are
16:09
directory entries okay directory entries
16:16
may refer to the normal file or may
16:22
refer to the another directory trees
16:30
and then in directory entries the first
16:35
twos directory entries are reserved for
16:39
the special profits the current
16:41
directory and then parent directory
16:44
however the root tree for the root
16:47
directory there is no parent directory
16:50
so it points to itself so these are the
16:54
data structures you are going to
16:56
implement in realizing the hierarchical
16:59
directory structure
