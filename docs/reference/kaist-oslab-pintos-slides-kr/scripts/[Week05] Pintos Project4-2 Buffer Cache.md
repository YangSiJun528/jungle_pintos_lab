# [Week05] Pintos Project4-2 Buffer Cache

Source: https://youtu.be/AydN8n7PCaY?si=6c5_Lk4gVNU_LYk_

## Buffer Cache Overview

project number four에서는 file system을 위해 세 가지를 구현할 것입니다. 첫 번째는 buffer cache이고, 두 번째는 files를 indexed and extensible하게 만드는 것이며, 세 번째는 subdirectories concept를 구현하는 것입니다.

제가 설명할 첫 번째 것은 buffer cache입니다.

buffer cache의 idea는 simple합니다. memory의 일부를 disk로 사용합니다. 이것은 virtual memory와 swap space의 opposite concept입니다.

이것이 memory이고 이것이 disk라고 합시다. virtual memory는 disk의 일부를 memory로 사용하는 concept입니다. 이런 식입니다. 이 part는 memory에 mapped되고, 이 part는 disk에 mapped되며, 이 portion은 swap space라고 불립니다. 이것이 virtual memory입니다.

buffer cache에서는 concept가 반대입니다. 이것이 buffer cache라고 합시다. memory의 일부를 disk로 사용합니다. 이런 식입니다. 좋습니다. 그래서 memory의 일부를 disk로 사용합니다. 그것이 buffer cache의 큰 concept입니다. disk의 일부가 memory에 mapped됩니다. 그것이 buffer cache입니다.

buffer cache는 consecutive physical pages이고, system이 start될 때 또는 file system이 mounted될 때 initialized됩니다. 그래서 file system을 unmount하고 다시 mount하면 buffer cache 안의 모든 contents가 reset됩니다.

current Pintos에는 disk I/O를 위한 cache가 없습니다. 하지만 reality에서는 대부분의 operating system이 disk I/O를 위한 buffer cache를 가지고 있습니다. 그래서 file blocks를 physical memory에 cache할 수 있도록 file system을 modify할 것입니다.

이 project에서는 memory에 cache될 64 file pages를 allocate할 것입니다.

## What to Implement

이것이 구현 방식입니다. current Pintos는 모든 read/write request마다 storage에 access합니다. 그래서 user I/O가 buffer cache를 통해 performed되도록 바꾸어야 하고, 여기에 buffer cache layer를 add할 것입니다.

이것은 buffer cache를 implement하기 전이고, 이것은 buffer cache를 implement한 후입니다.

이것이 해야 할 일들의 list입니다. 첫 번째는 당연히 buffer cache를 위한 data structure를 define하는 것입니다. 그리고 buffer cache를 allocate하고 initialize하는 routine을 write해야 합니다. 그 다음 read와 write routine이 data를 reading and writing할 때 buffer cache를 utilize하도록 modify해야 합니다.

그리고 물론 read/write routine을 바꿔서 cache miss가 발생하면, 즉 buffer cache에서 data blocks를 read하고 싶은데 그 block이 buffer cache에 존재하지 않으면, disk에서 read해야 합니다. 좋습니다. 그래서 cache miss를 handle하는 routine을 implement해야 합니다.

또 다른 중요한 routine도 write해야 합니다. new block을 buffer cache로 bring in하고 싶은데 buffer cache가 이미 full일 수 있습니다. 그러면 new block을 bring in하기 위한 room을 만들어야 합니다. 그런 경우 buffer cache entry 중 하나를 evict해서 free로 만들어야 합니다. 그래서 buffer cache replacement algorithm을 implement해야 합니다.

그리고 dirty buffer cache를 write하는 routine을 write해야 합니다. block이 disk에서 buffer cache로 read된 뒤 application이 buffer cache의 contents를 changed했을 수 있습니다. 그런 경우 dirty page를 그것이 read된 곳으로 write back해야 합니다. 그래서 dirty buffer cache를 disk에 synchronize하는 routine을 write해야 합니다.

