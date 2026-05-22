# [Week05] Pintos Project4-4 Subdirectories

Source: https://youtu.be/yQqNrIiB2hU?si=D7G5jkCkcnrFNj0-

## Reserved Directory Entries

second directory entry는 itself의 inode를 point합니다. 좋습니다. 그래서 file system에서 subdirectory feature를 implement할 때, 각 directory entry에서 two directory entries를 reserve해야 합니다. 각 directory마다 two directory entries를 reserve합니다.

첫 번째는 current directory이고, 두 번째는 parent directory입니다. 이 concept는 바로 다음 slide에서 다루겠습니다.

## Directory Structure

다음 slide로 가겠습니다. 이것들은 implement해야 할 것들의 list입니다.

첫 번째는 hierarchical directory structure를 implement해야 합니다. directory entry가 regular file뿐만 아니라 directory file도 point할 수 있게 만듭니다. 그래서 Pintos에는 두 type의 file이 있을 것입니다. regular file과 directory file입니다.

좋습니다. 이것은 매우 important part입니다. directory entry가 regular file뿐만 아니라 directory file도 point하게 만드는 것입니다. 이것을 가능하게 만드는 details는 무엇일까요? 이것은 나중에 설명하겠습니다.

두 번째로 중요한 것은 dot과 dot dot이 있는 directory entries를 add하는 것입니다. 여러분은 모두 dot의 notion에 익숙할 것이라고 생각합니다. 예를 들어 file list `.` and `*.c`를 perform하면, current directory 아래에서 extension이 C인 모든 files를 list한다는 뜻입니다.

두 번째는 dot dot입니다. 여러분 모두 dot dot이 parent directory를 의미한다는 것을 알고 있다고 생각합니다. 예를 들어 `cd ..`, 즉 directory를 dot dot으로 change하면 current directory의 notion을 its parent directory로 change한다는 뜻입니다.

subdirectory를 implement할 때는 every individual directory마다 current directory와 parent directory를 위한 first two directory entries를 reserve해야 합니다.

그 다음 당연히 directory를 위한 data block이 있고, 그것은 number of entries를 contain합니다. 그러면 먼저 first entry를 reserve해야 합니다. first entry의 name은 current directory이고, 이것은 current directory의 inode를 point합니다.

second entry의 name은 dot dot입니다. 이것은 two bytes이고, parent directory의 inode를 point합니다.

그래서 new directory를 create하면 directory는 entirely empty가 아닙니다. 두 entries를 contain합니다. 첫 번째는 current directory의 inode이고, 두 번째는 parent directory의 inode입니다. 물론 두 개의 position을 switch할 수는 있지만 code 전체에서 consistent해야 합니다.

current directory, current directory의 notion은 process에 대해 global입니다. 그래서 `struct proc` 또는 thread structure에 current directory가 무엇인지 represent하는 field를 add해야 합니다. 물론 이것은 current directory를 위한 inode를 가리키는 pointer가 될 것입니다.

좋습니다. 그 다음 directory-related functions를 modify해야 합니다. file system create, file opening, file deleting입니다. 또한 directory와 associated된 new system calls를 create해야 합니다.

## Path

좋습니다. 이제 가장 important part는 path의 concept가 있다는 것입니다. subdirectory를 enabling함으로써 이제 path라는 concept를 가지게 됩니다.

모든 file이 same directory에 있거나 모든 file이 flat directory에 있으면 path라는 notion이 있을 필요가 없습니다. 하지만 이제 예를 들어 `/user/pintos/source/util/.../s.c` 같은 것이 있습니다. 이것들은 모두 paths입니다.

path는 directories의 sequence이고, 각각은 parent-child relationship을 가집니다. 그래서 이제 path라는 concept가 있습니다. 이것들이 모두 paths입니다.

다음으로 path에는 two types가 있습니다. 첫 번째는 absolute path이고, 다른 하나는 relative path입니다.

absolute path와 relative path를 distinguish하는 방법은 path의 start, path의 starting character입니다. absolute path에서는 root directory에서 starting하는 entire path를 specify합니다. 그래서 absolute path에서는 path string이 root로 start합니다. `/user/src/pintos/blah blah blah`처럼 path string이 root로 start합니다. 그런 경우 이것을 absolute path라고 부릅니다.

relative path는 dot 또는 dot dot으로 start하는 것입니다. 예를 들어 `../src/util/something/something`입니다. 그러면 이 path의 first directory는 제가 어디에 있든, current directory가 어디든 parent directory입니다. 이 path는 parent directory에서 start합니다. 그래서 location은 always current position 또는 current directory에 relative합니다. 이런 type의 path specification은 relative directory 또는 relative path입니다.

