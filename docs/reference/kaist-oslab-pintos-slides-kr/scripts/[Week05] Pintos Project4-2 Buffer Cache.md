# [Week05] Pintos Project4-2 Buffer Cache

Source: https://youtu.be/AydN8n7PCaY?si=6c5_Lk4gVNU_LYk_

## Buffer Cache

첫 번째 Project 4 feature는 buffer cache입니다.

buffer cache의 idea는 memory 일부를 disk block의 cache로 사용하는 것입니다. 이는 virtual memory와 반대 방향입니다. virtual memory는 disk 일부를 memory의 확장처럼 사용할 수 있고, buffer cache는 memory 일부를 disk access의 cache처럼 사용합니다.

original Pintos file system에서 file read와 write는 block device로 직접 갑니다. buffer cache를 추가한 뒤에는 file-system read와 write가 cache layer를 거쳐야 합니다.

Project 4는 64 file-system blocks에 대한 cache를 요구합니다.

## Tasks

구현해야 할 일은 여러 가지입니다.

첫째, buffer cache entry를 위한 data structure를 정의합니다.

둘째, file system이 시작될 때 buffer cache를 allocate하고 initialize합니다.

셋째, read와 write routine이 disk를 직접 호출하지 않고 buffer cache를 사용하도록 수정합니다.

넷째, cache miss를 처리합니다. requested disk block이 cache에 없으면 Pintos는 disk에서 cache로 가져와야 합니다.

다섯째, replacement algorithm을 구현합니다. cache가 full이면 Pintos는 new block을 위한 room을 만들기 위해 cache entry 하나를 evict해야 합니다.

여섯째, dirty cache entry를 eviction 시점, file system shutdown 시점, 또는 periodically disk에 write back합니다.

## Buffer Cache Entry

buffer cache entry에는 data와 metadata가 모두 필요합니다.

data area는 cached disk sector 하나를 저장합니다. metadata는 그 data area가 현재 무엇을 담고 있는지 설명합니다.

유용한 metadata에는 dirty flag, in-use 또는 valid flag, accessed flag, on-disk sector number, cached sector를 저장하는 memory area pointer가 있습니다.

dirty flag는 cached contents가 disk에서 읽힌 뒤 수정되었다는 뜻입니다. Dirty entry는 discard되기 전에 write back되어야 합니다.

accessed flag는 entry가 최근 사용되었는지 기록합니다. replacement algorithm은 이 flag를 사용해 hot entry를 evict하지 않고 최근 사용되지 않은 entry를 선호할 수 있습니다.

## Allocation and Initialization

cache는 64 entries를 가집니다. Pintos file-system block은 각각 512 bytes이므로, 64 cached blocks를 위한 data area는 32 KB memory가 필요합니다.

system은 64 buffer cache entries에 대한 metadata도 필요합니다.

이 structure들은 file system initialization 때 initialize됩니다. file system이 unmount되고 다시 mount되면 buffer cache state는 reset됩니다.

## Original Read Path

original read path는 `file_read()`에서 시작합니다. 이는 inode, destination buffer, size, current file position을 넘겨 `inode_read_at()`을 호출합니다.

`inode_read_at()`은 file data를 sector 단위로 loop합니다. 각 sector에 대해 file offset을 `byte_to_sector()`로 disk sector로 변환합니다.

read가 full sector를 덮으면 original code는 `block_read()`를 직접 호출합니다.

read가 sector 일부만 덮으면 Pintos는 bounce buffer를 사용합니다. full sector를 bounce buffer로 읽고, requested bytes만 caller의 buffer로 copy합니다.

## Reading Through the Buffer Cache

buffer cache를 추가한 뒤에는 file system이 normal file read path에서 `block_read()`를 직접 호출하지 않아야 합니다. 대신 buffer cache에 sector를 요청해야 합니다.

cache hit이면 Pintos는 cache entry를 찾고, cached sector에서 requested bytes를 copy하며, entry를 accessed로 표시합니다.

cache miss이면 Pintos는 sector를 cache로 가져와야 합니다. empty cache entry가 있으면 그것을 사용합니다. cache가 full이면 victim entry를 선택해야 합니다.

victim entry가 dirty이면 재사용하기 전에 disk로 flush해야 합니다. 그런 다음 requested disk sector를 cache entry로 읽고, original read는 memory에서 계속 진행됩니다.

## Original Write Path

original write path는 `file_write()`에서 시작합니다. 이는 inode, source buffer, size, current file position을 넘겨 `inode_write_at()`을 호출합니다.

`inode_write_at()`도 sector 단위로 loop합니다.

write가 full sector를 덮으면 Pintos는 whole sector를 직접 쓸 수 있습니다.

write가 sector 일부만 덮으면 해당 sector의 다른 byte들을 보존해야 합니다. 따라서 original sector를 bounce buffer로 읽고, target range만 bounce buffer에서 overwrite한 뒤, full sector를 disk에 다시 씁니다.

partial write는 write 전에 read가 필요하므로 full-sector write보다 expensive합니다.

## Writing Through the Buffer Cache

buffer cache를 추가한 뒤에는 write가 disk에 직접 쓰지 않고 cached sector를 update해야 합니다.

cache hit이면 Pintos는 data를 cached sector로 copy하고, entry를 dirty로 표시하며, accessed로 표시합니다.

cache miss이면 Pintos는 그 sector를 위한 cache entry가 필요합니다. write가 partial-sector write이면 unchanged bytes를 보존해야 하므로 old sector contents를 먼저 cache로 load해야 합니다. full sector overwrite이면 old sector를 먼저 읽을 필요가 없습니다.

cache가 full이면 Pintos는 victim entry를 선택합니다. dirty victim은 재사용 전에 flush되어야 합니다. entry가 준비되면 Pintos는 cached contents를 update하고 entry를 dirty로 표시합니다.

## Flushing Dirty Entries

dirty cache entry는 결국 disk에 write back되어야 합니다.

flush point 중 하나는 eviction입니다. dirty cache entry가 victim으로 선택되면 file system은 해당 entry를 재사용하기 전에 write back해야 합니다.

다른 flush point는 file system shutdown입니다. Pintos에는 `filesys_done()` 같은 file system shutdown path가 있고, 여기서 dirty entries를 disk와 synchronize해야 합니다.

세 번째 가능한 flush point는 periodic write-behind입니다. timer-based routine이 periodically cache를 scan하고 dirty entries를 disk에 write back할 수 있습니다.

중요한 rule은 modified cached data를 그냥 discard해서는 안 된다는 것입니다.

## `byte_to_sector()`

read와 write operation은 모두 file offset을 disk sector로 변환해야 합니다.

original Pintos file system에서 `byte_to_sector()`는 file의 start sector에 offset divided by sector size를 더해 sector를 계산할 수 있습니다.

indexed file이 구현된 뒤에는 이 function도 바뀌어야 합니다. 더 이상 file block이 consecutive하다고 가정할 수 없습니다.

buffer cache는 이 translation 아래에 위치합니다. file system이 requested byte range를 포함하는 disk sector를 결정하면, 그 sector를 read하거나 write하도록 cache에 요청합니다.