이것이 해야 할 일들의 list입니다. 매우 simple하고, 매우 fun합니다.

## Buffer Cache Data Structure

좋습니다. 가장 먼저 해야 할 일은 buffer cache를 위한 data structure를 define하는 것입니다.

buffer cache는 memory 안의 page입니다. page entry를 buffer cache로 사용하려면 이 page 안의 contents를 나타내는 어떤 metadata structure를 define해야 합니다.

dirty flag를 포함해야 합니다. dirty flag는 주어진 content가 disk에서 read된 이후 changed되었는지 여부를 나타냅니다.

entry가 being used인지 아닌지를 indicating하는 이런 종류의 flag도 header에 있습니다.

물론 access flag를 introduce해야 합니다. 이것은 entry가 recently accessed되었는지 아닌지를 나타냅니다. 이 access flag는 operating system이 new one을 위한 room을 만들기 위해 buffer cache entries 중 하나를 select해야 할 때 victim selection에 사용됩니다.

hot blocks와 cold blocks를 distinguish하는 mechanism이 있어야 합니다. hot blocks는 actively accessed되고 있는 buffer cache entry를 의미하고, cold buffer cache entry는 recently used되지 않은 entry입니다.

그래서 block의 hotness를 precisely identify하는 어떤 mechanism이 있어야 합니다. 이것은 operating system이 recently used되지 않은 data blocks를 select하게 하기 위한 것입니다.

buffer cache entries 중 하나를 kick out해야 할 때, recently used되지 않은 buffer cache entry를 choose해야 performance가 그 entries를 kick out하는 것에 의해 affected되지 않습니다. 이것이 중요합니다.

또한 on-disk location을 가져야 하고, 당연히 associated cache entry의 virtual address도 가져야 합니다.

buffer cache를 위해 64 pages를 allocate할 것입니다. 그래서 data structure의 이름을 buffer head라고 합시다. system이 start될 때 64 buffer heads를 allocate해야 합니다.

64 data structures를 accommodate할 수 있는 data structure는 많습니다. array, list, hash table을 사용할 수 있습니다. 여기서는 table을 사용합니다. 이것은 64 buffer heads의 table이고, 각 buffer head는 virtual memory 안의 actual location을 가리키는 pointer를 가집니다.

이것은 buffer cache에 대한 diagram입니다. disk에서 buffer cache로 disk block을 reading하는 act는 caching이라고 부릅니다. 이것이 caching입니다.

buffer cache의 일부가 disk에서 read된 후 updated되면, buffer cache의 contents를 그것이 belong하는 place로 다시 synchronize해야 합니다. 이것을 flush라고 부릅니다.

좋습니다. 이것이 buffer cache data structure에 대한 entire diagram이고, 이것이 여러분이 implement해야 할 것입니다.

application program이 certain block을 read하고 싶으면, 먼저 buffer cache table을 search해서 buffer cache의 location을 identify해야 합니다.

좋습니다. 다음 step입니다. buffer head를 위한 data structure를 define할 수 있도록 필요한 설명은 거의 다 했습니다. 그래서 buffer head data structure를 implementing하는 데 어려움이 없어야 합니다.

## Allocating and Initializing Buffer Cache

두 번째 issue는 buffer cache를 allocating하고 initializing하는 것입니다. 물론 이것은 system boot time과 file system이 mounted될 때 수행되어야 합니다.

64 file system blocks를 위한 space가 필요합니다. block size는 512 bytes이므로 64 file blocks를 accommodate하려면 32 kilobytes가 필요합니다. 물론 buffer heads, 64 buffer heads를 위한 memory도 allocate해야 합니다. 이것은 buffer head의 size times 64입니다. 이것이 64 buffer entries를 defining하기 위해 allocate해야 하는 memory의 amount입니다.

