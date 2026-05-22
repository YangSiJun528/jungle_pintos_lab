url: https://youtu.be/AydN8n7PCaY?si=6c5_Lk4gVNU_LYk_

-----------------

0:00
in project number four we're gonna implement three things for the file system first one is buffer cache and then
0:06
second one is make the files index and extensible and implement the concept of
0:14
subdirectories the first one i'm going to explain is buffer cache
0:21
the idea of buffer cache is simple we used the part of memory
0:28
as the disk this is an opposite concept of virtual memory and swap space
0:36
and there is let's say there is this is memory and
0:43
this is disk virtual memory is the concept of using a
0:50
part of disk as some memory it's something like this
0:56
so um say this part is mapped
1:02
to memory and this part is mapped to the disk and this portion is chord
1:09
as swap space this is virtual memory in buffer cache
1:17
the concept is the opposite let's say this is buffer cache
1:24
we use part of the memory as the disk something like this
1:31
okay so use part of the memory as a disk that's the
1:38
big concept of buffer cache so part of the disk is mapped to the memory
1:50
that's perfect cache so buffer caches consecutive physical pages
1:57
and it is initialized when the system starts or when the file system is mounted so if
2:04
you unmount the file system and mount it again to all the contents in the buffer
2:09
cache is reset and so in current pintas there's no cache
2:15
for the disk io in reality but however in most operating system
2:21
have a buffer cache for the desk io so we're gonna change modified file system cad modify
2:27
the file system so that we can cache the file blocks into the physical memory
2:34
in this um in this project you are going to allocate 64
2:43
um file pages to be cached in the memory
2:50
okay this is why how how you are going to implement um
2:57
current pentas accesses storage on every read-write request
3:04
so um we have to change the user io to be performed through buffer cache and we
3:12
are going to add layer buffer cache layer here
3:23
this is before you implement the buffer cache and this is after you implement the buffer cache
3:30
so this is the list of things to do the first thing of course is define the data structure for buffer
3:36
cache and then you have to write the routine to allocate and initialize the buffer cache and then
3:43
you modify read and write routine so that they
3:48
utilize buffer cache in reading and writing the data and then of course you have to
3:56
change the changed the read write routine so that if cache miss occurs
4:05
if you want to read the data blocks from the buffer cache but if that block does not exist in the buffer cache then you
4:11
have to read it from the desk okay so you have to implement a routine
4:16
to handle cache mesh and there is another important routine you
4:21
have to write when you want to bring in the new block
4:28
to the buffer cache it is possible that buffer cache is already full and you have to make a room for incorp
4:35
and make a room for bring in bringing in the new blocks then um in that case you have to event
4:43
evict one of the buffer cache entry and make it free so this is
4:50
you have to implement buffer cache
4:56
replacement algorithm and then you have to write a routine to
5:04
write the dirty perfect cash it is possible that after a block is read from the disk to the buffer cache
5:10
the application might have changed it might have changed the contents of the buffer cache in that case you have to
5:16
write the dirty page back to where it read from so you have to write a routine that
5:22
synchronized dirty buffer cache to the disk this is the list of things to do
5:30
it's very simple and it is very fun all right the first thing you have to do
5:36
is define the data structure for the buffer cache define the data structure for the buffer
5:42
cache okay so buffer cache is
5:47
a page in memory okay and uh for
5:53
use a page entry as a buffer cache you have to define some kind of metadata structure that
6:00
represent the contents in this page so it has to contain a dirty flag
6:11
the reflect denotes whether a given contents has been changed since it was read from the desk
6:20
and this type of flag in the heading indicating that whether the entry being
6:27
used or not and of course there is
6:34
we have to introduce the access flag so it indicates whether the entry has been
6:41
accessed recently or not this access flag is used for the victim selection when the
6:49
operating system has to select one of the buffer cache entries to make a room for the new one
6:56
then there has to be mechanism to distinguish hot blocks from the cool blocks hot
7:03
blocks means the buffer cache entry that is being accessed actively
7:08
and cold buffer cache is called buffer cache entry is the one that has not been used recently
7:14
so we have to have some type of mechanism to precisely identify the hardness of
7:21
the block and this is to let the operating system to select the data blocks that has not been used
7:28
recently it is important that when you have to kick out one of the buffer cache entries
7:37
you have to choose the buffer cache that has not been used recently so that
7:45
the performance is not affected by kicking out those entries
7:53
and also it has to have on disk location and then of course it has to have a virtual address up there to associate
7:59
virtual cache entry so oh we are going to um
8:06
allocate 64 pages for the buffer cache so we need let's name the
8:13
data structure as buffer head buffer head data structure so we have to
8:21
allocate 64 buffer head when the system starts there are many
8:29
data structures that can accommodate 64 data structures we can use array list or hash table
8:40
here we are using the table so this is the table of 64 buffer heads
8:46
and each buffer head has a pointer to actually its location in the virtual
8:52
memory so this is diagram for the buffer cache um so um the act of reading
9:03
this is buffer cache so act of reading a disk block from the disk to
9:09
buffer cache is cord caching this is caching
9:17
and when some of the back buffer cache is updated after it has been read from the
9:24
desk then you have to synchronize the contents of the buffer cache back to the place where it belongs
9:31
that is cord flush
9:36
okay this is entire diagram for the buffer cache data structure and this is the one you have to implement
9:44
and um when when an application program wants to read
9:50
a certain block then it has to search the buffer cache table first to identify the location of
9:57
the buffer cache
10:03
okay all right so uh the next step is so um i pretty much have explained
10:09
everything for you to be able to define the data structure to for the buffer head so you should not
10:15
have any difficulty in implementing the buffer head data structure
10:28
and the second issue is allocating and initializing the buffer
10:35
cache of course this has to be done at the system boot time and fire stem is mounted
10:42
so um we are going to allocate we we need a space for 64
10:50
fire system blocks the block size is 512 byte so we need 32 kilobytes to accommodate
10:58
64 file blocks and of course also we have to allocate a memory for
11:04
buffer heads sixty four above ads so this is the size of buffer head times
11:11
64. this is the amount of memory you have to allocate for defining 64 buffered entries
    11:19
    and this is done when the file system initialization time
    11:25
    so you have that code here that's pretty much easy
    11:34
    okay that's easy okay so um now let me explain the read
    11:42
    and write in current pentas um this is the function file
    11:47
    read and it calls a function called inode on the read and about it
    11:54
    okay um read of course the read um read system core
    12:03
    takes two parameters for of course versus five descriptor and the amount of data blocks to read
    12:12
    and then the operating system reads a certain amount of data blocks from the
    12:18
    disk to the memory
    12:24
    okay so um in current printers this is how the read
    12:31
    is implemented um there is a variable called size okay every time it reads from the
    12:39
    disk it reads 512 byte so it iterates this loop
    12:46
    reads and reads and reads until it reads amount of data that has been specified
    12:53
    as a parameter to the file under the read operation so each time the size decreases
    13:01
    by 5 turbines so eventually the size becomes zero or
    13:09
    negative um okay so read the full sector
    13:16
    and then block width from the disk and compute the remaining ridge size
    13:22
    if the size of the read is still greater than 0 then it reads the full sector again
    13:27
    but it is possible that if you want to read like for example um
    13:34
    100 and 100 and sorry 1500 byte
    13:41
    okay then it reads 5 12 byte first and then it reads
    13:49
    five 12 bytes again five 12 byte sorry then
    13:55
    it sums up to 1024 byte
    14:01
    okay so um the remaining part um
    14:08
    six seven four uh four seven six byte
    14:15
    needs to be read from the disk but it is not a single block right it is not a single
    14:22
    block so you should not call a block read from the desk
    14:33
    and then so you perform you allocate a bounce buffer and then perform
    14:41
    partial read the partial read means um even though you read
    14:47
    entire blog from the desk which is five to a byte
    14:54
    the application has asked you to read only four seven six first 76 bytes from
    15:01
    the disk so read five tel byte to the bounce buffer
    15:12
    and then copy only 576 bytes 476 bytes from the bonds for fur
    15:19
    to the buffer designated by the read this is 476 byte
    15:26
    this is how you perform partial read okay so now
    15:34
    um you have to implement this part of course the the entire diagram looks
    15:40
    a lot more complicated but don't be afraid it is easy and you
    15:46
    should be able to do it very quickly so um after calling the function i node under by
    15:53
    read on the it okay instead of reading directly from the disk you read the
    16:00
    buffer cache so you read perform read in the buffer cache first thing you have to do is find
    16:05
    the buffer head if it does exist then
    16:11
    read data from the buffer cache to the buffer and then update beforehand here what is
    16:17
    updated is probably the access bit access flag denoting that this buffer
    16:24
    cache just uh has just been accessed okay that's lucky part of the story
    16:34
    but life does not go always as easy as you want sorry oops
    16:40
    okay so um the first issue is uh if what if the entry does not
    16:46
    exist then you have to bring in you have to read the disk block from the
    16:53
    disk uh to the buffer cache and perform the read again
    16:58
    okay so um in that case if the cache
    17:05
    if the entry does not exist then you have to bring in the disk clock from
    17:11
    the desk to the buffer cache if the cache is not full then that's
    17:16
    better then you can just read the blocks from the disk to the buffer cache and read it again so select empty entry from the buffer
    17:24
    head bring in the disk blocks and then perform read that's good but the worst part is
    17:34
    the case when the buffer cache is entirely full
    17:44
    okay then
    17:51
    um if the cache is full then it's select victim entry
    17:59
    has to select the victim to kick out or and to make a room for uh bringing the new one bring
    18:06
    the disc blocks so select the victim entry
    18:11
    and if the victim entry has been modified since it has been brought up brought
    18:19
    into the memory from the disk then we say it's dirty then we have to save it to the disk we have to flush
    18:25
    victim entry to the disk and after after
    18:32
    after flushing the disk flashing that buffer cache enter to the disk it has to release the victim entry and
    18:39
    make it free okay if um
    18:45
    the victim page is not big sorry the victim entry is not dirty then it does not have to perform flush it
    18:52
    goes directly to releasing the victim entry and then again because the now the free
    18:58
    block is available in the buffer cache entry it can perform operating system can perform read
    19:03
    from the address to the memory
    19:18
    okay so we have to modify the disk read
    19:24
    to buffer cache read the function we have to change is inode read at
    19:31
    so we have to modify the block read
    19:37
    so when reading a file we modify the read to read
    19:43
    the data from the buffer cache not from the disk okay so you have to modify this
    19:51
    right part so this is the the loop okay
    19:59
    um the right has to be changed in similar fashion so um right is the same it
    20:07
    um it iterates in the while loop if you want to
    20:14
    write end and byte then each time it iterates pinterest
    20:21
    operating system writes one block at a time so each time it performs right
    20:28
    it decreases this size by five terabyte okay
    20:36
    and then calculate the remaining right size do it again and do it again and do it again um if
    20:42
    the size is becomes zero okay then the write is done so
    20:50
    it releases the bounce buffer but if the remain in case the remaining amount of data to
    20:57
    write is less than a sector okay this is this is important and tricky part we
    21:04
    call it a partial right
    21:12
    um actually partial right takes more time than
    21:19
    a blind write we call it a blind write um the blind write means the right full sector so you don't have to
    21:25
    read the disk plug from the from the disk but in partial when you write entire
    21:32
    sector you can just write it to the disk
    21:39
    but if you write only part of if you want to write full blocks
    21:46
    then just read it to this write it to the disk sorry if you want to write the full block then
    21:51
    write it to the desk but if you want to write part of the
    21:56
    blocks then what you have to do is you have to first
    22:02
    read the contents from the disk to memory
    22:09
    from the disk to memory
    22:14
    then fill the memory part with the data blocks you
    22:22
    want to write then write it to the disk so
    22:29
    partial write is more expensive so i'll locate the bounce buffer
    22:34
    you have to read the data blocks from the disk to the bounds buffer and then perform partial write to the
    22:40
    bounce buffer and then write the balance buffer to the disk so partial right is more expensive
    22:49
    um writing with buffer cache is similar
    22:54
    so you have to write in the buffer cache so find the buffer head if those blocks
    23:01
    already exist in the buffer cache if there exists then you are lucky then
    23:07
    update the buffer cache and you're done however if entry does not exist then you
    23:14
    have to write that to the buffer cache and then in that case
    23:21
    if the cache is not full you're lucky you select the empty you find the empty buffer head and then
    23:28
    write the block to the disk not to the disk to the buffer cache to the buffer cache and returns
    23:44
    okay um there is interesting part
    23:49
    [Music] block read data from the to the disk to
    23:55
    the buffer cache here this part
    24:02
    read read here block read here is
    24:08
    required only for the partial right
    24:17
    um if you perform full black right or if you perform blind right blind right you do not have
    24:33
    to read the display from the buffer cache okay if the cache is full if the cache
    24:40
    is full then you have to select the victim entry and if it's dirty then you have to flush
    24:45
    the victim enter to the desk this is the same as before and then releases this block
    24:51
    and if it's dirty it's not if not dirty then release victim enter from the buffer hat
    24:57
    so um [Music] in modifying the rised
    25:05
    right you have to modify disk right to write to the buffer cache right to
    25:11
    the buffer cache okay when writing your file modify it to write the date above case right then to
    25:16
    the disk this is what you're supposed to do and you are supposed to modify this part
    25:23
    block right okay so that's gonna be easy
    25:30
    so you're done
    25:36
    good all right
    25:43
    and um one of the important things you that remains you
    25:50
    have to do is synchronize the dirty buffer cache entry um when you releases a buffer cache entry
    25:58
    to account accommodate a newly incoming disk block you have to check whether the
    26:05
    existing buffer cache contents is dirty or not if the existing buffer cache entry is
    26:10
    dirty then you have to write it to the desk we call it a synchronized we call it a
    26:16
    synchronized synchronization activity synchronize
    26:22
    so uh um write uh 30 pro for cash entries uh
    26:28
    this happens when the buffer cache entry is evicted first or if the file system
    26:35
    is unmounted the shutdown means unmounted
    26:44
    so there is function called virus is done then you have to write a code here okay that's going to be easy so you
    26:50
    should be able to do that or um
    26:56
    there is another another situation you want to write the dirty bifurcation entry to the desk is periodically
    27:04
    so um um for example in five five seconds
    27:15
    five seconds interval and in this case you use time interrupt
    27:23
    okay so um let me explain the details of read write in pentos um
    27:30
    reading a file yes uh performed by file on the read function it contains three
    27:36
    parameters this is the pointer to the file object and this is the pointer to the memory
    27:43
    area which you want to read the data blocks into and the amount of data blocks you want to read this is
    27:50
    size data structure struct file contains a pointer to the
    27:56
    in core or in memory inode okay and in memory inode pintos maintains the
    28:02
    linked list of inode that has been opened by the operating system so i know the list
    28:09
    here there's link list here and then each inode takes this form and in in memory inode it contains
    28:17
    the structure inode and inode under the disk it contains a start address
    28:27
    and size of the file so looking at inode disk you should be able to find the location
    28:33
    of the data blocks
    28:39
    okay um the second one is write in current uh is pinterest
    28:47
    so let me go back so um it performs if it performs right um
    28:54
    yeah this is the data blocks we have covered in the previous slide so let me skip this right and then
    29:02
    um file right we perform we get three parameters first
    29:09
    parameter is file data structure and buffer and the size
    29:16
    okay and and in file right [Music]
    29:24
    it uh gets it calls the function i node on the right and about it
    29:31
    and it gets three parameters first parameter is inode second parameter is the address of the buffer and then
    29:37
    this is the in the fourth parameter is size and the fourth parameter is the current
    29:42
    offset okay so uh it passes the the position uh from which
    29:50
    you have to write the file to and finally you have write the database too and right after
    29:56
    writing some contents you have to update the file file position
    30:01
    then you return the amount of data that has been written to the file
    30:10
    okay this is the details of the important function inode right at okay it takes four parameters
    30:18
    inode buffer and size and offset so here
    30:26
    this is file and this is offset
    30:31
    and in inode write add you write size amount of data
    30:41
    and this is buffer
    30:46
    so you write this is size
    30:53
    so you write this here this is how i node right at works
    31:00
    let's look at the details of the right function let's see that um
    31:08
    this is the amount of this is a file and you want to write
    31:16
    from this position to this amount
    31:22
    okay and let's say this is three and
    31:28
    [Music] four blocks um let's say
    31:36
    this black one block two block three block four and block five but in block five you
    31:43
    only write half of the block okay this is file
    31:50
    this is beginning of the file this is off offset this is start address of the file
    31:58
    then when you write a file the first thing you have to do is you have to change the offset to the actual location
    32:07
    on the disk okay so this is the function which translates
    32:16
    the offset into the real sect address
    32:22
    okay and then you have to check whether the location you want to
    32:28
    write the data block is aligned with the sector address so
    32:33
    you have to make sure that in writing to the disk all the disk
    32:39
    blocks should start with the starting of the sec address
    32:45
    so if the sector offset is zero and the size of the chunk you want to write is
    32:51
    the sector size then you're happy and then you can perform block write
    32:56
    okay so let's assume that you want to write the
    33:01
    data blocks but data block starts with the sector address and then
    33:07
    you should write you should perform the full block right repeatedly until the size is greater
    33:13
    than zero but at some point at some point um if the chunk size
    33:18
    is less than block size then you have to perform the partial right
    33:24
    you perform partial write which means that you have to when you are writing part of the block
    33:30
    uh you need a bounce buffer and you first have to read the data blocks from the disk
    33:35
    to the bounce buffer then you have to read the bonds with that this block from the disk to
    33:41
    the memory and then you merge the contents of the bonds buffer with the blocks
    33:47
    fractional blocks you want to write so there's disk and then
    33:55
    you want to write part of the block this is the contents you want to write
    34:02
    blocks and then you first have this is bounce buffalo bounce buffer
    34:08
    and then this is the block you want to write
    34:13
    okay then you read the data box from the disk to the bounce buffer
    34:21
    and then copy this portion to the bounce waffle updating part of
    34:28
    the contents in the bounce buffer this is first step and this is second
    34:34
    step and writing the entire contents to the box buffer uh writing the
    34:40
    contents of bonds buffer to the disk this is how you update the disk block
    34:45
    that's easy so you're done again we are happy
    34:53
    we are happy hackers happy hackers
    35:01
    happy packers good well let me put some more hairs
    35:12
    all right so um this is um the function called
    35:18
    biased to sector
    35:25
    it gets the inode and it gets the position
    35:30
    and then it returns the sector address of the given position so this is very
    35:36
    simple inode has a start address of the file
    35:45
    okay and then we have a position so um if the position is here
    35:52
    and then you divide it by the sector size so position here is it's a position
    36:02
    within a file file of course the position is byte
    36:10
    okay so uh we divided by the sector it means that how many sectors does it
    36:16
    apart from the beginning of the file and then you add it to the the start sector number of the
    36:24
    file and then you will get the sector address of the blocks you want to write
    36:30
    that's easy okay so next next
    36:39
    we're going to explain the read this is the read of the current pentas it reads
    36:46
    from the disk and if it's partial read then i'll locate bounce
    36:52
    buffer and read part of that
    37:02
    okay so this is the same code this is the code for the file on the read it gets three parameters first one
    37:10
    is pointer to the file this is pointer to the memory chunk and
    37:15
    this memory chunk is the place where the operating system reads the data blocks from the disk to this buffer and then
    37:21
    the amount of data blocks you want to read this is size so it contains three problems
    37:26
    file buffer and size
    37:31
    [Music] okay so um this is file
    37:40
    okay and then it contains pointer to dino data structure
    37:46
    after reading a file the file position is updated
    37:55
    this is the last slide of the explanation on the buffer cache the function is inode and the bar read
    38:02
    and the at it reads the given file so it takes four
    38:07
    parameters inode buffer size and offset
    38:13
    so the inode represent the file you want to read and the buffer is the start of the
    38:19
    memory address you want to read the file to and this is amount of data plus you want to read
    38:25
    and this is the position of the file you will start reading so um similar to
    38:33
    the inode write um it iterates and for each iteration
    38:40
    it reads a block and each time he performs read it performs a
    38:47
    read on the block device and then each time you provide
    38:53
    a sector index so this is the sector you want to read so after reading the full block you will
    38:59
    perform the part of it at the end at the par at the end of inode read at so this is the end of the
    39:06
    explanation and i hope you enjoy the project good luck
    39:18
    and we are happy hackers
