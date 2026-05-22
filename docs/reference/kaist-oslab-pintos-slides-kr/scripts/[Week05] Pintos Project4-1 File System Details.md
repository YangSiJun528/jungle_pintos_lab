# [Week05] Pintos Project4-1 File System Details

Source: https://youtu.be/mCAKZvZ1brs?si=wcu5zaZdeR6ICy6y

## Block Device

안녕하세요. 이 lab은 Pintos의 file system details에 대한 것입니다.

block device는 SSD, hard disk drive 같은 storage를 가리킵니다. block device는 block들로 구성됩니다. 이것은 block들의 set이고, block들의 linear array이며, 각 block은 자기 number를 가집니다. 우리는 이것을 logical block address, LBA라고 부릅니다.

어떤 context에서는 각 block이 512-byte sector를 가리키고, 어떤 context에서는 4-kilobyte file system block을 가리킵니다. 그래서 각 occasion에서 LBA가 무엇을 가리키는지 확인해야 합니다.

이것이 Pintos operating system에서 block device를 표현하는 방식입니다. 이것은 data structure입니다. data structure의 이름은 `block`입니다. block에서 가장 중요한 attribute는 아마 size, sector 단위의 size입니다. 그리고 Pintos는 block device의 attribute로 16-character-length name을 allocate합니다. 그 다음 block device type이 있고, specific block device에 대해 정의된 operation들의 set을 포함하는 device driver를 가리키는 pointer가 있습니다.

Pintos는 block device에 대해 read count와 write count를 정의하는데, 이것은 read된 sector 수와 written된 sector 수를 가리킵니다. 그래서 이것이 block device입니다.

file system을 formatting한다는 것은 이 block들 각각 위에 어떤 contents를 쓰는 process입니다.

## Pintos File System Layout

이것은 Pintos file system의 layout입니다. 이것은 block 0, block 1, block 2, block 3, block 4, 이런 식입니다.

여기 Pintos에서 각 block은 512 bytes입니다. file system block size는 file system이 결정합니다. file system을 4-kilobyte blocks로 format할 수도 있고, 32-kilobyte blocks로 format할 수도 있고, 심지어 512-byte blocks로 format할 수도 있습니다. 여기 Pintos에서는 sector size와 file system block size가 모두 512 bytes입니다.

이것은 Pintos file system의 basic sample layout입니다. 첫 번째 block에는 bitmap을 위한 inode가 있습니다. bitmap은 각 block이 사용 중인지 아닌지를 결정하는 data structure입니다. 두 번째 block은 root directory를 위한 inode가 차지합니다. 그 다음 네 개 block이 오며, 각각은 이 file system partition을 위한 bitmap을 나타냅니다. 그 다음 root directory가 있습니다. 물론 진행하면서 각각의 data structure를 detail하게 설명하겠습니다.

bitmap을 위한 inode를 만들고, root directory를 위한 inode를 만들고, bit stream을 만들고 initialize하며, directory를 만들고, 이 data structure의 initial value를 initialize하고 save하는 것을 file system formatting이라고 합니다.

그래서 기본적으로 file system formatting은 clean block device에서 file system을 operate하는 데 필요한 essential data structures의 initial values를 write하는 것입니다.

## Bitmap Size

여기에서 bitmap size를 보겠습니다. block들을 위해 네 개 sectors를 allocate하고, 각 block은 sector에 대응합니다. 각 sector는 512 bytes이므로 bitmap을 위해 2048 bytes를 allocate하는 것이고, 이것은 16,384 bits에 해당합니다.

좋습니다. 그래서 bitmap 안에는 16,384 bits가 있고, 각 bit는 주어진 sector가 free인지 being used인지 나타냅니다. 그래서 bitmap 안의 bit 수에 sector size를 곱하면 file system size를 얻습니다. 이 file system partition에는 8 megabytes가 있습니다.

bitmap을 위해 four sectors를 가지고 있다면, 그것은 8-megabyte file system partition을 cover할 수 있습니다. bitmap을 eight로 double하면 당연히 16-megabyte file system partition을 cover하게 됩니다. 이것이 작동 방식입니다.