이것은 file system initialization time에 수행됩니다. 그래서 여기 code가 있습니다. 꽤 easy합니다.

좋습니다. 쉽습니다.

## Current Pintos Read

이제 current Pintos에서 read와 write를 설명하겠습니다. 이것은 `file_read` function이고, `inode_read_at`이라는 function을 call합니다.

read system call은 parameter를 받습니다. 물론 file descriptor와 read할 data blocks의 amount이고, 그 다음 operating system은 disk에서 memory로 certain amount of data blocks를 read합니다.

current Pintos에서 read는 이렇게 implemented되어 있습니다. size라는 variable이 있습니다. disk에서 read할 때마다 512 bytes를 read합니다. 그래서 이 loop를 iterate하고, `file_read` operation의 parameter로 specified된 data amount를 read할 때까지 read하고 read하고 read합니다. 매번 size는 512 bytes씩 decreases합니다. 결국 size는 zero 또는 negative가 됩니다.

좋습니다. full sector를 read하고, disk에서 `block_read`하고, remaining read size를 compute합니다. read size가 still greater than 0이면 full sector를 again read합니다.

하지만 예를 들어 1500 bytes를 read하고 싶을 수 있습니다. 그러면 먼저 512 bytes를 read하고, 다시 512 bytes를 read합니다. 그러면 합쳐서 1024 bytes입니다. remaining part, 476 bytes는 disk에서 read되어야 하지만, single block은 아닙니다. 그렇죠? single block이 아닙니다. 그래서 그 part만을 위해 disk에서 `block_read`를 call해서는 안 됩니다.

그 다음 bounce buffer를 allocate하고 partial read를 perform합니다. partial read는, disk에서 entire block, 즉 512 bytes를 read하더라도 application은 disk에서 476 bytes만 read하라고 요청했다는 뜻입니다. 그래서 bounce buffer로 512 bytes를 read하고, bounce buffer에서 read가 지정한 buffer로 476 bytes만 copy합니다. 이것이 476 bytes입니다. 이것이 partial read를 perform하는 방식입니다.

## Reading Through Buffer Cache

좋습니다. 이제 이 part를 implement해야 합니다. 물론 entire diagram은 훨씬 complicated해 보이지만 두려워하지 마세요. 쉽고 매우 quickly 할 수 있어야 합니다.

`inode_read_at` function을 calling한 뒤, disk에서 directly reading하는 대신 buffer cache를 read합니다. 그래서 buffer cache에서 read를 perform합니다.

먼저 해야 할 일은 buffer head를 find하는 것입니다. 존재한다면 buffer cache에서 buffer로 data를 read하고, 그 다음 buffer head를 update합니다. 여기서 updated되는 것은 아마 access bit, access flag일 것입니다. 이 buffer cache가 just accessed되었음을 나타냅니다. 좋습니다. 이것이 story의 lucky part입니다.

하지만 life does not always go as easy as you want.

첫 번째 issue는 entry가 존재하지 않으면 어떻게 하는가입니다. 그러면 disk block을 disk에서 buffer cache로 bring in, read해야 하고, read를 again perform해야 합니다.

그 경우 entry가 존재하지 않으면 disk block을 disk에서 buffer cache로 bring in해야 합니다. cache가 full이 아니라면 더 좋습니다. disk에서 buffer cache로 blocks를 read하고 다시 read하면 됩니다. buffer head에서 empty entry를 select하고, disk blocks를 bring in한 다음 read를 perform합니다. 좋습니다.

하지만 worst part는 buffer cache가 entirely full인 경우입니다.

cache가 full이면 victim entry를 select합니다. new disk blocks를 bring하기 위한 room을 만들기 위해 kick out할 victim을 select해야 합니다. victim entry를 select합니다. victim entry가 disk에서 memory로 brought된 이후 modified되었다면 그것을 dirty라고 말합니다. 그러면 disk에 save해야 합니다. victim entry를 disk에 flush해야 합니다.

