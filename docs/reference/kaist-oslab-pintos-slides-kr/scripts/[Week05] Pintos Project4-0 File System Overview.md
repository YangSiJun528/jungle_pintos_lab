# [Week05] Pintos Project4-0 File System Overview

Source: https://youtu.be/bqtjcc7-_yA?si=XcxTIi-JIpqSXjue

## Project 4 Overview

안녕하세요. 이제 project number four를 구현하는 자세한 단계를 설명하겠습니다. 이것은 file system입니다.

project number four에는 기본적으로 세 가지가 있습니다. Pintos file system에 buffer cache를 구현할 것입니다. 그리고 Pintos file system을 확장해서 individual file들이 block들의 set으로 표현되고, indexed되고 extensible해지도록 만들 것입니다. 그리고 subdirectory를 구현할 것입니다.

넘어가기 전에 file system의 몇 가지 기본 concept를 설명하겠습니다.

## Inode

file system에서 가장 중요한 data structure 중 하나는 inode입니다. 죄송합니다, inode입니다.

inode는 disk 위의 file, disk 위의 file을 나타냅니다. 그래서 모든 file은 data structure로서 자기 own inode를 가집니다. 그리고 이것은 무엇을 나타낼까요? file의 size, 즉 file이 얼마나 큰지를 포함하고, file에 속한 data block들의 location을 포함합니다. 그래서 file에 속한 disk block들을 가리키는 pointer일 수 있습니다. 또한 permission, 마지막으로 access된 time, 가장 최근의 modification time 등도 포함합니다.

inode에는 두 종류가 있습니다. on-disk inode와 in-memory inode가 있습니다. disk 위에는 inode들의 array가 있고, 각 inode는 disk 위의 file들을 나타냅니다. operating system이 disk 위의 file에 접근하려면 disk에서 inode를 읽어야 하고, 그 disk-based inode에 기반해서 in-memory inode를 만듭니다.

그래서 in-memory inode는 on-disk inode의 superset입니다. 이것은 on-disk inode이고, in-memory inode, 또는 in-core inode는 on-disk inode와 몇 가지 다른 information을 포함하는 data structure를 나타냅니다. 그 other information에는 on-disk inode의 disk location이 포함되고, 또한 그 inode가 속한 file system을 포함할 수도 있습니다.

## File Object and Current Offset

또 다른 중요한 data structure는 file object입니다. 그래서 inode object가 있고 file object가 있습니다. file object는 open된 file을 나타냅니다.

UNIX에서 file이 open되면 operating system은 current offset을 정의합니다. 이것은 modern file system에서 가장 중요한 concept 중 하나입니다. current offset은 read 또는 write application, read 또는 write system call이 적용되어야 하는 file 안의 position을 나타냅니다.

예를 들어, 여기 이것이 큰 file이고, 이것이 file의 start이고, 이것이 file의 end입니다. 그러면 UNIX operating system은 current offset이라는 attribute를 정의하는데, 이것은 read 또는 write operation이 적용되어야 할 location을 나타냅니다. read 또는 write system call이 적용되고 나면, offset은 read된 data의 양 또는 write된 data의 양만큼, read/write system call에 기반해서 update됩니다. 그래서 update됩니다.

좋습니다. 그래서 inode와 file object라는 두 concept가 있습니다.

## Regular Files, Directories, and Bitmap

넘어가기 전에 다른 basic concept를 소개하겠습니다. file이 있습니다. 이것은 그냥 regular file입니다. 그리고 directory가 있고 bitmap이 있습니다.

각 file은 actual data를 가집니다. 이것들이 data blocks입니다. music file일 수도 있고, video file일 수도 있고, Microsoft Word file일 수도 있고, C code일 수도 있습니다. 어쨌든 music contents, video images, documents, 또는 C files를 포함할 수 있습니다. 이것이 file의 actual contents입니다.

하지만 모든 file은 associated inode를 가집니다. inode는, 앞서 말했듯이, size와 permission을 포함하고, 각 inode는 자신이 가지고 있는 data block들을 가리키는 pointer를 포함합니다.