## Formatting Steps

이제부터 initial file system boot stage에서 각각의 field가 어떻게 occupied되는지 설명하겠습니다.

file system을 formatting할 때 Pintos는 기본적으로 세 단계를 만듭니다.

첫째, file system partition의 각 sector가 어떻게 사용 중인지 create하고 initialize합니다. 물론 file-system formatting phase에서는 대부분의 sector가 free입니다.

bitmap을 만든 다음, bitmap을 file로 간주합니다. bitmap도 file이므로 bitmap을 위한 inode도 만듭니다.

file system의 basic data structure는 root directory이므로 root directory도 만들어야 합니다. 물론 bitmap을 위한 inode를 만든 뒤에는 그것을 disk에 write해야 하고, file-system state를 disk에 safely synchronize하기 위해 bitmap을 disk에 write해야 합니다.

세 단계가 있습니다. bitmap을 create하고 initialize하는 것이 first step입니다. second step에서는 bitmap을 위한 inode를 create하고 그 data를 disk에 write합니다. third step은 root directory의 inode를 create하는 것입니다. 이것이 first step, second step, third step입니다.

좋습니다. file system을 formatting하는 과정에서 이제 sector 0부터 write합니다. 저는 특히 Pintos file system에서는 sector와 block을 interchangeably 사용하고 있습니다. Pintos file system에서는 각 block이 sector에 대응하기 때문입니다.

그래서 Pintos에서 file system formatting은 file system의 first six blocks를 먼저 write하고 initialize하는 것에 해당합니다. bitmap을 위한 inode block, root directory를 위한 inode block, 그리고 inode bitmap을 initialize하는 것입니다. 이 part는 tricky합니다. 처음에 file system이 first created될 때 root directory를 위한 empty directory가 created되지만, 그 directory 안에는 contents가 없습니다. 어떻게 되는지 봅시다.

## `filesys_init()`

이것은 file system을 initializing하는 code입니다. 이것은 `filesys.c`에 있습니다. function의 name은 `filesys_init`입니다.

이것은 세 가지 basic steps를 포함합니다. inode를 initializing하고, bitmap을 initializing하고, format을 perform합니다. 이것이 무엇에 대응할까요? details를 보겠습니다.

첫 번째 function인 `inode_init`은 open files를 위한 data structure를 create하고 initialize합니다. system이 first boots up할 때, computer system에는 두 basic worlds가 있습니다. 첫 번째는 memory이고, 두 번째는 disk입니다.

system이 start할 때, 이것을 system boot라고 부르는데, open files의 list를 가리키는 empty list를 만듭니다. inode는 file을 나타내는 data structure입니다.

또한 `free_map_init`에서는 8 megabytes를 cover할 수 있는 bitmap을 만들어야 하고, 그것을 properly initialize해야 합니다.

formatting은 이 data structure를 disk에 properly write하는 것입니다. 여기에서 formatting에는 두 가지가 있습니다. 첫 번째는 bitmap file을 create하고 write하는 것이고, formatting의 두 번째 part는 root directory를 create하는 것입니다. 두 가지 important things가 있습니다.

그래서 기억하세요. inode를 initialize하고, bitmap을 initialize한 다음, format을 perform합니다. format은 bitmap을 disk에 write하는 process이고, root directory를 disk 위에 create하는 process이며, file을 create하는 것은 두 parts로 구성됩니다. 첫 번째는 inode를 creating하는 것이고, 두 번째는 inode와 associated된 data blocks를 creating하는 것입니다.

좋습니다. 넘어가겠습니다. 이것이 file system을 initializing하는 process입니다. 우리는 이것을 format이라고 부릅니다.

앞에서 보았듯이 첫 두 important parts는 open files를 위한 empty inode list를 create하는 것, 둘째 bitmap을 create하는 것, 그리고 format을 perform하는 것입니다. format은 bitmap file의 inode를 create하고 disk에 save해서 crash recovery를 위해 disk를 save하는 것, 그리고 root directory를 create하는 것으로 구성됩니다. 두 가지가 있습니다.

