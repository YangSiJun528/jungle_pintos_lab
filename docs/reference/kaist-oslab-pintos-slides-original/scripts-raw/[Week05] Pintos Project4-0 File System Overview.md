url: https://youtu.be/bqtjcc7-_yA?si=XcxTIi-JIpqSXjue

-----------------

0:00
hi we are going to explain the detail steps of implementing the project number
0:06
for this is that the file system there
0:12
are basically three things in project number four we are going to implement the buffer cache in the painters file
0:19
system and then we are going to extend the painters file system such that the
0:24
individual files are represented by the set of blocks indexed and make it extensible and then we are going to
0:31
implement the sub directories before we
0:37
move on we're gonna explain this a few basic concepts in the fire system one of the
0:45
most important data structure in the fire system is inode sorry
0:55
inode represent a file on the disk a file on the disk so every file has its
1:07
own inode as a data structure and what does it represent
1:13
it contains a size of the file how large is the file and it contains the location
1:20
of data blocks that belongs to a file so it may be a pointer to the disciplines
1:26
that belongs to a file and also it contains a permission and the time it
1:34
has been accessed for the last time and then the most recent modification time
1:40
and etc around there are two types of vinyl there are on this guy note and in
1:50
memory inode so under disk there are array of I knows that each of the inode
1:56
represent the files on the disk when when the operating system wants to
2:02
access a file on the disk it has to read the inode from the disk and based upon
2:08
the disk based inode it builds a in memory inode so in memory inode is a superset of on
2:17
disk inode so this is on disk i know'd and in memory i know or in core inode
2:26
represent a data structure that contains on disk I note and some other
2:33
informations the other informations include the disk location of the
2:40
undisguised and also it may contains the fire system that the inode belongs
2:47
another important data structure is a file object so there is a node object
2:53
and there is a file object fire object
2:59
represent an open file in UNIX in UNIX
3:07
when a file is opened the operating system defines current offset this is
3:22
one of the most important concepts in modern file system current offset denotes the position within a file where
3:30
the read or write application write read or write system course has to be applied for example here this is big file and
3:38
this is start of the file and this is the end of the file and then UNIX
3:45
operating system defines an attribute called current offset which represent a
3:51
location where the read or write operation has to be applied once they
3:57
read the write up like a system for is applied then offset is updated by the
4:03
amount of data that has been read or that has been written - based upon the
4:09
raid wide system core so it is updated [Music]
4:16
okay so there are two concepts I note and file object before we move on let me
4:30
introduce another basic concepts there are a file this is just a regular file
4:36
and this is directory and this is bitmap so each file has actual data this is the
4:46
data blocks it may be music file it may
4:54
be video file may be Microsoft Word file
4:59
or it may be a C code anyway it may contain the music contents video
5:07
imageries or documents or C files and then this is actual contents of the file
5:13
however every file has its associated I note and I know it contains as I told
5:21
you before it contains a size it contains permission and each ainít
5:26
contains a pointer to the data blocks which it holds okay so this is how
5:32
regular block regular file is represented in this form the file the
5:39
the block of a file is represented by the start address and start address
5:49
sorry this should be sighs my apologies
5:55
so at the beginning of the file this is start address and this is size so in
6:07
this type of representation okay
6:13
data that belongs to a file is represented by the start address on the
6:20
disk and the size of the data that it
6:25
occupies on the desk so it may if it occupies three megabytes then fire size
6:33
is going to be 3 megabyte if it is B movie file and then the size of the file
6:40
corresponds to 1 gigabyte then this file occupies consecutive 1 Giga bytes of the
6:47
data blocks consecutively that is very important so this is how Pinter sorry
6:54
this is how pintos represent a file this is current form and the second thing is
7:05
directory directory is a set of files so
7:14
I guess all of you are very familiar with the term directory accepted what
7:20
you are probably don't know that exact definition of the directory directory is
7:25
a set of file name and ID number pair so directory is file name and I know the
7:36
number pair yeah this is a definition of
7:42
directory but in Pentos directory itself is also a file that is standard way of
7:50
representing a directory in UNIX file system the same as saying the same as in
7:56
UNIX pintos defines a directory as it's as a file it means that directory has
8:04
its own inode then I know it has a
8:09
pointer to the data block location different from the regular file directory file contains an array of file
8:19
name and I know pair and each pair is
8:25
represented by file name and inode number I naught and each entry is called
8:35
entry in the directory which is called directory entry the third concept you
8:42
might want to know is bitmap bitmap is array of bits as very
8:47
easy so 0 0 1 0 0 1 0 0 something
8:53
something this is bitmap we're going to explain
9:01
why and how it may be is applied in to meet this operating system anyway in
9:10
Pintas bitmap is also represented as a file so represented as a file it means
9:19
that bitmap has its own inode and I know it has location or contains location of
9:26
the bitmap ok so that's very coarse horse explanation and basic concepts of
9:35
painters operating system and pinches file system let's move on this is fire
9:41
system layout in Pintas let's consider currently i guess you have defined
9:47
painters file system with 8 megabytes fair-sized VAR system size 8 megabytes it's pretty small very very small fire
9:57
system partition and in pintas the block size is 5 2 byte it's a block block the
10:06
term block in modern operating system is unit of i/o to the disk so that's what
10:18
we call the size of the block in Pintas operating system block size corresponds
10:23
to a 512 byte these things normally the block size is 4 claw byte so since the
10:32
file system partition is 8 megabytes so there are 16 thousand blocks in this vs.
10:40
and partition and this is a total farce
10:48
system partition layout so there are 16,000 locks from block number 0
10:57
number sixteen thousand three hundred eighty-three
11:02
this is last line so every block is located by its index okay so that is the
11:11
basic size from from block zero to plus
11:16
sixteen thousand three hundred and eighty-three so many blocks and then
11:23
these are the details layout of the pinches fire system partition one thing
11:32
you might want to remember is in Pintas
11:37
inode disk sorry this is under sky note
11:43
on disk inode is 512 bytes large it's
11:50
very big of course I node contains uh of course I
11:58
know does not use all its 512 bytes space only fuel only the fraction of 512
12:06
block is used for containing an information from the I know anyway the
12:13
first block contains an inode for bitmap file I'm going to explain what the
12:21
bitmap is shortly later in the second block which is block one contains an
12:27
inode for the root directory and from
12:36
block two to block five it contains a block bitmap okay so blood zero contains
12:48
an inode for the bitmap file and these are the actual contents of the bitmap
12:54
file so the block 0 contains inode and it should contain the start address of
13:00
the bitmap file like this and block
13:06
bitmap consists of four blocks from plop - to block five and then block
13:15
number six is contents of the directory
13:20
so it contains inode and file name pair
13:25
for the files that resides in the root directory so this says I know the number and the
13:35
file name so this is detail structure of this this part and then there comes
13:41
bunch of data blocks okay so let me
13:47
explain what the bits map is bits man yes an array of bits of course and each
13:58
bit represent whether we associate that blood is being used or not so there are
14:06
four blocks and each block has 512 bytes and each byte is 8 bit so we need to
14:16
allocate 16384 bits to represent whether
14:24
each of the blocks in the fire system partition is being used or not so we are
14:29
we there are 16384 blocks in this fire
14:35
system partition so we need to allocate this much bit to present whether each of
14:43
the block is being used or not so we are locating four bits for blocks but if you
14:49
double the file system partition from 8 bytes from 8 megabyte for example for 32
14:55
megabyte if you if we increase the fire system partition size from 8 megabytes
15:03
to 32 megabytes then of course we have to allocate 16 blasts for the bits map
15:10
not 4 because we have to put ripple the number of blocks that has been used for
15:16
block bitmap so this is fire system layout in pintas
15:25
this is the details okay so the first
15:33
block was ie note for the bitna and the
15:38
second block was I know it bit to Matt for root directory and then this from 2
15:45
to 5 was bitmap and this was root
15:51
directory okay and then there is data blocks this is
15:59
plain data blocks let's look at the
16:04
contents of the block number 0 this is inode block for the bitmap and in
16:10
Pinter's each file block is represented by the start address and the length okay
16:17
so it says the start address of its contents is block number 2 a and length
16:26
of the file is plus the length of the file is 2048 byte this is Uni despite
16:34
and there are cons matting number and you know this is a part of this data
16:39
structure design the integer array of
16:45
125 integer is bringing unused in this data structure so also assume that this
16:51
is for byte in this session that this is for byte and then 120 125 times 4 which
17:04
is 500 byte is being unused all right
17:10
well this is the design of painters file system and if you have time to modify
17:15
and you if you have time to make it look better and then you are welcome to
17:21
modify this file system partition and I'm sure that you should be able to do that ok and then let's look at the root
17:29
directory your start address is 6 there are the data block for the root
17:35
directory file starts and block number six and then the size of the fire is 320
17:43
byte so it fits within a block because the block sizes 512 byte then there
17:51
comes huge space of unused space okay so
17:57
this is the I know structure and then
18:03
let's look at the contents of root directory as we have covered before
18:10
directory is an array or sorry directory is a set of file name and inode so file
18:21
name and I not in Pentos so in painters
18:28
data structure directory entry is an array of file name in I node and the
18:36
length of the file name is fixed to 14
18:41
byte at maximum that's how we do how the
18:47
pintos defines if our name so in Pintas the file name cannot exist 14 characters
18:53
however in modern operating system the length of the file name is virtually
19:02
infinite you can use 100 character file name or 200 character file names anyway
19:10
this is how Pinter's represents a directory okay so from block number 7
19:17
there comes a data block so data plot means you can use you can use the blocks
19:25
that starts from the block number 7 to 16,000 you can use those blocks at as an
19:34
inode or the data block that belongs to an inode in this in this far system line
19:42
layout inode number 7 a black number seven is containing an
19:49
eyelid block and the name of the file we don't know we don't know what the name
19:55
of the family is but lets me let me get
20:01
back to the point later so anyway the inode datablock starts at eight and the
20:09
length of the fire is ten thousand twenty-four so the length of the fire is
20:16
two blocks two blocks and it starts from
20:21
eight so this is the data block for my for a block that points pointed by the
20:30
inode block which is stored at block number seven and there's another block a
20:38
block number ten contains an eyelid block and it's data block starts at
20:44
eleven and it consists of 2048 byte
20:49
which corresponds to four blocks so it'll be eleven twelve if we have more
20:58
space in thirteen and fourteen then this
21:04
file will consist of four blocks okay and I know that us that is stored at
21:13
block number ten is pointing to the plant number eleven and it contains an
21:19
information about the file size and the fire size 2048 like this here okay and
21:29
then let me get back to the root directory structure the first entry in
21:36
the root directory stays that name of the file is my file and it's nine inode
21:42
number is number seven so it is I know
21:48
the number but actually it represents the location of the Aisne associated inode so file name is my file and the
21:56
associate inode is store at the block number seven the second
22:01
entry is file that see this file name and then the number I note number is ten
22:07
but it represent a location I know location of the inode so the inode for
22:13
file that C is stored at inode number 10
22:21
there is a very very interesting phenomenon here as you see in I note
22:29
there is no file name if you look at the
22:36
inode there is no file name field no no
22:43
you know what in modern operating system in modern
22:48
fire system file name is not the part of
22:54
file attributes so for us human banging
23:00
we recognized a file by the string which we call file name but from computer
23:07
systems point of view file name has nothing to do with the file itself it's
23:14
not a part of the final attribute directory data structure relates a
23:21
character string calls file name to its inode so that's the important
23:27
interesting characteristics of modern fire system ok so there are two types by
23:35
note first one is in memory I know and the second one is under sky note so I
23:41
know to represent a file on the disk all right so let's first look at the inode
23:49
we call in memory I also sometimes we get calls discarded in core inode in
24:00
printers the name of the data structure for in memory inode is trapped and
24:06
I note and in memory I note contains the
24:14
address of the under sky note on the desk it represents a blonde number where
24:19
I know the stored and then it contains
24:25
the disk I note so that is it and then
24:34
it contains a flag it contains a flag whether to delete the file or not okay
24:43
let's look at the actual data structure the easiest part the sector this is the
24:51
location of the inode on the desk so this part is done and this the last part
25:00
is the data structure for on this kind note so as I told you before in memory
25:06
inode it's a superset of the undisguised so it this says in memory I note that
25:12
says in memory inode or in core I note
25:19
it harbors the under sky note this is on disk I not
25:27
and it contains a sector which represents a location of the inode on
25:34
the desk and then it contains the flag
25:39
could removed to denote whether a file
25:44
has been deleted or not of course the file can be deleted but operating system
25:53
does not immediately deallocate the in memory inode when a file is deleted
26:01
normally operating system dl locates the in memory i knows in a synchronous
26:08
manner so when you delete a file a ring system just marks that this file this
26:15
data structure in memory I know needs to be do located and do it later at some time so that's what is this is
26:24
for and as we covered before some fires
26:33
are not writable for example the executable file or the files that are
26:39
being loaded to disk they shouldn't be modified while the operating system modifies it so we
26:44
propose the printers operating system defines the field in the in-memory node
26:51
did you note whether the file can be writable or not so the test denied right
26:56
count the reason this is a count not the flag is that there are multiple
27:01
processes accessing the file in that case the file cannot be modified until
27:07
none of the processes are accessing the file so for those reasons minutes file
27:15
system defines a deny write count not the denied right flag and the last thing
27:28
there is open count it represents a
27:33
number of processes or number of open system cores that has opened a given
27:40
file so that's a structure of in memory inode okay
27:48
next part is more essential data structure called on disk I note it
27:53
represent a file on the desk so the name of the data structure is chopped
28:00
I know disk this is very large this is
28:08
512 byte that's very sad and of course
28:13
unfortunately out of 512 byte
28:21
500 bite is not used okay
28:33
this is not good however that's the way the pintos defines inode so if you have
28:40
time then for some time on modifying this data structure and make it more efficient give it a try it's very easy
28:47
anyway the important part is how Pintas operating system defines a file in
28:55
Pintas operating system fire system represent a file as a single block of
29:02
large chunk it is pointed by the start address and the size this is how the
29:14
pintos fire system defines a file okay
29:23
and this is the data structure of a directory object directory object is a
29:36
format of data block for the directory
29:44
directory file so directory file has its
29:53
own inode its own I know and i know'd contains a pointer to the start address
30:00
of the data block and its size and it'll contain something so this is in memory
30:13
data structure that represent a directory block so it contains a struct
30:21
directory contains the pointer to the Associated inode and the position the
30:29
position defines the next directory entry to read or write write there are
30:37
so it were present and open directory there are two fields I know the pointer
30:44
to the Associated in memory node and then POS is a
30:49
position of the next director entry to reading right that yes directory object
30:59
and let me explain the directory entry directory data block consists of an
31:06
array of directory entries which I have explained default then each directory
31:14
entry is actually file name or inode number pair right so I know the sector
31:25
as a sector number of The Associated inode and then there comes a file name
31:33
and then that slot may not be in use in that case we need a flag to represent
31:39
whether being current slot is being used or not so the actual data structure for
31:46
directory entry is represented by Dinitz sector and the character string the
31:54
maximum number which is 14 plus 1 and then a flag to do not whether a given
32:01
directory slot is being used or not so as you can see directory entry this is
32:09
the data structure for directory entry
32:17
so in this case when you never use search whenever you search whenever you
32:22
need to find a certain inode for a given file name every time it needs to scan
32:28
the directory block to find a matching filename well if there are 10 files or
32:35
15 fires linearly scanning the 10 elements of 15 elements may be
32:40
reasonable but what if directory contains like three hundred thousand
32:47
files in a directory then if it is not
32:53
sorted or if or it does not organ if it is not organized to it
32:59
certain search structure such as red extreme B plus tree or red black tree
33:04
then linearly scanning the array of 3,000 elements requires substantial
33:12
amount of search it takes very long ok anyway in Pintas the directory is an
33:20
array of directory entries and directory entries are not sorted and or okay the
33:30
next data structure is block bitmap the block bitmap represent whether a given
33:38
block in the fire system partition is being used or not the data structure
33:45
name is Freema so it is a bitmap to represent the status of the blocks in
33:51
the fire system partition and the bitmap is stored as a fires which means that it
33:58
has its own inode and it has two films the number of this plus in the entire file system as bit count and actual bit
34:06
array there is not important data
34:15
structure of file struct it is created and when a file is open
34:25
remember for closed file this data structure is not created so this struct
34:34
fire is create allocate and creating and file is open and it contains a pointer
34:39
to the inode and the most important attributes of the struct file is
34:46
position it represent the position of a file where the read and write operation
34:52
should apply and it has a field to indicate whether a file is write over or
34:58
not so in this project we have to do
35:09
basically three things first one is implement buffer cache and second one is
35:16
make the file system define abstraction indexed and extensible and implement the
35:24
subtract trace so for buffer cache the
35:30
purpose of buffer cache is using the parts of memory as a disk so it's the
35:37
opposite concept of virtual memory in virtual memory virtual memory we are
35:51
using the part of disk as memory now
35:56
using disk as memory in buffer cache is
36:05
the opposite we are using part of memory
36:10
as disk
36:18
okay so for buffer cache we allocate buffer cache we are we are going to we
36:27
are going to allocate physical page to occur the mate accommodate 64 disk
36:34
blocks and we do not cache the data blocks to this perfect cast when reading
36:39
or write two data blocks we are going to save it to the buffer cache and then
36:45
once we are done with accessing the block sometimes we have to save the modified data blocks back to the disk
36:52
space or when the fire system shut down and for the the file current Pintas
37:04
represent a file as a single extent as a single consecutive block which contains
37:12
the start address and size so in Pintas
37:19
inode it contains two films the start address
37:25
of a file blocks and size but what if we
37:31
want to extend the block we want to extend a file but the next location
37:39
right next to the files is already occupied by the other files if I'll be
37:47
for example then there is no way for the painters file system to extend this fire
37:52
then we have to find free space and that we have to find a free space that can
37:59
get accommodate extended file and then we have to migrate entire file it will
38:08
consume huge amount of time for this copy so we're gonna implement a block
38:16
pointers in an inode there but there are variety of ways to represent a file
38:22
block that belongs to a file but now we are going to use UNIX like fine
38:29
structure and then we are going to implement higher column space for a file