url: https://youtu.be/mCAKZvZ1brs?si=wcu5zaZdeR6ICy6y

-----------------

0:00
hello this lab is about the file system details in the Pintas block device
0:07
refers to the storage like SSD or hardest drive and such and so forth and
0:14
block device consists of a block that's a set of blocks it's a linear array of
0:19
blocks and each block has its number we call it a logical block address and will
0:25
be a in some context each block refers to the 512 byte sector or in some
0:33
context it refers to the 4 club I'd far system block so you have to check
0:39
whether what LBA refers to in each occasion this is how you represent how
0:47
we represent a block device in Pintas operating system this is data structure
0:52
the name of the data structure is block and the most important attributes for
0:58
the block is the size probably the size the size of this in terms of sectors and
1:04
then Pintas allocates a 16 length 16 character lengths named as an attribute
1:12
for the block device then it there is a block device type and then there is
1:22
pointer to the device driver which contains a set of operations defined for
1:27
the specific block device and then Pintrest defines a read count and write
1:34
count for a block device which refers to the number of sectors read and sectors
1:39
written so this is the block device and then for marrying a file system is
1:45
process of writing some contents over each of these blocks this is laid out of
1:55
the Pintus file system this is so this
2:04
is block so a block 0 block 1 block 2 block 3 and block 4
2:10
and such and such so force in here in
2:15
Pintas each block is 512-byte so the
2:21
fire system block size is determined by the fire system you can format a fire
2:28
system with four kilobyte block or you can format the file system with 32
2:34
kilobyte block or even you can format a file system with five turbine block here
2:41
in printers they is five to a byte as a sector size as well as fire system block
2:47
size and this is a basic sample layout of the fire system in Pintas the first
2:55
block there is a node for the bitmap the
3:01
bitmap is a data structure which determines whether each of the block is being used or not and second block is
3:09
occupied by the inode for the root directory and then there comes a four
3:14
block each represent a bitmap for this
3:19
fire system partition and there's there with directory of course I'm going to
3:26
explain each of these data structures in detail as we go on and there is and so
3:33
here creating the inode for the bitmap and creating the inode for the root
3:39
directory and creating and initializing the bitstream and creating the rectory
3:44
this part and initializing and saving the initial value of this data structure
3:51
is called formatting of filesystem so
3:56
basically formatting of our system is from the clean block device you write
4:02
initial values for the essential data structure that is required to operate
4:07
the fire system let's look at a bitmap size here we locate four sectors for
4:17
blocks but each block responds a sector and each sector is 512 bytes so
4:23
we are allocating 2048 bytes for a bitmap and it corresponds to 16 384 bits
4:32
okay so in bitmap there are sixteen
4:38
thousand three hundred four eighty four bits and each bit represent whether a
4:45
given sector is free or being used so if you multiply the number of bits in the
4:52
bitmap by this exercise there are we gonna get the file system size so in to
4:58
us in this file system partition you can have you have eight megabytes so if you
5:07
have four sectors 4-bit a bitmap then
5:13
that can cover eight megabyte file system partition so if you double the
5:18
bitmap to eight then of course it's gonna offer 16 megabytes for system
5:23
partition so this is how it works then from now on I am going to explain
5:30
how each of these fields are occupied in initial identifier system boot stage so
5:39
from adding a file system Pintas create basically three steps first one create
5:44
and initialize each how each of these sectors in the file system partition is
5:51
being used of course most of the sectors free at the vs. temper fair system
5:56
portunity phase and then after creating a bitmap you you regard a bitmap as a
6:06
file so if bitmap is also fire so you curate and I know it for a bitmap also
6:12
and the basic data structure for the fire system is the root directory so you
6:18
have to also create a root directory of course after all you creating inode for a bitmap you have to write it to the
6:25
disk right the bitmap to the disk to synchronize the filesystem States to the desk safely hey there are
6:32
three steps create and initialize the bitmap first step in the second step create I
6:39
need a bitmap right it's data on the desk this is second step the third step is create I note of a root directory
6:46
this is thirst and step one step two and step three okay so in the course of
6:55
formatting of our system now you write from 0 to from sector 0 or from Luxor I
7:06
am I'm I'm using the sector and block interchangeably especially in the
7:11
painters file system because in penis file system each block response to sector so so formatting a file system in
7:20
Pintas corresponds to writing the first writing and initializing the first six
7:26
blocks in the fire system inode block for a bits map I need block four root
7:34
directory and initializing inode bits map and this part is tricky at the
7:47
beginning when the first agents first created the empty directory for the root
7:53
directory is created but it's not going to have any contents inside its
7:59
directory let's see how it happens okay
8:07
so this is the code for initializing the file system it is indeed function file
8:15
system see the name of the files name the function is physis underbar init
8:24
okay so it contains three basic three basic steps initializing the inode and
8:35
initializing the bitmap and perform
8:40
format okay so what does it corresponds to we're going to look at the details
8:46
first function I node in it is create and
8:51
initialize in the data structure for open files okay when the system is first
8:57
create the first Rita first boots up in in met there is do basic two basic word
9:07
in computer system first one is memory
9:13
the second one is desk so when the system starts which we call all system
9:21
boots you create an empty list that
9:29
points there is a list of open files in inode is the data structure that
9:35
represent a file and also in a free map
9:40
in it you have to create a bitmap of that can cover eight megabytes and you
9:49
have to initialize it properly and formatting the formatting is writing
9:57
this data structure to the disk properly this is and and also the formatting here
10:07
has two things first one is the create and write the bitmap file and the second
10:17
part of the formatting is create the root directory there's two important
10:23
things so remember initialize the inode this initialize the bitmap and then performed
10:32
format format yes the process of writing the bitmap to the disk and the process
10:39
of creating the root directory on the disk and creating the file consists of
10:45
two parts the first one creating the inode and second one creating the data
10:51
blocks associated inode so create the [Music]
10:56
okay let's move on so this is the process of initializing the fire system
11:02
we call it format okay so as we see B as
11:11
we saw before the first two important part is create the Mt inode list for
11:17
open files second thing is create the bitmap and then perform perform format format
11:26
consists of creating of sorry oops oops
11:32
okay creating the inode of bitmap file and save it to the desk for to save the
11:42
disk from the crash recovery and then create the root directory there are two
11:49
things okay let's let's look at the actual code so first thing we have to do
11:58
is initialize list of open I nodes it corresponds to initializing the list of
12:03
in memory I nodes so it I note init function it it course list underbar init
12:11
this is a typical function that initializes the empty inode list at the beginning and it is open I nodes so this
12:18
is an essential function to create a list empty list okay so these are in Pintas as you see
12:27
it mayn't it maintains a SIDS set of I nodes for the opener files as a linked
12:34
list my question is what is the data structure that were present open files
12:41
in a v6 operating system probably the Pintas and sv6 or more wide leaves the
12:48
operating system for operating system education they are using slightly
12:53
different data structure for representing each of the concepts in the operating system so we in the in the
13:02
class in the textbook they are using we are using explicit six code and in the
13:07
lab we are dealing with the Pinta Zoe if you are interested in different
13:15
ways of implementing the operating system concept you may want to read both
13:20
of the codes for comparison purpose so my question is what is the data structure that represent the open files
13:27
index physics please check it out so the
13:35
first step is creating and initializing the bitmap so first you create the
13:41
bitmap of block size so here the first
13:47
system is in megabytes as we saw before and then at the very beginning we need
13:55
to I note one inode for bitmap and the
14:00
other inode for a root directory so we have to mark those bitmap as being used
14:08
and we're going to allocate inode for the bitmap at sector zero and we're
14:15
gonna allocate the bit I note for the root directory at sector one so you have to mark the associated bits my associate
14:23
entering the bitmap so this is location we're gonna mark this is location of the
14:30
we gonna mark also for it root directory and this is the pointer to the array of
14:35
bitmaps so the next in the initial line
14:46
of our system there are three steps creating the inode of bitmap and of
14:51
course after you are you are done with initializing bit my beret you have to write it them write them to the disk for
14:58
synchronization purpose and then you have to create the root directory
15:06
writing the contents the pin up to the disk and create I know the root directory to the disk
15:13
okay so in in this function there are two steps create the freemen file and
15:22
then second one create the root directory so there are two things you have to the first one creating the
15:29
bitmap file and second one creating the root directory especially the root directory the second
15:36
parameter is number of entries in the directory number of entries in the
15:41
directory and here you are initializing the root directory with maximum of 16
15:48
entries and this is the location of the
15:55
inode where you are going to place the inode for the root directory the first
16:04
one the first step create create and save the bitmap file so you create the
16:10
inode and write it to the disk this is actual code for creating a bitmap
16:18
creating a bitmap so you have to first
16:23
create the inode for the bit map this is the first parameter is the location of
16:31
the inode where you want to create the inode for a given file okay then there
16:40
are two functions you want to pay attention to I know to create and file
16:46
open and bitmap right so after creating
16:52
the inode for a bitmap file you bring those I know to the memory and save the contents of the bitmap to the disk so
16:59
these are three basic function for creating a free free map next thing is
17:09
creating a root directory root directory corresponds to creating a root directory corresponds to creating an inode for the
17:17
root directory so creating an inode
17:23
at this location location of inode and
17:31
then the size of the size of the
17:37
directory entrance so again the inode create access to
17:45
parameters for joining sector this is block number which has the inode for the root directory an entry count is the
17:52
maximum number of entries in the root directory so you specify it as 16 the
17:58
code and the second thing is after
18:06
you're creating an inode of course every every directory need a place to save the
18:12
directory entry other than the inode so you have the allocated data blocks for the root directory and you have to save
18:20
the start address of the data block at the inode so there is two functions by
18:32
two sector is a simple function that translates the number of bytes into the number of sectors and then the Fremen
18:40
allocate is allocate or certain amount of data blocks from the bitmap and so
18:48
allocate contiguous blocks so for example you want to allocate for example
18:56
16 sectors then what you have to do is you have to scan the free bitmap and
19:04
looking for the consecutive bits array where all these bits arrays are free so
19:12
this is 16 bits every for example you find that this log is this bit is
19:18
occupied occupied occupied RQ is not occupied it's occupied of course this
19:25
bit is occupied but because the next bit is being used even though this 0 1 0 1 2
19:35
3 even though the third beat is free because the fourth beat is being used
19:40
you cannot allocate consecutive 16 bits so if you find the consecutive 16 bits
19:48
are free then you allocate consecutive 16 bits and corresponding blocks and
19:55
then returns the start address of the sec responding sectors that's what free
20:01
map allocated is used and then you have to save the starting address that be
20:08
associated I know that a starting point
20:13
then you write disk I note to the disk safely because one thing you have to
20:22
make sure is memory we call it as a DRAM
20:28
is volatile well means that if the powers goes off then you lose all the
20:34
contents storage it's usually either
20:40
desk is non-volatile if you save that
20:45
contents of the disk you you say you you can have it even though the power goes
20:51
out so but every operation every operating system operation deals with the memory data structure so you have to
20:59
be careful and if the data structure needs to be saved carefully and saved
21:04
safely even with the existence of power crash then you have to be make sure that
21:09
every data structure needs to be stored at the disk safely at the when the
21:16
operation completes so this you write the disk inode to the disk
21:27
okay and then after you're writing the bitmap contents to the disk then you
21:35
close inode and then after closing the inode UDI locate and remove the in memory nine notes from the up knighted
21:42
list and saved it to the disk the next
21:48
step to do is to load the bitmap to memory so instead read the bitmap contents on the disk from the disk to
21:55
memory so it consists to face first Sonia's open the file and then really
22:05
now we're going to explain the details of creating our file creating a file is
22:12
done by creating the file system mm-hmm fire says on the back create fire says
22:18
on the bar create this is the driver function of the system core create so by
22:25
looking at the this details of the code you're going to learn how the fire system that crew fire system create a
22:32
file so creating of our consists of creating the inode let's think about
22:39
what kind of data structure we have to modify in creating a file in the fire
22:45
system let's say we are going to create a file then we have to create the inode
22:53
for new file and then of course you have to you may want to initialize the data
23:00
blocks associated with the given file okay and then you have to modify the
23:11
directory parent directory which the newly created file belongs to for example let's say the name of the file
23:18
is ADA C then a that C contains a pair which is file name and inode number okay
23:27
so this is data so this is the new file
23:33
data block and The Associated inode then there is the director block is that
23:42
all actually there is one more thing so there is a bitmap the fire system so you
23:50
have we have four sectors we have allocated four sectors in the fire
23:57
system let's say this first sector second sector and the third sector and
24:04
because you have allocated two blocks one sector for inode in one sector for
24:11
data block in this specific example so you may have updated some part of this
24:19
bitmap let's say you have updated two bits in the third part okay then in the
24:29
course of creating a file you have updated at least four blocks the first
24:37
law and second block and third law and fourth block one two three and four so
24:48
in the course of creating a file we have updated and modified four blocks we have
24:54
to synchronize all these blocks to the desk so as you can see creating a fire
25:00
system creating a file is non-trivial exercise so the details of the fire
25:07
system is create and initialize then I know to write it to the desk and then
25:12
add new entry in the root directory okay this is the simplified steps of creating
25:18
a file in this example they did not allocate any data blocks and it does not
25:23
show how the bitmap is are located and updated but we are going to show how it
25:29
works so this is detail steps of creating at a file the name of the file
25:36
is test file so it we call our function
25:41
filesystem create test file okay so for
25:46
very first step it's a read the inode of the with directory from the desk to memory and insert it to the open inode list the
25:55
second step is write the inode of the test file to the disk the create inode
26:03
and then read the entries on the root directory and then add a new directory
26:12
entry for the test file and then write the interest of the root directory okay
26:21
so did consists of five steps then let me ask a question
26:27
among these files then the five steps it only shows the steps of modifying and
26:33
updating the inode block and directory block I know the block and directory
26:42
block directory blog is updated but I
26:52
know blog is newly allocated newly
26:57
allocated so in the course of allocating
27:03
a new I know new blood for inode we have to update bitmap we have to mark the
27:11
Associated we have to find a free block and mark the Associated bitmap as marked okay then which of the five steps three
27:21
four five we have to include where do we have to include or where do we have to
27:27
insert the process of updating the bitmap let's say this is 1.5 2.5 3.5 4.5
27:39
and 525 and 0.5 where among these possible six positions
27:47
where we have to include the process of finding the free entry in the bitmap
27:55
where think about it of course the answer is gonna go up and
28:00
sir that's show up shortly okay let's move on to the next page the first
28:07
iStent create consists of well there of
28:13
course there there comes an answer right after we first have to open the root directory and then the the first system
28:29
the creating file consists of creating an initialized inode and add new
28:35
directory entry to the root directory in the current printers filesystem here we
28:40
do not have hierarchical directory structure we only have root directory so
28:47
open root directory and if the directory
28:53
root directory is successfully and okay then we allocate free block and this is
29:07
number of sectors we want to allocate this is number of sectors number of
29:17
sectors you want to allocate and then in the course of allocating a new plot you
29:24
insert the start address of those sectors and be specified I know the
29:32
dress okay so free module locate allocate here specifically it locates
29:37
one sector as specified here from the free map and then save the start address
29:42
at the item sector this is how you create the sector so in the course of
29:49
calling free map I'll locate inside of this function it automatically updates
29:55
the free bitmap array and then of course
30:02
I not create the course of creating an inode it initialize an inode with
30:10
initial size byte and write it on the disk and then add
30:16
the creative directory entry - dir specified as a parent directory of the
30:23
product just created file ok so this is
30:30
how we creating a file now let me explain the procedure of opening up and
30:41
then I known for the first time so the function we function in concern is inode
30:47
open it read on the sky node etc so this
30:54
is first parameter so it reads the inode from the disk at the sector location and
31:00
then create its pointer of course the important operation of inode open is
31:08
inserting the read inode to the linked list of the OP nine out here so the
31:17
pintos maintains a list of inode list of open inode in the system is it is global
31:24
linked list and the course of opening and I note you have to insert those
31:31
design it to the list so this is the process of inserting and inode to the
31:37
open inode list okay and then you have
31:44
to set the field properly okay there is one important field you may want to pay
31:54
attention to there's this open count this is open count
32:03
more than one process can open a file and then if there are two processes
32:10
process 1 and process 2 and both of the
32:17
file are course a function open after C for example then the same file is opened
32:29
twice by different process in this case this open count is set to 2 so this
32:37
represents the number of process that opened a given file opening a directory
32:47
is similar to opening a file except that directory entries are opened instead of the inode itself so directory open the
32:54
function gets the inode pointer as its parameter and in opening a file in a
33:03
locate directory structure and read this country read the directory entry and
33:10
initialize the field of the directory structure with the inode and the
33:16
associated position then the important
33:25
function most essential function is allocating a free block this is function
33:33
free map allocate so it accepts 3 use
33:40
except two parameter sorry two parameters first one is count the
33:45
objective of free map allocate is find C and T consecutive road blocks scanning
33:51
free block so C and T stands for the number of blocks to allocate and as a
33:59
result the sect or our sector position sector P specify the start address of
34:06
the blocks allocated and the in the course of allocating a free block it
34:14
sets the free bitmap so my question at the Benigno of this few slides back my question of
34:21
slides back was where the free block free block bitmap is updated in the
34:27
course of luck allocating a new block it is with Infantryman allocate so find the
34:37
consecutive fours big map or false bit memory is a bit may be setting with 0
34:42
and set them to true the next function
34:51
is how we create a directory a few slides before we agree we look at the
34:58
function that create a file now we are looking at the function of how we create
35:05
a directory okay so it it has what we
35:13
would like to do is add a name file to the directory and I know the file then
35:21
needs to be inserted yes at I know sector parameter so it has three
35:26
parameter named directory in I know sector okay so directory it started target
35:36
directory where the newly created file is located in the name of the newly
35:42
created file then associated location of the inode for the newly created file the
35:50
first thing you have to do is it look up the directory and check whether a file
35:58
under the given name is does already exists or not if it exists then the
36:04
creation attempt should fail [Music]
36:10
and then it has to scan the directory
36:15
and directory block and find the empty spots okay this is let's say this is
36:21
directory block then first for example first entry is let's say a does see
36:28
second and dress B does see and third entries MT fourth entry is d dot C for
36:34
example then in the course of creating a directory entry it first have to scan
36:40
the entire directory blog and find the first empty spots and then create the blocks at this position what you you may
36:51
want to notice is scanning the directory
36:56
blog is very time consuming it's very expensive operation okay so if you if if
37:07
you write in an application if you write an application that frequently create a file then your file system should be
37:14
very very efficient in creating a file and every time when you create a file
37:19
you cannot linearly scan all the directory plus to find out empty spots
37:25
it is going to be very very expensive anyway you might want to have more you
37:34
might want to use more sophisticated data structure for creating a file especially in finding empty spots in the
37:40
directory block okay so this is how we
37:46
create TIF directory entry let me skip to the next slide now the director
37:51
lookup director lookup is it takes
37:56
directory pointer and it takes the name and it is a directory entry point and
38:03
upset so the lookup is a function and it
38:09
checks if the file name exists in the directory or not and then it returns the address of directory entry structure
38:16
using the parameter it has supplied okay
38:21
so there this is input for a given directory find the name and then return the
38:28
address of directory entry using the parameter supplied this is lookup the
38:37
next function we're going to cover is opening a file so it is it's called file
38:43
system on the by open and it gets the name so it's opening a file system and
38:51
it returns to the point on to the truck file so viruses open is a system core of
39:03
the fire cysts on dopa open is called by system core open and it does two things
39:14
first it as inode to the inode open inode list this it as an inode to open a
39:23
node list second it allocate an initialized struct file and return its address okay so the
39:34
first one you have to first check if the file has already been append five by the
39:41
other open system core in that case you do not have to insert the same inode to
39:49
the list twice instead you just increase the reference count of the inode entry
39:55
in the under list however you have to allocate an initialize struct fire
40:01
structure every time when a file is opened meaning that every time the fire
40:08
sis open is called so this is the
40:13
process of allocating and opening a file
40:19
this is the process of opening a file the name of the Phyllis let's provide an
40:24
example the name of the file is test file a farces open test file consists of
40:35
the following five steps first you have to read the inode of the root directory first and then you read
40:43
scanned entries in the root directory and find the lame test file okay you
40:51
scan the directory entries and you find test file and then you met you find the
41:02
matching inode and we find it that the inode number for testifier is 7 so you
41:10
add sir step you have to insert in memory I know test file to the inode list so you are done within searching
41:19
and inode to the inode list and then as a next structure you allocate a struct
41:28
file data structure every time you open
41:33
a file we have to look at a fire stroke data structure and you set the address
41:42
of in memory I know it and return its address of course it is possible that
41:51
the inode for test file is already exist in in-memory nodes in that case you
41:57
don't have to read it again or insert it again
42:03
this is system core for allocating or
42:11
opening a file ok the most important step of allocating enough most important
42:18
task of opening a file is allocate and initialize struct file and return its
42:24
address so you return the file open for
42:33
the inode so find the inode number for a given file and core file and about open
42:43
so this is essential function and it a liqu AIT's an initialized structure
42:49
file on memory so next function is
42:57
director look up each director look up is the easy function it looks up it open
43:04
a directory and looks up the file under the name name and return the inode so
43:14
that's what directory lookup is for once you find the associate I know it's time
43:21
to open a file so you get the inode you you supply the inode number to the
43:27
functions file and the bar open the role file of underware open is allocate the
43:34
data structure for struct file and then initialize it and then return its
43:39
address so allocate fire struct or
43:46
strands let's say struct file and initialize it you fill it with some
43:52
value and return the pointer with return
43:59
the start address of the file and what is the process of initializing the file structure initializing the fire system
44:06
and fire structures right here ok the fire structure has a pointer to its
44:13
inode okay then the most important attribute to fire struct is the offset
44:20
here we call it as a position the position stands for the offset for a
44:26
file where we apply the read and write operation so this is called offset and
44:33
in Pintas it is called pause position and when you'd blindly open a file you
44:41
have to set that deny right field as forms okay but if this fire is
44:49
executable or for for some reason you may want for for some special case you
44:54
may want to write this field as true okay so this is the important step of
45:00
any lysing initializing the fire struct struct file when I open the file so
45:07
creating the inode and initializing the position the desk but the opening of
45:14
what open a file is for let me provide
45:20
the steps of removing a file the system the matching system core is viruses and
45:28
the bar remove and this composition can remove the basically the base what it
45:35
does is to set the flag removed and inode to be true and then remove the
45:42
directory entry that's the two steps it needs to do so set the flag and remove
45:48
the directory entry this is a float so it opens the root inode and searches the
45:57
root directory for a given file and if it does not exist then if the unlock
46:04
case the root directory inode then return false means it's basically the
46:10
deletion failures if the directory the file exists in the directory then the
46:17
directory entry becomes force the in use
46:23
field of the directory entry will set the false and I know the puffle remote
46:31
will be set to start with true and then et allocating memory I know the root
46:37
directory and we control this is the example of removing the test file so we
46:45
call filesystem the Bourbon move we call test file ok then it read the inode of
46:53
the root directory and then it scans their entries of root
47:00
directory and find the Sajit file and the set in use as force and what I note
47:10
it says the removed field to be true and
47:19
let's look at the actual code so if there's two things first it remove the
47:27
target file entry from the directory and then it's set the removed flag from the
47:35
inode to be true so that's basically two things that have to do first it removes
47:42
the directory entry to be true that's that's the flag okay so the mixing the
47:51
function directory move it removed the entry of the target directory and then
47:57
it sets the removed flag in the in memory I know to the true and they write the updated directory entry on the desk
48:03
so this is the details of the director remove it gets two functions for Sony's target directory and a second is name
48:10
this is the name of the file it wants to be removed so first step is search the directory and get the
48:18
Associated directory entry and make it as false and then write back the updated
48:26
directory back to the desk so I not right add and then update the disk
48:36
contents of the associate director and after after removing the directory entry
48:43
it's time to remove them inode is self of the file the first thing you have to
48:48
do is it opens inode and then it called a function called inode remove but
48:57
calling einer remove does not mean that I know it is immediately deleted because
49:02
the inode might have been opened by the other process more than one process so in Idol open ad
49:10
in memory eyelid to the open eyelids list and then card I not remove in
49:16
eyelid remove the most important part is sets they remove the flag in the in
49:24
memory I know to be true okay that's it so it does not do anything other than
49:30
setting the remove the flag to the inode to be true nothing more than that and
49:38
then after that week or the function inode and about close in the course of
49:46
calling the inode on the back close it does all the dirty works so it checks
49:52
the reference count and it checks if they there are any other process that refers to design out so if all is
50:00
completely free then eat the unlock ace the Associated inode from the disk and
50:05
then make the associate sectors free that's the role of either close that
50:11
this is and removing a file so in this
50:17
chapter we have covered a few details of the fire system interfaces from any of
50:24
our system creating a file could be in a directory and look up a directory
50:30
opening a file and removing a file this is basic steps of delimiter fires in the
50:36
painters fire system so you're done
50:44
so at POS