그 buffer cache entry를 disk에 flushing한 뒤에는 victim entry를 release하고 free로 만들어야 합니다. victim entry가 dirty가 아니라면 flush를 perform할 필요가 없습니다. victim entry를 release하는 것으로 바로 갑니다.

다시 말해 이제 buffer cache entry 안에 free block이 available하므로, operating system은 disk에서 memory로 read를 perform할 수 있습니다.

좋습니다. 그래서 disk read를 buffer cache read로 modify해야 합니다. 바꿔야 할 function은 `inode_read_at`입니다. `block_read`를 modify해야 합니다. file을 reading할 때 data를 disk가 아니라 buffer cache에서 read하도록 read를 modify합니다. 좋습니다. 그래서 이 read part를 modify해야 합니다.

## Current Pintos Write

write도 similar fashion으로 changed되어야 합니다. write도 같습니다. while loop에서 iterate합니다. `n` bytes를 write하고 싶으면, Pintos operating system은 each iteration마다 한 block씩 write합니다. write를 perform할 때마다 size를 512 bytes씩 decrease합니다. 그 다음 remaining write size를 calculate하고, 다시 하고, 다시 하고, 다시 합니다.

size가 zero가 되면 write는 done이고, bounce buffer를 release합니다. 하지만 write할 remaining amount of data가 sector보다 less than인 경우, 이것이 important and tricky part입니다. 우리는 이것을 partial write라고 부릅니다.

사실 partial write는 blind write보다 더 많은 time이 걸립니다. 우리는 이것을 blind write라고 부릅니다. blind write는 full sector를 write한다는 뜻이므로 disk에서 disk block을 read할 필요가 없습니다. entire sector를 write할 때는 그냥 disk에 write하면 됩니다.

full block을 write하고 싶으면 disk에 write합니다. 하지만 block의 part를 write하고 싶으면, 먼저 contents를 disk에서 memory로 read해야 합니다. 그 다음 memory part를 write하고 싶은 data blocks로 fill하고, 그 다음 disk에 write합니다.

그래서 partial write는 더 expensive합니다. bounce buffer를 allocate합니다. disk에서 bounce buffer로 data blocks를 read해야 하고, 그 다음 bounce buffer에 partial write를 perform하고, 그 다음 bounce buffer를 disk에 write해야 합니다. 그래서 partial write는 더 expensive합니다.

## Writing Through Buffer Cache

buffer cache를 통한 writing도 similar합니다. buffer cache 안에 write해야 합니다.

buffer head를 find합니다. 그 blocks가 buffer cache 안에 already exist한다면, 존재한다면 lucky합니다. 그러면 buffer cache를 update하고 끝입니다.

하지만 entry가 존재하지 않으면 그것을 buffer cache에 write해야 합니다. 그 경우 cache가 full이 아니면 lucky합니다. empty buffer head를 find하고, block을 disk가 아니라 buffer cache에 write한 다음 return합니다.

좋습니다. interesting part가 있습니다. disk에서 buffer cache로 data를 `block_read`하는 이 part입니다. 여기의 이 `block_read`는 partial write에만 required됩니다. full block write를 perform하거나 blind write를 perform한다면 disk block을 buffer cache에서 read할 필요가 없습니다.

cache가 full이면 victim entry를 select해야 합니다. dirty이면 victim entry를 disk에 flush해야 합니다. 이것은 before와 같습니다. 그 다음 이 block을 release합니다. dirty가 아니면 buffer head에서 victim entry를 release합니다.

write를 modifying할 때는 disk write를 buffer cache에 write하는 것으로 modify해야 합니다. buffer cache에 write합니다. file을 writing할 때 disk가 아니라 buffer cache에 data를 write하도록 modify합니다. 이것이 해야 할 일이고, 이 part, `block_write`를 modify해야 합니다.

좋습니다. easy할 것입니다. 끝났습니다. 좋습니다.

## Synchronizing Dirty Buffer Cache Entries