## Open Inode List

좋습니다. actual code를 보겠습니다. 우리가 해야 할 첫 번째 일은 open inodes의 list를 initialize하는 것입니다. 이것은 in-memory inodes의 list를 initializing하는 것에 해당합니다.

`inode_init` function은 `list_init`을 call합니다. 이것은 beginning에서 empty inode list를 initialize하는 typical function입니다. 그것은 `open_inodes`입니다. empty list를 create하기 위한 essential function입니다.

이것들이 Pintos 안에 있습니다. 보시다시피 opened files에 대한 inode들의 set을 linked list로 maintain합니다.

제 질문은, V6 operating system에서 open files를 나타내는 data structure는 무엇인가입니다. 아마 Pintos와 xv6, 또는 더 넓게는 operating system education을 위한 operating system들은 operating system의 각 concept를 표현하기 위해 slightly different data structures를 사용합니다.

class와 textbook에서는 xv6 code를 사용하고, lab에서는 Pintos를 다룹니다. operating system concept를 구현하는 different ways에 관심이 있다면 comparison purpose로 두 code를 모두 읽어 보면 좋습니다. 그래서 제 질문은, xv6에서 open files를 나타내는 data structure는 무엇인가입니다. 확인해 보세요.

## Creating and Initializing the Bitmap

첫 번째 step은 bitmap을 creating하고 initializing하는 것입니다. 먼저 block size의 bitmap을 create합니다. 여기서 file system은 앞에서 보았듯이 8 megabytes입니다.

가장 beginning에는 bitmap을 위한 inode 하나와 root directory를 위한 다른 inode 하나가 필요합니다. 그래서 그 bitmap entries를 being used로 mark해야 합니다. bitmap을 위한 inode를 sector 0에 allocate할 것이고, root directory를 위한 inode를 sector 1에 allocate할 것입니다. 그래서 bitmap 안의 associated entries를 mark해야 합니다.

이것이 우리가 mark할 location입니다. 이것이 root directory에 대해서도 mark할 location입니다. 이것은 bitmap들의 array를 가리키는 pointer입니다.

file system의 initial line에는 세 steps가 있습니다. bitmap의 inode를 creating하고, 물론 bitmap array를 initializing한 뒤에는 synchronization purpose를 위해 그것들을 disk에 write해야 하며, 그 다음 root directory를 create해야 합니다. bitmap의 contents를 disk에 write하고, root directory의 inode를 disk에 create합니다.

좋습니다. 이 function에는 두 steps가 있습니다. free map file을 create하고, 둘째 root directory를 create합니다. 해야 할 두 가지가 있습니다. 첫 번째는 bitmap file을 creating하는 것이고, 두 번째는 root directory를 creating하는 것입니다.

특히 root directory의 경우 second parameter는 directory 안의 entries 수, directory 안의 entries 수입니다. 여기에서는 maximum 16 entries로 root directory를 initialize합니다. 이것은 root directory를 위한 inode를 배치할 inode의 location입니다.

## Creating the Free Map File

첫 번째 step은 bitmap file을 create하고 save하는 것입니다. inode를 create하고 disk에 write합니다. 이것은 bitmap을 creating하는 actual code입니다.

먼저 bitmap을 위한 inode를 create해야 합니다. 첫 번째 parameter는 주어진 file을 위한 inode를 create하고 싶은 inode의 location입니다. 그 다음 주의해야 할 세 functions가 있습니다. `inode_create`, `file_open`, 그리고 `bitmap_write`입니다.

bitmap file을 위한 inode를 create한 뒤에는 그 inode를 memory로 가져오고 bitmap의 contents를 disk에 save합니다. 이것들이 free map을 creating하기 위한 세 basic functions입니다.

## Creating the Root Directory

다음은 root directory를 creating하는 것입니다. root directory를 creating한다는 것은 root directory를 위한 inode를 creating하는 것에 해당합니다. 그래서 이 location, inode의 location에 inode를 create하고, 그 다음 directory entries의 size를 지정합니다.