좋습니다. 그래서 regular file은 이렇게 표현됩니다. 이 형태에서 file의 block은 start address와 start address로 표현됩니다. 죄송합니다, 이것은 size여야 합니다. 미안합니다. 그래서 file의 beginning에는 start address가 있고, 이것은 size입니다.

이런 종류의 representation에서는 file에 속한 data가 disk 위의 start address와 disk에서 차지하는 data의 size로 표현됩니다. 만약 3 megabytes를 차지한다면 file size는 3 megabytes가 됩니다. 만약 movie file이고 file size가 1 gigabyte에 해당한다면, 이 file은 consecutive한 1 gigabyte의 data blocks를, consecutive하게 차지합니다. 이것이 매우 중요합니다.

그래서 이것이 Pintos가 file을 표현하는 방식입니다. 이것이 current form입니다.

두 번째는 directory입니다. directory는 file들의 set입니다. 여러분 모두 directory라는 term에는 매우 익숙하다고 생각하지만, 아마 정확한 directory의 definition은 모를 수 있습니다. directory는 file name과 inode number pair의 set입니다. 그래서 directory는 file name과 inode number pair입니다. 네, 이것이 directory의 definition입니다.

하지만 Pintos에서 directory 자체도 file입니다. 이것이 UNIX file system에서 directory를 표현하는 standard way입니다. UNIX와 동일하게 Pintos는 directory를 file로 정의합니다. 이는 directory가 자기 own inode를 가진다는 뜻입니다. inode는 data block location을 가리키는 pointer를 가집니다.

regular file과 다르게 directory file은 file name과 inode pair의 array를 포함합니다. 각 pair는 file name과 inode number로 표현됩니다. 각 entry는 directory 안의 entry라고 부르며, 이것을 directory entry라고 합니다.

세 번째로 알아두면 좋은 concept는 bitmap입니다. bitmap은 bits의 array입니다. 매우 쉽습니다. 0, 0, 1, 0, 0, 1, 0, 0, 이런 식입니다. 이것이 bitmap입니다. 이것이 왜 그리고 어떻게 이 operating system에 적용되는지는 설명할 것입니다.

어쨌든 Pintos에서 bitmap도 file로 표현됩니다. file로 표현된다는 것은 bitmap이 자기 own inode를 가진다는 뜻이고, inode는 bitmap의 location을 가지거나 포함합니다.

좋습니다. 이것이 Pintos operating system과 Pintos file system의 매우 대략적인 explanation과 basic concept입니다.

## Pintos File System Layout

넘어가겠습니다. 이것은 Pintos의 file system layout입니다.

현재 여러분은 Pintos file system을 8 megabytes file-system size로 정의했을 것이라고 생각해 봅시다. 8 megabytes는 꽤 작고, 매우 매우 작은 file-system partition입니다.

Pintos에서 block size는 512 bytes입니다. modern operating system에서 block이라는 term은 disk에 대한 I/O의 unit입니다. 이것이 block의 size라고 부르는 것입니다. Pintos operating system에서 block size는 512 bytes에 해당합니다. 요즘에는 보통 block size가 4 kilobytes입니다.

file system partition이 8 megabytes이므로, 이 file-system partition에는 16 thousand blocks가 있습니다. 이것이 전체 file-system partition layout입니다. block number 0부터 number 16,383까지 16,000 blocks가 있습니다. 이것이 마지막 것입니다.

모든 block은 index로 locate됩니다. 좋습니다. 그것이 block 0부터 block 16,383까지의 basic size입니다. 아주 많은 blocks입니다.

이것들이 Pintos file-system partition의 detailed layout입니다. 기억해 두면 좋은 한 가지는 Pintos에서 on-disk inode가 512 bytes 크기라는 점입니다. 매우 큽니다. 물론 inode는 512 bytes space를 전부 사용하지 않습니다. 512-byte block의 일부 fraction만 inode의 information을 담는 데 사용됩니다.

어쨌든 첫 번째 block은 bitmap file에 대한 inode를 포함합니다. bitmap이 무엇인지는 곧 나중에 설명하겠습니다. 두 번째 block, 즉 block 1은 root directory에 대한 inode를 포함합니다. block 2부터 block 5까지는 block bitmap을 포함합니다.