남아 있는 important thing 중 하나는 dirty buffer cache entry를 synchronizing하는 것입니다.

newly incoming disk block을 accommodate하기 위해 buffer cache entry를 release할 때, existing buffer cache content가 dirty인지 아닌지 check해야 합니다. existing buffer cache entry가 dirty라면 disk에 write해야 합니다. 이것을 synchronization activity라고 부릅니다. synchronize입니다.

dirty buffer cache entries를 write합니다. 이것은 먼저 buffer cache entry가 evicted될 때, 또는 file system이 unmounted될 때 발생합니다. shutdown은 unmounted를 의미합니다. `filesys_done`이라는 function이 있고, 여기에 code를 write해야 합니다. 좋습니다. easy할 것이므로 할 수 있어야 합니다.

dirty buffer cache entry를 disk에 write하고 싶은 another situation이 있습니다. periodically입니다. 예를 들어 five-second interval입니다. 이 경우 timer interrupt를 사용합니다.

## File Read and File Write Path

좋습니다. Pintos에서 read/write의 details를 설명하겠습니다.

file을 reading하는 것은 `file_read` function에 의해 performed됩니다. 이것은 three parameters를 포함합니다. 이것은 file object를 가리키는 pointer이고, 이것은 data blocks를 read해 넣고 싶은 memory area를 가리키는 pointer이며, 이것은 read하고 싶은 data blocks의 amount, size입니다.

data structure `struct file`은 in-core 또는 in-memory inode를 가리키는 pointer를 포함합니다. in-memory inode에서 Pintos는 operating system에 의해 opened된 inode들의 linked list를 maintain합니다. 여기 inode list가 있고, 각 inode는 이런 form을 취합니다. in-memory inode 안에는 structure inode가 포함되고, inode disk는 file의 start address와 size를 포함합니다. 그래서 inode disk를 보면 data blocks의 location을 find할 수 있어야 합니다.

두 번째는 current Pintos의 write입니다. 다시 돌아가 보겠습니다. 이것은 write를 perform합니다. 네, 이것은 previous slide에서 다룬 data blocks이므로 skip하겠습니다.

`file_write`는 three parameters를 받습니다. first parameter는 file data structure, buffer, 그리고 size입니다. `file_write` 안에서는 `inode_write_at` function을 call하며, parameters를 받습니다. first parameter는 inode, second parameter는 buffer의 address, third parameter는 size, fourth parameter는 current offset입니다.

file에 write해야 하는 position을 pass합니다. 마지막으로 data blocks도 write하고, contents를 write한 직후 file position을 update해야 합니다. 그 다음 file에 written된 data의 amount를 return합니다.

## `inode_write_at()`

이것은 important function `inode_write_at`의 details입니다. 이것은 inode, buffer, size, offset이라는 four parameters를 받습니다. 여기 이것은 file이고 이것은 offset입니다.

`inode_write_at`에서는 size amount of data를 write합니다. 이것은 buffer입니다. 그래서 이 size를 여기에 write합니다. 이것이 `inode_write_at`이 작동하는 방식입니다.

write function의 details를 보겠습니다. 이것이 file이고, 이 position부터 이 amount까지 write하고 싶다고 합시다. 이것이 three and four blocks라고 합시다. 이것이 block 1, block 2, block 3, block 4, block 5라고 하고, block 5에서는 block의 half만 write한다고 합시다.

이것이 file이고, 이것이 file의 beginning이고, 이것이 offset이고, 이것이 file의 start address입니다. file을 write할 때 가장 먼저 해야 할 일은 offset을 disk 위의 actual location으로 change하는 것입니다. 이것이 offset을 real sector address로 translate하는 function입니다.

그 다음 data block을 write하고 싶은 location이 sector address와 aligned되어 있는지 check해야 합니다. disk에 writing할 때 모든 disk blocks가 sector address의 start에서 시작해야 한다는 것을 확인해야 합니다.

