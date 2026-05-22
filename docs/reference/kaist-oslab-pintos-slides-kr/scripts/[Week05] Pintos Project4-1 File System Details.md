# [Week05] Pintos Project4-1 File System Details

Source: https://youtu.be/mCAKZvZ1brs?si=wcu5zaZdeR6ICy6y

## Block Devices

block device는 SSD나 hard disk 같은 storage입니다. block device는 block의 linear array로 model되며, 각 block은 logical block address, 즉 LBA라는 address를 가집니다.

"block"의 의미는 context에 따라 달라집니다. Pintos에서 block device sector size는 512 bytes이고, Pintos file system도 512-byte sector를 file system block size로 사용합니다. 다른 file system은 4 KB처럼 더 큰 file system block을 사용할 수 있습니다.

Pintos는 block device를 `struct block`으로 표현합니다. 중요한 field는 sector 단위 device size, device name, block device type, device-driver operation pointer, 그리고 read/write된 sector count입니다.

file system code는 hardware를 직접 조작하지 않습니다. block layer를 호출하고, block layer가 device driver가 제공하는 operation을 호출합니다.

## Formatting the File System

file system을 format한다는 것은 clean block device에 initial file system data structure를 쓰는 것입니다.

Pintos의 기본 layout은 free-map bitmap의 inode, root directory의 inode, bitmap data blocks, root directory data block으로 시작합니다.

512-byte sector를 가진 8 MB partition에는 16,384 sectors가 있습니다. free map은 16,384 bits가 필요하므로 네 개 sector면 bitmap을 저장하기 충분합니다. 네 개 sector는 2,048 bytes이고, 2,048 bytes는 16,384 bits입니다.

formatting 중 Pintos는 free map을 initialize하고, bitmap file의 inode를 만들고, bitmap을 disk에 쓰고, root directory의 inode를 만들고, root directory를 initialize해야 합니다.

따라서 처음 몇 개 sector는 essential file system metadata가 차지합니다.

## File System Initialization

file system initialization은 크게 세 단계입니다.

첫째, `inode_init()`은 in-memory open inode list를 initialize합니다. Pintos는 open inode들을 linked list로 관리합니다.

둘째, `free_map_init()`은 free sector와 used sector를 나타내는 in-memory bitmap을 만들고 initialize합니다.

셋째, format이 요청되었다면 Pintos는 file system을 format합니다. formatting은 free-map file과 root directory를 disk 위에 만듭니다.

memory data structure는 volatile합니다. machine power가 꺼지면 memory는 사라집니다. 따라서 reboot 후에도 남아야 하는 file system metadata는 disk에 write되어야 합니다.

## Creating the Free Map File

free map 자체도 file로 저장됩니다.

이를 만들기 위해 Pintos는 free map을 위해 reserved된 sector에 inode를 만듭니다. 그런 다음 그 inode를 file로 open하고 bitmap contents를 disk에 씁니다.

이 flow에서 중요한 function은 `inode_create()`, `file_open()`, 그리고 free-map code가 사용하는 bitmap write operation입니다.

ordinary allocation이 시작되기 전에 free-map inode와 root-directory inode를 위한 reserved sectors는 bitmap에서 used로 표시되어야 합니다.

## Creating the Root Directory

root directory는 root-directory sector에 inode를 만들어 생성됩니다. lecture example에서 root directory는 처음에 16 entries 같은 fixed number의 directory entry 공간을 가집니다.

directory를 만드는 일은 inode만 만드는 일이 아닙니다. directory entry를 저장할 data block도 필요합니다. inode는 그 directory data block의 위치와 size를 기록합니다.

original Pintos implementation에서 allocation은 `free_map_allocate()`를 사용해 consecutive free sectors를 찾습니다.

## Creating a File

file creation은 file system state의 여러 부분을 수정합니다.

Pintos는 file을 위한 new inode가 필요합니다. file contents를 위한 data block도 필요할 수 있습니다. parent directory에는 new file name을 new inode sector에 mapping하는 directory entry를 추가해야 합니다. 또한 newly allocated sector들을 used로 표시하도록 free map도 update해야 합니다.