좋습니다. block 0은 bitmap file에 대한 inode를 포함하고, 이것들이 bitmap file의 actual contents입니다. 그래서 block 0은 inode를 포함하고, 이처럼 bitmap file의 start address를 포함해야 합니다. block bitmap은 block 2부터 block 5까지 네 개 block으로 구성됩니다.

그 다음 block number 6은 directory의 contents입니다. root directory 안에 reside하는 file들에 대한 inode와 file-name pair를 포함합니다. 그래서 이것은 inode number와 file name을 말합니다. 이것이 이 part의 detailed structure입니다.

그 다음에는 많은 data blocks가 옵니다.

## Bitmap Size

좋습니다. bitmap이 무엇인지 설명하겠습니다. bitmap은 당연히 bits의 array이고, 각 bit는 associated block이 사용 중인지 아닌지를 나타냅니다.

네 개의 blocks가 있고, 각 block은 512 bytes이며, 각 byte는 8 bits입니다. 그래서 file-system partition 안의 각 block이 사용 중인지 아닌지를 표현하려면 16,384 bits를 allocate해야 합니다.

이 file-system partition에는 16,384 blocks가 있으므로, 각 block이 사용 중인지 아닌지를 표현하기 위해 이만큼의 bit를 allocate해야 합니다. 그래서 bitmap을 위해 네 개 block을 allocate합니다.

하지만 file-system partition을 8 megabytes에서 두 배로, 예를 들어 32 megabytes로 늘린다면, 즉 file-system partition size를 8 megabytes에서 32 megabytes로 증가시킨다면, 당연히 bitmap에는 4개가 아니라 16개 block을 allocate해야 합니다. block bitmap에 사용되는 block 수를 네 배로 늘려야 하기 때문입니다.

그래서 이것이 Pintos의 file-system layout입니다.

## Block 0 and Root Directory Contents

이것이 details입니다. 첫 번째 block은 bitmap을 위한 inode였고, 두 번째 block은 root directory를 위한 inode였습니다. 2부터 5까지는 bitmap이었고, 이것은 root directory였습니다. 좋습니다. 그리고 data blocks가 있습니다. 이것은 plain data blocks입니다.

block number 0의 contents를 보겠습니다. 이것은 bitmap에 대한 inode block입니다. Pintos에서 각 file block은 start address와 length로 표현됩니다. 좋습니다. 그래서 이것은 contents의 start address가 block number 2이고, file의 length가 2048 bytes라고 말합니다. 이것은 unit bytes입니다.

그 다음 magic number가 옵니다. 알다시피 이것은 data structure design의 일부입니다. 125 integers의 integer array는 이 data structure에서 unused 상태입니다. 또한 이 session에서는 이것이 4 bytes라고 가정합시다. 이것은 4 bytes이고, 125 times 4, 즉 500 bytes가 unused 상태입니다.

좋습니다. 이것이 Pintos file system의 design입니다. 만약 이것을 modify할 시간이 있고, 더 보기 좋게 만들 시간이 있다면, 이 file-system partition을 modify해도 됩니다. 여러분은 분명히 그렇게 할 수 있을 것입니다.

좋습니다. 그러면 root directory를 보겠습니다. start address는 6입니다. root directory file의 data block은 block number 6에서 시작하고, file의 size는 320 bytes입니다. 그래서 block 안에 들어갑니다. block size가 512 bytes이기 때문입니다. 그리고 unused space의 huge space가 옵니다.

좋습니다. 이것이 inode structure입니다.

그 다음 root directory의 contents를 보겠습니다. 앞에서 다루었듯이 directory는 array, 아니 죄송합니다, directory는 file name과 inode의 set입니다. file name과 inode입니다.

Pintos data structure에서 directory entry는 file name과 inode의 array이고, file name의 length는 maximum 14 bytes로 고정되어 있습니다. 이것이 Pintos가 file name을 정의하는 방식입니다. Pintos에서 file name은 14 characters를 exceed할 수 없습니다. 하지만 modern operating system에서는 file name의 length가 virtually infinite입니다. 100-character file name이나 200-character file name을 사용할 수 있습니다.