다시 말해 `inode_create`는 two parameters를 받습니다. root directory의 경우 이것은 root directory를 위한 inode를 가진 block number이고, entry count는 root directory 안의 maximum number of entries입니다. 그래서 code에서 이것을 16으로 specify합니다.

두 번째는, inode를 creating한 뒤에는 당연히 모든 directory가 inode 외에 directory entries를 save할 place가 필요하다는 것입니다. 그래서 root directory를 위한 data blocks를 allocate해야 하고, data block의 start address를 inode에 save해야 합니다.

두 functions가 있습니다. `bytes_to_sectors`는 number of bytes를 number of sectors로 translate하는 simple function이고, `free_map_allocate`는 bitmap에서 certain amount of data blocks를 allocate합니다. 이것은 contiguous blocks를 allocate합니다.

예를 들어 16 sectors를 allocate하고 싶다면, free bitmap을 scan하고 이 bits가 모두 free인 consecutive bits array를 찾아야 합니다. 이것이 예를 들어 16-bit array입니다. 이 block은 occupied, occupied, occupied이고, maybe not occupied이고, occupied입니다. 물론 이 bit는 occupied되어 있지 않지만, next bit가 being used이므로, 이 third bit가 free여도 fourth bit가 being used이기 때문에 consecutive 16 bits를 allocate할 수 없습니다.

consecutive 16 bits가 free인 것을 찾으면, consecutive 16 bits와 corresponding blocks를 allocate하고, corresponding sectors의 start address를 return합니다. 이것이 `free_map_allocate`가 사용되는 목적입니다.

그 다음 starting address를 associated inode, starting point에 save해야 합니다. 그리고 disk inode를 disk에 safely write합니다.

확실히 해야 할 한 가지는 memory, 우리가 DRAM이라고 부르는 것이 volatile하다는 점입니다. power가 꺼지면 모든 contents를 잃는다는 뜻입니다. storage, 보통 disk는 non-volatile입니다. contents를 disk에 save하면 power가 꺼져도 그것을 가질 수 있습니다.

하지만 모든 operating system operation은 memory data structure를 다룹니다. 그래서 조심해야 합니다. power crash가 있어도 data structure를 carefully and safely save해야 한다면, operation이 complete될 때마다 모든 data structure가 disk에 safely stored되도록 해야 합니다. 그래서 disk inode를 disk에 write합니다.

좋습니다. bitmap contents를 disk에 write한 뒤에는 inode를 close합니다. inode를 closing한 뒤에는 in-memory inode를 open inode list에서 deallocate하고 remove하며 disk에 save합니다.

다음 step은 bitmap을 memory로 load하는 것입니다. 그래서 disk 위의 bitmap contents를 disk에서 memory로 read합니다. 이것은 두 phases로 구성됩니다. 첫 번째는 file을 open하는 것이고, 그 다음 read하는 것입니다.

## Creating a File

이제 file을 creating하는 details를 설명하겠습니다.

file을 creating하는 것은 `filesys_create`가 수행합니다. 이것은 system call `create`의 driver function입니다. code의 details를 보면 file system이 file을 어떻게 create하는지 배울 수 있습니다.

file을 creating하는 것은 inode를 creating하는 것으로 구성됩니다. file system에서 file을 creating할 때 어떤 data structure를 modify해야 하는지 생각해 봅시다.

file을 create한다고 합시다. 그러면 new file을 위한 inode를 create해야 합니다. 물론 주어진 file과 associated된 data blocks를 initialize하고 싶을 수도 있습니다.

그 다음 newly created file이 속하는 parent directory를 modify해야 합니다. 예를 들어 file의 name이 `a.c`라고 합시다. 그러면 `a.c`는 file name과 inode number인 pair를 포함합니다.

좋습니다. 이것은 data이고, 이것은 new file data block이며, associated inode입니다. 그리고 directory block이 있습니다. 그게 전부일까요? 사실 하나 더 있습니다. file system에는 bitmap이 있습니다.

file system에서 bitmap을 위해 four sectors를 allocate했습니다. 이것이 first sector, second sector, third sector라고 합시다. 이 specific example에서 inode를 위한 one sector와 data block을 위한 one sector, 두 blocks를 allocate했기 때문에 이 bitmap의 어떤 part를 update했을 수 있습니다. third part에서 two bits를 update했다고 합시다.