sector offset이 zero이고 write하고 싶은 chunk의 size가 sector size이면 happy하고, block write를 perform할 수 있습니다.

data blocks를 write하고 싶은데 data block이 sector address에서 start한다고 가정합시다. 그러면 size가 greater than zero인 동안 full block write를 repeatedly perform해야 합니다. 하지만 어느 point에서 chunk size가 block size보다 less than이면 partial write를 perform해야 합니다.

partial write는 block의 part를 writing할 때 bounce buffer가 필요하다는 뜻입니다. 먼저 data blocks를 disk에서 bounce buffer로 read해야 합니다. 그 다음 이 block을 disk에서 memory로 read해야 하고, bounce buffer의 contents를 write하고 싶은 fractional blocks와 merge해야 합니다.

그래서 disk가 있고, block의 part를 write하고 싶습니다. 이것은 write하고 싶은 contents입니다. 이것은 bounce buffer이고, 이것은 write하고 싶은 block입니다. 그러면 data block을 disk에서 bounce buffer로 read하고, 이 portion을 bounce buffer에 copy해서 bounce buffer 안의 contents의 part를 updating합니다. 이것이 first step이고, 이것이 second step입니다. 그 다음 bounce buffer의 entire contents를 disk에 write합니다. 이것이 disk block을 update하는 방식입니다.

쉽습니다. 그래서 다시 끝났습니다. 우리는 happy합니다. We are happy hackers. Happy hackers. 좋습니다.

## `byte_to_sector()`

좋습니다. 이것은 `byte_to_sector`라고 부르는 function입니다. inode를 받고 position을 받으며, 그 다음 given position의 sector address를 return합니다.

이것은 매우 simple합니다. inode는 file의 start address를 가지고 있습니다. 그 다음 position이 있습니다. position이 여기라면 sector size로 divide합니다. 여기 position은 file 안의 position입니다. 물론 position은 bytes입니다. 이것을 sector로 divide합니다. 이것은 file의 beginning에서 몇 sectors만큼 떨어져 있는지를 뜻합니다. 그 다음 이것을 file의 start sector number에 add하면, write하고 싶은 block의 sector address를 얻습니다.

쉽습니다.

## `file_read()` and `inode_read_at()`

다음으로 read를 설명하겠습니다. 이것은 current Pintos의 read입니다. disk에서 read하고, partial read이면 bounce buffer를 allocate하고 그 part를 read합니다.

이것은 같은 code입니다. 이것은 `file_read`의 code입니다. three parameters를 받습니다. 첫 번째는 file을 가리키는 pointer입니다. 이것은 memory chunk를 가리키는 pointer이고, 이 memory chunk는 operating system이 disk에서 data blocks를 read해 넣는 place입니다. 그 다음 read하고 싶은 data blocks의 amount가 size입니다. 그래서 이것은 file, buffer, size라는 three parameters를 포함합니다.

이것은 file이고, inode data structure를 가리키는 pointer를 포함합니다. file을 reading한 뒤 file position은 updated됩니다.

이것이 buffer cache에 대한 explanation의 last slide입니다. function은 `inode_read_at`입니다. 이것은 given file을 read합니다. inode, buffer, size, offset이라는 four parameters를 받습니다.

inode는 read하고 싶은 file을 나타내고, buffer는 file을 read해 넣고 싶은 memory address의 start입니다. 이것은 read하고 싶은 data blocks의 amount이고, 이것은 reading을 start할 file의 position입니다.

`inode_write_at`과 similar하게 iterate하고, each iteration마다 block을 read합니다. read를 perform할 때마다 block device에 read를 perform하고, 매번 sector index를 provide합니다. 그래서 이것이 read하고 싶은 sector입니다. full block을 reading한 뒤에는 `inode_read_at`의 end에서 partial read를 perform할 것입니다.

이것이 explanation의 end이고, project를 enjoy하기 바랍니다. Good luck. We are happy hackers.