단순화한 creation path에서 `filesys_create()`는 root directory를 open하고, `free_map_allocate()`를 호출해 new inode를 위한 sector 하나를 allocate하며, `inode_create()`로 inode를 만들고, `dir_add()`로 directory entry를 추가합니다.

작은 file creation도 bitmap, new inode sector, allocate된 경우 file data sector, directory data block 등 여러 sector를 update할 수 있습니다.

## Opening an Inode

`inode_open()`은 disk에서 inode를 읽고 in-memory inode object를 return합니다.

Pintos는 open inode들의 global list도 유지합니다. 어떤 inode가 이미 open되어 있다면 같은 sector에 대해 duplicate in-memory inode를 만들지 않습니다. 대신 open count를 증가시키고 existing object를 return합니다.

open count는 해당 inode에 대해 몇 개의 open reference가 있는지 기록합니다.

이는 두 process가 같은 file을 동시에 open할 수 있기 때문에 중요합니다. 둘은 같은 underlying inode를 참조해야 하지만, 각 open file object는 자기 own file position을 가질 수 있습니다.

## Opening a Directory

directory를 open하는 것은 file을 open하는 것과 비슷하지만, object는 directory traversal을 나타냅니다.

`dir_open()`은 inode를 받아 directory object를 만듭니다. directory object는 inode pointer와 다음 directory entry operation이 일어날 위치를 기록하는 position field를 포함합니다.

directory의 data block은 directory entry의 array를 포함합니다.

## Allocating Free Blocks

`free_map_allocate()`는 bitmap을 scan해 free sector를 찾습니다.

original Pintos file system에서는 consecutive free bit run을 찾습니다. caller가 16 sectors를 요청하면 bitmap에는 16개의 consecutive free bit가 있어야 합니다. 그런 region을 찾으면 해당 bit들을 used로 표시하고 starting sector를 return합니다.

이 contiguous allocation strategy는 original file system이 file을 쉽게 extend하지 못하는 이유 중 하나입니다.

## Adding a Directory Entry

`dir_add()`는 file name과 inode sector를 directory에 추가합니다.

먼저 주어진 name이 directory에 이미 존재하는지 확인합니다. 이미 존재하면 creation은 fail합니다.

그다음 directory data block을 scan해 unused entry를 찾습니다. empty slot을 찾으면 그 slot에 new name과 inode sector를 쓰고 entry를 in use로 표시합니다.

이 scan은 linear입니다. 단순하지만 큰 directory에서는 비싸질 수 있습니다.

## Looking Up a Directory Entry

`dir_lookup()`은 directory에서 주어진 file name을 검색합니다.

directory entry를 scan하고 name을 비교합니다. matching name을 찾으면 associated inode를 return합니다. 이 operation은 file name을 file을 나타내는 inode sector로 변환합니다.

original Pintos file system에는 root directory만 있으므로, 대부분의 path resolution은 root directory에서 name을 lookup하는 것으로 줄어듭니다.

## Opening a File

`filesys_open()`은 `open` system call이 사용하는 file-system-level operation입니다.

이는 root directory를 open하고, requested file name을 lookup하고, inode를 얻은 뒤, 그 inode에 대해 `file_open()`을 호출합니다.

`file_open()`은 `struct file`을 allocate하고 initialize합니다. file object는 inode pointer를 저장하고, current position을 zero로 initialize하며, write-denial state를 initialize합니다.

file을 open할 때마다 new `struct file`이 생성됩니다. underlying inode가 이미 open되어 있어도 마찬가지입니다.

## Removing a File

file removal은 두 부분으로 이루어집니다.

첫째, Pintos는 directory entry를 not in use로 표시해 directory entry를 제거합니다. 이렇게 하면 file name에서 inode로 가는 mapping이 끊어집니다.

둘째, Pintos는 inode를 removed로 표시합니다. `inode_remove()`는 in-memory inode의 removed flag를 설정합니다. 다른 process가 아직 file을 open하고 있을 수 있기 때문에 inode와 data block이 즉시 free되는 것은 아닙니다.

실제 deallocation은 open count가 zero가 되는 `inode_close()`에서 일어납니다. 그 시점에 Pintos는 inode sector와 file data sector를 free map에 돌려줄 수 있습니다.

이 delayed removal 때문에 file name을 제거하는 것과 underlying inode를 free하는 것은 별도의 step입니다.