좋습니다. 그러면 file을 creating하는 course에서 적어도 four blocks를 update했습니다. first block, second block, third block, fourth block입니다. one, two, three, four입니다. 그래서 file을 creating하는 course에서 four blocks를 update하고 modify했습니다. 이 모든 blocks를 disk에 synchronize해야 합니다.

보시다시피 file을 creating하는 것은 non-trivial exercise입니다.

file system의 details는 inode를 create하고 initialize하고, disk에 write하고, 그 다음 root directory에 new entry를 add하는 것입니다. 좋습니다. 이것이 file을 creating하는 simplified steps입니다. 이 example에서는 data blocks를 allocate하지 않았고, bitmap이 어떻게 allocated되고 updated되는지도 보이지 않습니다. 하지만 어떻게 작동하는지 보여 줄 것입니다.

## `filesys_create("testfile")`

이것이 file을 creating하는 detailed steps입니다. file의 name은 `testfile`입니다. 그래서 function `filesys_create("testfile")`을 call합니다.

가장 첫 step에서는 root directory의 inode를 disk에서 memory로 read하고, 그것을 open inode list에 insert합니다.

second step은 test file의 inode를 disk에 write하는 것, inode를 create하는 것입니다.

그 다음 root directory의 entries를 read하고, test file을 위한 new directory entry를 add한 다음, root directory의 entries를 write합니다. 좋습니다. 그래서 이것은 five steps로 구성됩니다.

질문을 하나 하겠습니다. 이 five steps 중에서 inode block과 directory block을 modifying하고 updating하는 steps만 보여 줍니다. directory block은 updated되지만, inode block은 newly allocated됩니다. 그래서 new inode, inode를 위한 new block을 allocating하는 course에서는 bitmap을 update해야 합니다. free block을 찾아서 associated bitmap을 marked로 mark해야 합니다.

그러면 five steps 중 어디에 bitmap을 updating하는 process를 include해야 할까요? 이것이 1.5, 2.5, 3.5, 4.5, 5.25, 그리고 0.5라고 합시다. 가능한 여섯 positions 중 어디에 bitmap에서 free entry를 finding하는 process를 include해야 할까요? 생각해 보세요.

물론 answer는 곧 나올 것입니다.

## `filesys_create()` Code

좋습니다. 다음 page로 넘어가겠습니다. 당연히 바로 뒤에 answer가 나옵니다.

먼저 root directory를 open해야 합니다. file을 creating하는 것은 initialized inode를 creating하고 root directory에 new directory entry를 adding하는 것으로 구성됩니다. current Pintos file system에서는 hierarchical directory structure가 없습니다. root directory만 있습니다.

그래서 root directory를 open하고, root directory가 successfully opened되면 free block을 allocate합니다.

이것은 allocate하고 싶은 sectors의 number입니다. 이것은 number of sectors, allocate하고 싶은 sectors의 number입니다. new block을 allocating하는 course에서 그 sectors의 start address를 specified inode address에 insert합니다.

좋습니다. 여기에서 `free_map_allocate`는 specifically one sector를 free map에서 allocate하고, start address를 inode sector에 save합니다. 이것이 sector를 create하는 방식입니다.

`free_map_allocate`를 calling하는 course에서, 이 function 안에서 free bitmap array를 automatically update합니다. 그 다음 당연히 `inode_create`에서는 inode를 creating하는 course에서 initial size bytes로 inode를 initialize하고 disk에 write합니다. 그리고 just-created file의 parent directory로 specified된 `dir`에 created directory entry를 add합니다.

좋습니다. 이것이 file을 create하는 방식입니다.

## Opening an Inode

이제 inode를 first time open하는 procedure를 설명하겠습니다.

우리가 concern하는 function은 `inode_open`입니다. 이것은 on-disk inode를 read합니다. 이것이 first parameter입니다. sector location에서 disk의 inode를 read하고, 그 pointer를 create합니다.