어쨌든 이것이 Pintos가 directory를 표현하는 방식입니다.

block number 7부터 data block이 옵니다. data block이라는 것은 block number 7부터 16,000까지 시작하는 blocks를 사용할 수 있다는 뜻입니다. 그 blocks를 inode로 사용할 수도 있고, inode에 속하는 data block으로 사용할 수도 있습니다.

이 file-system layout에서 inode number 7, block number 7은 inode block을 포함합니다. file의 name은 모릅니다. file의 name이 무엇인지는 모릅니다. 하지만 그 point로는 나중에 돌아오겠습니다.

어쨌든 inode data block은 8에서 시작하고, file의 length는 1024입니다. 그래서 file의 length는 two blocks이고, 8에서 시작합니다. 이것은 block number 7에 저장된 inode block이 가리키는 file block에 대한 data block입니다.

또 다른 block이 있습니다. block number 10은 inode block을 포함하고, 그것의 data block은 11에서 시작합니다. 이것은 2048 bytes로 구성되어 있으며, 네 개 block에 해당합니다. 그래서 11, 12가 되고, 13과 14에 더 space가 있다면 이 file은 four blocks로 구성됩니다.

좋습니다. block number 10에 저장된 inode는 block number 11을 가리키고, file size에 대한 information을 포함하며, file size는 여기처럼 2048입니다.

## File Names Are in Directory Entries

좋습니다. root directory structure로 돌아가겠습니다.

root directory의 첫 번째 entry는 file의 name이 `myfile`이고 inode number가 number 7이라고 말합니다. 그래서 이것은 inode number이지만, 실제로는 associated inode의 location을 나타냅니다. file name은 `myfile`이고, associated inode는 block number 7에 저장되어 있습니다.

두 번째 entry는 `file.c`입니다. 이것은 file name이고, inode number는 10입니다. 하지만 이것은 inode location을 나타냅니다. `file.c`에 대한 inode는 inode number 10에 저장되어 있습니다.

여기에는 매우, 매우 interesting한 phenomenon이 있습니다. inode에서 보듯이 file name이 없습니다. inode를 보면 file name field가 없습니다. 없습니다.

알고 있나요? modern operating system, modern file system에서 file name은 file attributes의 일부가 아닙니다. 우리 human beings에게는 file name이라고 부르는 string으로 file을 인식합니다. 하지만 computer systems point of view에서는 file name은 file 자체와 아무 관련이 없습니다. 이것은 file attribute의 일부가 아닙니다.

directory data structure가 file name이라고 부르는 character string을 그 inode와 연결합니다. 이것이 modern file system의 중요하고 interesting한 characteristic입니다.

## In-Memory Inode

좋습니다. 그래서 inode에는 두 type이 있습니다. 첫 번째는 in-memory inode이고, 두 번째는 on-disk inode입니다.

inode는 disk 위의 file을 나타냅니다. 좋습니다. 먼저 in-memory inode라고 부르는 inode를 보겠습니다. 때로는 이것을 in-core inode라고 부릅니다.

Pintos에서 in-memory inode에 대한 data structure의 이름은 `struct inode`입니다. in-memory inode는 disk 위의 on-disk inode의 address를 포함합니다. inode가 저장된 block number를 나타냅니다. 그리고 disk inode를 포함합니다. 그게 전부입니다. 그리고 flag를 포함하는데, file을 delete할지 아닌지에 대한 flag입니다.

actual data structure를 보겠습니다. 가장 쉬운 part인 sector는 disk 위 inode의 location입니다. 그래서 이 part는 끝났습니다. 그리고 마지막 part는 on-disk inode에 대한 data structure입니다.

앞서 말했듯이 in-memory inode는 on-disk inode의 superset입니다. in-memory inode, 또는 in-core inode는 on-disk inode를 품고 있습니다. 이것이 on-disk inode입니다. 이것은 disk 위 inode의 location을 나타내는 sector를 포함하고, file이 deleted되었는지 아닌지를 나타내는 `removed` flag를 포함합니다.