## File Types

좋습니다. 이것들이 해야 할 일입니다. one부터 five까지 five things가 있습니다.

첫 번째는, 이제 two types of file이 있다는 것입니다. ordinary file 또는 regular file이 있고, directory file이 있습니다. 그래서 inode도 two types가 있습니다. regular file을 위한 regular inode, 그리고 directory를 위한 inode입니다.

정말로 둘을 distinguish해야 할까요? 네, 당연합니다.

regular file data block에서 regular file의 data는 bytes의 sequence입니다. 그래서 current offset이 있고, regular file에서는 read와 write function으로 이 current offset을 move합니다.

directory file의 경우 directory file은 special합니다. 이것은 단순한 bytes의 sequence가 아닙니다. directory entries의 sequence, array입니다. 그래서 certain specified bytes를 reading and writing하는 대신 한 번에 directory entry 하나를 read합니다.

regular file과 directory files에 access하는 방식, read하는 방식, write하는 방식이 다릅니다. 그래서 file이 어떤 type인지 specify해야 합니다.

inode가 regular file인지 directory file인지를 indicate하는 flag를 add할 것입니다. 두 가지만 differentiate하면 되므로 one bit가 필요합니다. one bit면 enough입니다. 하지만 우리 data structure에 그것을 save할 방법이 없으므로, 결국 available한 smallest data structure, 예를 들어 single character를 allocate하게 될 수 있습니다. 이것은 one byte입니다.

data structure `struct thread`에서는 current directory의 notion을 define해야 합니다. 이것은 current directory의 inode를 가리키는 pointer입니다.

new file을 creating하거나 opening하거나 removing할 때, 각각의 create, open, delete가 directory뿐만 아니라 regular file도 handle할 수 있도록 modify해야 합니다.

directory manipulation을 위해서는 directory manipulation을 위한 system calls를 add해야 합니다. 예를 들어 change directory를 위한 system calls, directory를 creating하기 위한 system call, directory를 removing하기 위한 system call 같은 것들을 implement해야 합니다.

그리고 dot과 dot dot이라고 called되는 special directory entries를 add해야 합니다.

## Inode Structure

좋습니다. journey를 시작합시다.

첫 번째로, on-disk의 inode structure를 modify해야 합니다. 이것이 file인지 indicating하는 flag를 add해야 합니다. 죄송합니다, 당연히 file입니다. directory도 file이기 때문입니다. 이것이 regular file인지 directory file인지를 indicate합니다.

이제 regular file flag 또는 directory flag를 add할 것입니다. file이 created될 때 그것이 regular file인지 directory file인지에 따라 이 flag를 properly set해야 합니다.

## Data Structure Design

다음 topic으로 넘어가기 전에, 가장 essential한 것, essential task를 기억하세요. data structure를 designing하거나 modifying하는 것입니다. 그것이 software를 writing할 때 가장 essential한 part입니다.

먼저 data structure를 define하거나 design하거나 modify해야 합니다. 이것이 object이고, 그것이 어떤 kind of fields를 가져야 하는지 define해야 합니다. 그 다음 code 또는 method를 define하거나 write합니다. function으로 사용할 때는 method라고 말할 수 있고, procedure라는 term을 사용할 수도 있고, operation이라는 term을 사용할 수도 있습니다.

먼저 object definition을 very clearly define하고, 그 다음 associated operation, procedure, function, method, whatever를 define합니다.

현재 first or second step에서는 directory의 concept를 accommodate하기 위해 data structure를 redefine하거나 modify합니다.

first step에서는 on-disk inode를 위한 data structure를 modify합니다. 그 다음 directory의 concept를 accommodate하기 위해 thread를 위한 data structure를 modify합니다. second part에서는 thread의 concept 안에 current directory의 notion을 adding합니다.

물론 modification은 매우 simple and straightforward합니다. first step에서는 current file이 directory인지 regular file인지를 represent하기 위해 on-disk inode structure에 flag를 add할 뿐입니다. `struct thread`에는 current directory가 무엇인지 denote하기 위해 single field를 simply add합니다.

하지만 대부분의 other software design에서처럼, data structure를 modifying하는 것은, modification이 simple하고 insignificant해 보여도, design의 나머지에 profound implications를 가질 수 있습니다.

## Current Directory Inheritance

그래서 thread가 first time created될 때, every thread는 parent를 가집니다. thread는 parent에 의해 created됩니다. 그래서 thread가 created될 때 parent로부터 current directory를 inherit합니다. parent로부터 current directory를 inherit합니다.