물론 `inode_open`의 important operation은 read한 inode를 open inode의 linked list에 inserting하는 것입니다. Pintos는 system 안에서 open inodes의 list를 maintain합니다. 이것은 global linked list입니다. inode를 opening하는 course에서 이 inode를 list에 insert해야 합니다. 그래서 이것이 inode를 open inode list에 inserting하는 process입니다.

그 다음 fields를 properly set해야 합니다. 주의해야 할 important field가 하나 있습니다. 이 open count입니다. 이것이 open count입니다.

두 개 이상의 process가 file을 open할 수 있습니다. process 1과 process 2가 있고, 두 file이 예를 들어 `open("a.c")` function을 call한다면, 같은 file이 different processes에 의해 twice opened됩니다. 이 경우 open count는 2로 set됩니다. 그래서 이것은 주어진 file을 opened한 processes의 number를 나타냅니다.

directory를 opening하는 것은 file을 opening하는 것과 비슷하지만, inode 자체 대신 directory entries가 opened된다는 점이 다릅니다. `dir_open`은 inode pointer를 parameter로 받습니다. file을 opening할 때는 directory structure를 allocate하고, 이 directory entry를 read하고, directory structure의 fields를 inode와 associated position으로 initialize합니다.

## `free_map_allocate()`

important function, 가장 essential한 function은 free block을 allocating하는 것입니다. 이것이 `free_map_allocate` function입니다.

이것은 two parameters를 받습니다. 첫 번째는 count입니다. `free_map_allocate`의 objective는 free blocks를 scanning해서 `cnt` consecutive blocks를 finding하는 것입니다. 그래서 `cnt`는 allocate할 blocks의 number를 뜻합니다. 결과적으로 sector position `sectorp`는 allocated된 blocks의 start address를 specify합니다.

free block을 allocating하는 course에서 이것은 free bitmap을 set합니다. 몇 slides 전에 제 질문은 new block을 allocating하는 course에서 free block bitmap이 어디에서 updated되는가였습니다. 그것은 `free_map_allocate` 안입니다.

그래서 consecutive false bitmap, 또는 false bits를 찾습니다. bitmap은 0으로 set되어 있고, 그것들을 true로 set합니다.

## Creating a Directory Entry

다음 function은 directory를 어떻게 create하는가입니다. 몇 slides 전에 file을 create하는 function을 보았습니다. 이제 directory를 create하는 function을 보고 있습니다.

우리가 하고 싶은 것은 named file을 directory에 add하는 것입니다. 그러면 file의 inode는 `inode_sector` parameter로 inserted되어야 합니다. 그래서 이것은 three parameters를 가집니다. name, directory, inode sector입니다.

directory는 newly created file이 located된 target directory입니다. name은 newly created file의 name입니다. 그리고 newly created file을 위한 inode의 associated location입니다.

먼저 해야 할 일은 directory를 look up하고, given name 아래의 file이 already exists하는지 아닌지를 check하는 것입니다. 존재한다면 creation attempt는 fail해야 합니다.

그 다음 directory block을 scan하고 empty spots를 find해야 합니다. 이것이 directory block이라고 합시다. 예를 들어 first entry는 `a.c`, second entry는 `b.c`, third entry는 empty, fourth entry는 `d.c`입니다. 그러면 directory entry를 creating하는 course에서 먼저 entire directory block을 scan해서 first empty spot을 find하고, 그 position에 block을 create해야 합니다.

여기에서 주의할 점은 directory block을 scanning하는 것이 매우 time-consuming하다는 것입니다. 매우 expensive operation입니다. file을 frequently create하는 application을 write한다면, file system은 file creating에서 매우, 매우 efficient해야 합니다. file을 create할 때마다 empty spot을 찾기 위해 모든 directory blocks를 linearly scan할 수는 없습니다. 그것은 매우, 매우 expensive할 것입니다.

어쨌든 file을 creating할 때, 특히 directory block에서 empty spots를 finding할 때는 more sophisticated data structure를 사용하고 싶을 수 있습니다.

좋습니다. 이것이 directory entry를 create하는 방식입니다.