물론 file은 deleted될 수 있지만, operating system은 file이 deleted될 때 in-memory inode를 immediately deallocate하지 않습니다. 보통 operating system은 asynchronous manner로 in-memory inode들을 deallocate합니다. 그래서 file을 delete하면 operating system은 이 file, 이 data structure, in-memory inode가 deallocated되어야 한다고 mark만 하고, 나중에 어느 시점에 그것을 수행합니다. 이것이 이것의 용도입니다.

앞에서 다루었듯이 어떤 file들은 writable하지 않습니다. 예를 들어 executable file, 또는 disk에 loaded되고 있는 file들은 operating system이 그것을 modify하는 동안 modified되어서는 안 됩니다. 그래서 Pintos operating system은 in-memory inode에 file이 writable할 수 있는지 아닌지를 나타내는 field를 정의합니다.

그래서 text는 `deny_write_cnt`입니다. 이것이 flag가 아니라 count인 이유는 file에 접근하는 multiple processes가 있기 때문입니다. 그런 경우에는 어떤 process도 file에 접근하지 않을 때까지 file이 modified될 수 없습니다. 이런 이유로 Pintos file system은 deny write flag가 아니라 deny write count를 정의합니다.

마지막으로 open count가 있습니다. 이것은 주어진 file을 open한 process의 수, 또는 open system call의 수를 나타냅니다. 이것이 in-memory inode의 structure입니다.

## On-Disk Inode

좋습니다. 다음 part는 on-disk inode라고 부르는 더 essential한 data structure입니다. 이것은 disk 위의 file을 나타냅니다. data structure의 이름은 `struct inode_disk`입니다.

이것은 매우 큽니다. 512 bytes입니다. 매우 안타깝습니다. 물론 불행히도 512 bytes 중 500 bytes는 사용되지 않습니다.

좋지 않습니다. 하지만 이것이 Pintos가 inode를 정의하는 방식입니다. 시간이 있다면 이 data structure를 modify하고 더 efficient하게 만드는 데 시간을 써 보세요. 한번 해 보세요. 매우 쉽습니다.

어쨌든 중요한 part는 Pintos operating system이 file을 어떻게 정의하는가입니다. Pintos operating system에서 file system은 file을 하나의 큰 chunk의 single block으로 표현합니다. 이것은 start address와 size로 pointed됩니다. 이것이 Pintos file system이 file을 정의하는 방식입니다.

## Directory Object and Directory Entry

좋습니다. 이것은 directory object의 data structure입니다.

directory object는 directory file에 대한 data block의 format입니다. 그래서 directory file은 자기 own inode를 가집니다. 자기 own inode는 data block의 start address와 그 size를 가리키는 pointer를 포함하고, 무언가를 포함할 것입니다.

이것은 directory block을 나타내는 in-memory data structure입니다. `struct dir`은 associated inode를 가리키는 pointer와 position을 포함합니다. position은 read 또는 write할 next directory entry를 정의합니다. 그래서 이것은 open directory를 나타냅니다.

두 field가 있습니다. inode, 즉 associated in-memory inode를 가리키는 pointer, 그리고 `pos`, 즉 read하고 write할 next directory entry의 position입니다. 이것이 directory object입니다.

directory entry를 설명하겠습니다. directory data block은 directory entries의 array로 구성되며, 이것은 앞에서 설명했습니다. 각 directory entry는 실제로 file name 또는 inode number pair입니다. 그렇죠?

그래서 `inode_sector`는 associated inode의 sector number입니다. 그 다음 file name이 옵니다. 그 slot은 use 중이 아닐 수도 있습니다. 그런 경우에는 current slot이 사용 중인지 아닌지를 나타내는 flag가 필요합니다.

그래서 directory entry에 대한 actual data structure는 `inode_sector`, maximum number가 14 plus 1인 character string, 그리고 주어진 directory slot이 사용 중인지 아닌지를 나타내는 flag로 표현됩니다. 보시다시피 이것이 directory entry에 대한 data structure입니다.