thread가 first time created될 때 child process 또는 child thread의 current directory는 parent thread의 current directory와 same입니다.

물론 parent가 없는 thread가 하나 있습니다. parent thread가 없는 thread는 무엇일까요? 이것은 beginning에서 world creation의 Genesis 같은 것입니다. parent thread가 없는 thread는 무엇일까요? 이것은 init process라고 called됩니다.

computer가 처음 enabled되고 electricity가 engaged되면, motherboard가 initialized되고, power가 모든 hardware에 engaged되고, fan이 running하기 시작하며, cooling fans 등에서 mechanical sounds가 들립니다. 그 다음 hard disk가 initialized되고, CPU가 initialized되고, DRAM이 initialized되고, software가 hard disk drive에서 memory로 loaded됩니다. operating system이 run하기 시작하고, 그 다음 very first thread, init process라고 called되는 것이 created됩니다.

그때 init process는 parent thread를 가지지 않습니다. 재미있게도 by itself created됩니다.

그렇다면 Pintos에서 first thread의 process ID는 무엇일까요? code를 보고 재미로 찾아보세요. system이 booted될 때 Pintos의 first thread의 process ID는 무엇일까요? 직접 찾아서 Piazza에 write해 보세요.

다음 topic으로 넘어가겠습니다.

subdirectory concept를 accommodate하기 위해 on-disk structure를 modify했고, thread structure를 slightly changed해서 thread에 current directory pointer를 add했습니다.

## Algorithm of File Creation

이제 algorithm을 modify할 차례입니다.

file을 create할 때 먼저 path를 parse해야 합니다. path를 parse해야 하고, target directory에 appropriate directory entry를 create해야 하며, inode도 create해야 합니다.

첫 번째로 이것이 absolute path인지 relative path인지 distinguish해야 하고, file을 create해야 하는 target directory를 find한 다음 file을 create해야 합니다.

이것들은 file을 creating하는 detailed steps입니다. 물론 Pintos에는 이미 file을 create하는 module이 있습니다. 다만 root directory에 file을 create할 뿐입니다. 그래서 file creation을 changing할 때 해야 할 일은 file을 create해야 할 right directory를 find하고, root directory에 file을 creating하는 대신 path 위에 file을 create하는 것입니다.

그래서 이것이 modify해야 할 부분입니다. path를 examine하고 path의 directory를 open하고, new inode를 allocate하고 newly created file로 inode를 initialize하고, target directory에 directory entry를 add합니다.

좋습니다. 하지만 operating system은 bulletproof해야 하고 any failure or exception에 against해서 written되어야 한다는 점이 중요합니다. file을 creating하는 course에서 disk space가 없다면 어떻게 될까요? new inode를 successfully allocate하고 file을 위한 new inode를 create했지만, 이 모든 것을 creating한 뒤 directory가 full이라는 것을 find했다면 어떻게 될까요? 그러면 new file creation은 fail해야 합니다.

그런 경우 방금 modified한 모든 inodes와 bitmaps를 deallocate해야 합니다. 그래서 succeed하면 directory를 close하고 success를 return합니다. fail하면 inode를 deallocate하고 `success`를 return해야 합니다. `success`는 variable `success`의 value를 return한다는 뜻입니다. 이것은 variable의 name입니다. file을 creating하는 데 succeeded했다는 뜻은 아닙니다. file creation에 failed했을 수도 있습니다.

그래서 file creation의 algorithm을 modify해야 합니다. 물론 file creation의 algorithm을 modify하면 file deletion의 algorithm도 modify하게 될 것입니다.

## Creating a File

이것들은 file을 creating하는 details입니다. 이것은 file의 name이고, Pintos에서는 initial file의 size를 specify합니다.

reality와 Pintos에서는 항상 root directory에 file을 create해야 합니다. 하지만 modification 이후에는 먼저 path를 parse한 다음 그 directory에 file을 create해야 합니다.

물론 앞서 언급했듯이 absolute path와 relative path를 distinguish하고 parse해야 합니다. file을 creating할 때 flag `is_directory`를 set하는 code를 add해야 합니다. 이것은 current file이 directory인지 아닌지를 denote하기 위해 add했을 수 있는 flag의 name입니다. regular file이면 flag를 zero로 set합니다.

물론 directory path에 new directory entry를 add해야 합니다.

## Opening a File

또한 file을 opening하는 것도 modify해야 합니다.

file을 open할 때 현재는 항상 root directory에서 file을 find합니다. 하지만 이제 subdirectory가 있고 path의 notion이 있으므로 path를 parse하고, 그 directory에서 file을 find한 다음 open해야 합니다.