## Directory Lookup

다음 slide로 넘어가겠습니다. 이제 directory lookup입니다.

directory lookup은 directory pointer를 받고 name을 받으며, directory entry pointer와 offset을 받습니다. lookup은 function이고, file name이 directory 안에 exists하는지 아닌지를 check합니다. 그리고 supplied한 parameter를 사용해서 directory entry structure의 address를 return합니다.

그래서 input은 given directory에 대해 name을 find하고, supplied parameter를 사용해서 directory entry의 address를 return하는 것입니다. 이것이 lookup입니다.

## Opening a File

다음으로 다룰 function은 file을 opening하는 것입니다. 이것은 `filesys_open`이라고 부르고, name을 받습니다. file system을 open하고, `struct file`을 가리키는 pointer를 return합니다.

`filesys_open`은 system call `open`에 의해 called되고, 두 가지 일을 합니다.

첫째, inode를 open inode list에 add합니다. 둘째, `struct file`을 allocate하고 initialize한 다음 그 address를 return합니다.

첫 번째의 경우, 먼저 file이 다른 open system call에 의해 already opened되었는지 check해야 합니다. 그런 경우에는 same inode를 list에 twice insert할 필요가 없습니다. 대신 inode list 안의 inode entry의 reference count를 increase하면 됩니다.

하지만 file이 opened될 때마다, 즉 `filesys_open`이 called될 때마다 `struct file` structure를 allocate하고 initialize해야 합니다. 그래서 이것이 file을 allocating하고 opening하는 process입니다.

이것은 file을 opening하는 process입니다. example을 하나 들겠습니다. file의 name은 `testfile`입니다. `filesys_open("testfile")`은 다음 five steps로 구성됩니다.

먼저 root directory의 inode를 read해야 합니다. 그 다음 root directory 안의 entries를 scan하고 name `testfile`을 find합니다. directory entries를 scan해서 `testfile`을 find합니다. 그 다음 matching inode를 find하고, `testfile`의 inode number가 7이라는 것을 찾습니다.

third step으로 `testfile`의 in-memory inode를 inode list에 insert해야 합니다. 그래서 inode를 inode list에 inserting하는 것은 끝났습니다.

그 다음 next structure로 `struct file` data structure를 allocate합니다. file을 open할 때마다 `struct file` data structure를 allocate해야 하고, 그 안에 in-memory inode의 address를 set하고, 그 address를 return해야 합니다.

물론 `testfile`에 대한 inode가 in-memory inodes 안에 already exists할 수도 있습니다. 그런 경우에는 다시 read하거나 다시 insert할 필요가 없습니다.

이것이 file을 opening하기 위한 system call입니다. file을 opening하는 가장 important task는 `struct file`을 allocate하고 initialize해서 그 address를 return하는 것입니다. 그래서 inode에 대해 `file_open`을 return합니다. given file에 대한 inode number를 find하고 `file_open`을 call합니다.

## `file_open()`

이것은 essential function이며, memory 안에서 initialized `struct file`을 allocate합니다.

다음 function은 directory lookup입니다. directory lookup은 쉬운 function입니다. directory를 open하고, name `name` 아래의 file을 look up한 다음 inode를 return합니다. directory lookup의 목적이 그것입니다.

associated inode를 찾고 나면 file을 open할 차례입니다. inode를 얻고, inode number를 `file_open` function에 supply합니다. `file_open`의 role은 `struct file`을 위한 data structure를 allocate하고, initialize하고, 그 address를 return하는 것입니다.

그래서 `struct file`을 allocate하고 initialize합니다. 어떤 value로 채우고 pointer를 return합니다. file의 start address를 return합니다.

file structure를 initializing하는 process는 무엇일까요? `struct file`을 initializing하는 것은 바로 여기입니다. file structure는 자기 inode를 가리키는 pointer를 가집니다. file struct의 가장 important attribute는 offset입니다. 여기서는 이것을 position이라고 부릅니다. position은 read와 write operation을 apply할 file의 offset을 뜻합니다. 이것은 offset이라고 부르고, Pintos에서는 `pos`, position이라고 부릅니다.