이 경우, search할 때마다, 주어진 file name에 대한 특정 inode를 찾아야 할 때마다 matching filename을 찾기 위해 directory block을 scan해야 합니다. 음, file이 10개 또는 15개라면 10 elements나 15 elements를 linearly scanning하는 것이 reasonable할 수 있습니다. 하지만 directory가 한 directory 안에 300,000 files 정도를 포함한다면 어떨까요? 그러면 sorted되어 있지 않거나 radix tree, B+ tree, red-black tree 같은 certain search structure로 organized되어 있지 않다면, 300,000 elements의 array를 linearly scanning하는 것은 substantial amount of search를 요구합니다. 매우 오래 걸립니다.

좋습니다. 어쨌든 Pintos에서 directory는 directory entries의 array이고, directory entries는 sorted되어 있지 않습니다.

## Free Map and Struct File

좋습니다. 다음 data structure는 block bitmap입니다. block bitmap은 file-system partition 안의 주어진 block이 사용 중인지 아닌지를 나타냅니다.

data structure name은 free map입니다. 이것은 file-system partition 안의 blocks의 status를 나타내기 위한 bitmap입니다. bitmap은 file로 저장되는데, 이는 자기 own inode를 가진다는 뜻입니다. 이것은 두 fields를 가집니다. 전체 file system 안의 disk blocks 수를 bit count로 가지고, actual bit array를 가집니다.

또 다른 important data structure인 `struct file`이 있습니다. 이것은 file이 open될 때 created됩니다. 기억하세요. closed file에 대해서는 이 data structure가 created되지 않습니다.

그래서 이 `struct file`은 file이 opened될 때 allocated되고 created됩니다. 이것은 inode를 가리키는 pointer를 포함하고, `struct file`의 가장 중요한 attribute는 position입니다. 이것은 read와 write operation이 적용되어야 하는 file의 position을 나타냅니다. 그리고 file이 writable한지 아닌지를 나타내는 field를 가집니다.

## Three Things in Project 4

그래서 이 project에서 우리는 기본적으로 세 가지를 해야 합니다. 첫 번째는 buffer cache 구현이고, 두 번째는 file-system file abstraction을 indexed and extensible하게 만드는 것이며, 세 번째는 subdirectories를 구현하는 것입니다.

buffer cache의 경우, buffer cache의 purpose는 memory의 일부를 disk로 사용하는 것입니다. 이것은 virtual memory와 opposite concept입니다. virtual memory에서는 disk의 일부를 memory로 사용합니다. 이제 buffer cache에서는 반대입니다. memory의 일부를 disk로 사용합니다.

좋습니다. 그래서 buffer cache를 위해 buffer cache를 allocate합니다. 64 disk blocks를 accommodate하기 위해 physical pages를 allocate할 것입니다. 우리는 data blocks를 이 buffer cache에 cache할 것입니다. data block을 read하거나 write할 때 그것을 buffer cache에 저장할 것이고, block에 접근하는 일이 끝나면 때로는 modified data blocks를 disk space에 다시 save해야 합니다. 또는 file system이 shut down될 때 그렇게 해야 합니다.

file의 경우, current Pintos는 file을 single extent, 즉 start address와 size를 포함하는 single consecutive block으로 나타냅니다. 그래서 Pintos inode에는 file blocks의 start address와 size라는 두 fields가 있습니다.

하지만 block을 extend하고 싶고, file을 extend하고 싶은데, file 바로 next location이 이미 다른 file, 예를 들어 file B에 의해 occupied되어 있다면 어떻게 될까요? 그러면 Pintos file system이 이 file을 extend할 방법이 없습니다. 그러면 free space를 찾아야 하고, extended file을 accommodate할 수 있는 free space를 찾아야 하며, entire file을 migrate해야 합니다. 이 copy에는 huge amount of time이 소모됩니다.

그래서 inode 안에 block pointers를 구현할 것입니다. file에 속한 file blocks를 표현하는 방법에는 variety of ways가 있지만, 지금은 UNIX-like file structure를 사용할 것이고, file을 위한 hierarchical space를 구현할 것입니다.