file 자체를 opening하는 것은 before처럼 file을 opening하는 것과 같습니다. 하지만 added step이 있습니다. path를 parse하고 그 directory에서 file을 find하는 것입니다.

path가 absolute이면 root directory에서 find합니다. root directory에서 find합니다. path가 relative이면 current directory에서 find합니다. 좋습니다. 이것이 file을 opening하는 것입니다.

## Removing Files

또 다른 step은 file을 removing하는 것입니다. before와 같습니다.

original Pintos에서는 항상 root directory에서 file을 remove했습니다. 이제 modification 이후에는 path에 의해 specified된 directory에서 file을 remove합니다.

target file의 inode가 regular file을 위한 것이라면 그냥 delete할 수 있습니다. 하지만 directory라면 directory가 empty인지 아닌지를 check해야 합니다. 여기서는 directory가 empty일 때만 directory file을 remove합니다.

directory files를 manipulating하기 위한 number of system calls를 add해야 합니다. 이것은 change directory, directory creating, directory reading, current file이 directory인지 checking, 그리고 associated inode의 number를 returning하는 것입니다.

첫 번째는 directory를 changing하는 것입니다. path를 받고, path를 parse하고, process의 current working directory를 function의 parameter로 specified된 target directory로 change하고, successful이면 true를 return하고 failure이면 false를 return합니다.

두 번째는 directory를 making하는 것입니다. 죄송합니다, parenthesis가 missing되어 있습니다. 이것은 `dir`이라는 이름의 directory를 create하고, successful이면 true를, failure이면 false를 return합니다.

## `readdir`

directory를 read하는 것은 file descriptor와 name을 accept합니다. `readdir`의 concept를 설명하겠습니다. 약간의 elaboration이 필요합니다.

regular file을 reading할 때 우리는 `read` function을 사용하고 buffer와 size라는 parameters를 pass합니다. file을 reading할 때 이 file은 descriptor `fd`로 represented되고, current offset이 있습니다. system call read가 called되면 operating system, 더 구체적으로 file system은 current offset position에서 starting해서 size amount of data를 read합니다. 이것이 read입니다.

`readdir`은 similar하지만 different합니다.

`readdir`에서는 당연히 directory가 `fd`에 의해 pointed됩니다. directory file에서는 directory file이 directory entries로 partitioned됩니다. directory file도 read할 current directory entry를 point하는 pointer를 가집니다.

`readdir`을 issue하면 이 function은 single directory entry를 read하고, 이 directory entry 안의 file name을 `name`으로 specified된 buffer에 copy하며, current offset을 next directory entry로 move합니다. 이것이 `readdir`의 목적입니다.

그래서 regular file과 directory file을 distinguish하기 위해 flag를 specify하는 것입니다. application이 read system call을 issue할 때, application은 given file descriptor가 directory와 associated되어 있는지 아닌지 알지 못합니다.

application이 regular file에 대해 read를 issue하면 file system은 그것을 pass하고 regular read를 call합니다. application이 regular file에서 read를 call하면 read를 call합니다. 하지만 application이 directory file에서 read를 issue하면 file system `readdir`을 call합니다.

하지만 current directory dot과 parent directory dot dot은 `readdir`에 의해 returned되어서는 안 됩니다.

`isdir` function이 있습니다. 이것은 file descriptor와 associated된 current file이 regular file인지 directory file인지 denote합니다. `fd`가 directory를 represent하면 true를 return합니다.

`inumber` function은 file descriptor와 associated된 inode number를 return합니다.

## Directory Entries

마지막 part입니다. special directory entries를 add해야 합니다. special directory entry dot은 itself를 represent한다는 뜻이고, dot dot은 parent directory를 represent합니다. 그래서 directory가 created될 때 special entries가 added되어야 합니다.

root directory에는 interesting property가 있습니다. 이것이 root directory를 위한 data block이라고 합시다. 이것은 root directory를 위한 inode이고, inode는 itself와 associated된 data block을 point하는 pointer를 가집니다. associated data blocks는 root를 위한 directory entries를 contain할 것입니다.

first entry의 current directory는 current directory를 위한 inode를 represent할 것이고, second는 parent entry일 것입니다. 하지만 root directory의 경우에는 이 둘이 same place를 point합니다. root directory에는 parent directory가 없기 때문입니다.

주의하세요. 이 two entries는 항상 there should be there always입니다. user가 그것들을 remove하려고 하면 system call은 failure를 return해야 하고 fail해야 합니다. directory entry에서 first entry와 second entry는 removable해서는 안 됩니다.

이것이 implement해야 할 functions의 end입니다. Good luck.