file을 blindly open할 때는 deny-write field를 false로 set해야 합니다. 하지만 이 file이 executable이거나, 어떤 special case에서 어떤 reason으로 이 field를 true로 write하고 싶을 수 있습니다.

좋습니다. 이것이 file을 open할 때 `struct file`을 initializing하는 important step입니다. inode를 creating하고 position을 initializing하는 것입니다. 이것이 file을 opening하는 목적입니다.

## Removing a File

file을 removing하는 steps를 제공하겠습니다. 대응되는 system call은 `filesys_remove`입니다.

이것이 기본적으로 하는 일은 inode 안의 `removed` flag를 true로 set하고, directory entry를 remove하는 것입니다. 해야 할 두 steps입니다. flag를 set하고 directory entry를 remove합니다. 이것이 flow입니다.

root inode를 open하고 given file을 위해 root directory를 search합니다. 존재하지 않으면 그 failure case에서는 root directory inode를 unlock하고 false를 return합니다. 기본적으로 deletion failure를 뜻합니다.

file이 directory 안에 exists하면 directory entry는 false가 됩니다. directory entry의 `in_use` field는 false로 set되고, inode의 removed flag는 true로 set됩니다. 그 다음 root directory에 대한 in-memory inode를 deallocate하고 return합니다.

이것은 test file을 removing하는 example입니다. `filesys_remove`를 call하고, `testfile`을 call합니다. 그러면 root directory의 inode를 read하고, root directory의 entries를 scan하고, target file을 find하고, `in_use`를 false로 set하고, inode에서는 `removed` field를 true로 set합니다.

actual code를 보겠습니다. 두 가지가 있습니다. 먼저 directory에서 target file entry를 remove하고, 그 다음 inode의 removed flag를 true로 set합니다. 그래서 기본적으로 해야 할 두 가지가 그것입니다.

먼저 directory entry flag를 false로 set합니다. 그것이 flag입니다. `dir_remove` function은 target directory의 entry를 remove하고, in-memory inode 안의 removed flag를 true로 set한 뒤, updated directory entry를 disk에 write합니다.

이것은 `dir_remove`의 details입니다. 이것은 two parameters를 받습니다. 첫 번째는 target directory이고, 두 번째는 name입니다. 이것은 remove하고 싶은 file의 name입니다. first step은 directory를 search하고 associated directory entry를 get한 다음, 그것을 false로 mark하고, updated directory를 disk에 write back하는 것입니다. 그래서 `inode_write_at`이 associated directory의 disk contents를 update합니다.

directory entry를 removing한 뒤에는 file의 inode itself를 remove할 차례입니다. 가장 먼저 해야 할 일은 inode를 open하고, `inode_remove`라는 function을 call하는 것입니다.

하지만 `inode_remove`를 calling한다고 해서 inode가 immediately deleted되는 것은 아닙니다. inode가 다른 process, more than one process에 의해 opened되었을 수 있기 때문입니다. 그래서 `inode_open`은 in-memory inode를 open inode list에 add하고, 그 다음 `inode_remove`를 call합니다.

`inode_remove`에서 가장 important part는 in-memory inode의 `removed` flag를 true로 setting하는 것입니다. 좋습니다. 그게 전부입니다. inode 안의 `removed` flag를 true로 setting하는 것 외에는 아무것도 하지 않습니다. 그 이상은 없습니다.

그 다음 `inode_close` function을 call합니다. `inode_close`를 calling하는 course에서 모든 dirty work를 수행합니다. reference count를 check하고, 이 inode를 참조하는 다른 process가 있는지 check합니다. 완전히 free라면 disk에서 associated inode를 deallocate하고 associated sectors를 free로 만듭니다. 그것이 `inode_close`의 role입니다.

이것이 file을 removing하는 것입니다.

## Covered Interfaces

이 chapter에서는 file-system interfaces의 몇 가지 details를 다루었습니다. file system formatting, file creating, directory creating, directory lookup, file opening, file removing입니다. 이것들이 Pintos file system에서 file system의 basic steps입니다. 이제 끝났습니다.
