# [Week05] Pintos Project4-3 Indexed and Extensible Files

Source: https://youtu.be/4Vg66wWwkXE?si=et5lvozGrkyBRALs

## Original File Representation

original Pintos file system은 file이 생성될 때 fixed size를 부여합니다.

on-disk inode는 start sector와 file length를 저장합니다. 이는 file의 모든 sector가 disk 위의 하나의 consecutive region에 있어야 한다는 뜻입니다.

이 representation은 제한적입니다. File system에서는 existing file에 data를 append하는 일이 흔합니다. file 바로 다음 sector가 이미 다른 file에 의해 사용 중이면 original Pintos representation은 그 file을 제자리에서 extend할 수 없습니다.

growing file을 지원하려면 inode representation을 바꿔야 합니다.

## Indexed File Representation

수정 후 inode는 start sector 하나만 저장하는 대신 block pointer들을 포함해야 합니다.

각 pointer는 data block을 가리킬 수 있습니다. file이 grow해야 하면 Pintos는 disk 어디든 free sector를 allocate하고, 그 new sector를 가리키는 pointer를 추가할 수 있습니다. file의 block들이 더 이상 consecutive할 필요가 없습니다.

이것이 indexed file allocation의 핵심 idea입니다.

## Direct, Indirect, and Double-Indirect Blocks

가능한 design 중 하나는 Unix inode와 비슷한 구조입니다.

inode는 여러 direct pointer를 포함합니다. direct pointer는 data sector를 직접 가리킵니다.

inode는 indirect pointer도 포함합니다. indirect pointer는 indirect block을 가리키고, indirect block은 data sector를 가리키는 pointer들을 포함합니다.

inode는 double-indirect pointer도 포함할 수 있습니다. double-indirect pointer는 indirect block들을 가리키는 pointer block을 가리킵니다. 각 indirect block은 다시 data sector들을 가리킵니다.

이 structure는 small file에는 direct pointer를 효율적으로 사용하면서, large file은 indirect와 double-indirect level을 통해 지원할 수 있게 합니다.

Pintos on-disk inode는 하나의 512-byte sector를 차지하므로 inode layout은 정확히 그 sector 안에 들어가야 합니다.

## Changing `struct inode_disk`

original `struct inode_disk`는 start sector, length, magic number, unused space를 가집니다.

indexed file에서는 start sector가 pointer field들로 대체됩니다. inode는 여전히 file length와 magic number가 필요하지만, direct, indirect, double-indirect pointer field도 필요합니다.

direct pointer의 정확한 개수는 design decision입니다. 단, inode가 한 sector 안에 들어가야 하고 file system이 required maximum file size를 지원할 수 있어야 합니다.

## Translating a File Offset

file system은 file 안의 byte offset을 그 byte를 포함하는 disk sector로 변환해야 합니다.

original implementation에서 `byte_to_sector()`는 file offset을 sector size로 나눈 값을 start sector에 더해 sector를 계산할 수 있습니다.

indexed file에서는 `byte_to_sector()`가 inode pointer들을 확인해야 합니다.

block index가 direct-pointer range에 있으면 direct pointer를 사용합니다. indirect range에 있으면 indirect block을 읽고 적절한 pointer를 선택합니다. double-indirect range에 있으면 first-level pointer block을 읽고, 다시 second-level indirect block을 읽습니다.

이 offset-to-sector translation은 indexed file implementation의 central routine 중 하나가 됩니다.

## Creating an Inode

`inode_create()`도 바뀌어야 합니다.

original implementation은 consecutive sector range를 allocate하고 start sector만 저장합니다.

indexed representation이 추가되면 `inode_create()`는 pointer structure를 initialize하고 initial file length에 필요한 sector들을 allocate해야 합니다. allocated data sector들은 direct, indirect, double-indirect pointer structure를 통해 기록되어야 합니다.

newly allocated file sector는 보통 zero byte로 initialize해야 합니다.

## Deleting an Inode

file deletion도 더 복잡해집니다.

original representation에서는 file data sector가 하나의 consecutive extent이므로 deallocation이 단순합니다.

indexed file에서는 Pintos가 inode 안의 모든 pointer를 walk하고, allocated data sector를 모두 release해야 합니다. indirect block이나 double-indirect block이 allocate되어 있었다면 그 metadata block들도 release해야 합니다.

이 deallocation logic은 open reference가 더 이상 남지 않은 뒤 inode가 finally close and remove되는 path에 있어야 합니다.

## Extending a File

file system은 current end of file 이후로 쓰는 write를 지원해야 합니다.

`inode_write_at()`이 existing file length를 넘어 write하면 Pintos는 newly required sector들을 allocate하고, inode pointer를 update하며, file length를 update해야 합니다.

program이 이전에 `seek()`로 end of file 너머로 이동해서 gap을 만든 뒤 write했다면, 그 gap은 zero로 읽혀야 합니다. lecture에서 말한 hole-punching behavior입니다. end를 넘어 seek하는 것 자체는 block을 allocate하지 않지만, 이후 write가 intermediate region을 zero-filled data처럼 동작하게 만들어야 합니다.

extension path는 조심해서 synchronize해야 합니다. 여러 operation이 같은 file을 동시에 extend하려고 할 수 있기 때문입니다.

## Seek and File Size

`seek` operation은 open file object의 current offset을 변경합니다.

program이 current file size 너머로 seek하더라도 file size는 즉시 변하지 않고 block도 즉시 allocate되지 않습니다. current offset만 이동합니다.

file은 실제 write가 old end of file 너머에서 일어날 때 grow합니다.

이 구분이 중요합니다. `seek()`는 open file object의 position을 변경하고, `write()`는 file contents를 변경하며 inode를 바꿀 수 있습니다.

## Transition to Subdirectories

indexed file 다음 feature는 subdirectory입니다.

original Pintos file system에는 root directory만 있습니다. Project 4는 file system을 바꿔 directory가 regular file과 다른 directory를 모두 포함할 수 있게 하고, hierarchical tree를 형성하게 합니다.

subdirectory를 지원하려면 inode metadata, directory entries, path parsing, file creation, file opening, file removal, 그리고 new directory-related system calls를 수정해야 합니다.
