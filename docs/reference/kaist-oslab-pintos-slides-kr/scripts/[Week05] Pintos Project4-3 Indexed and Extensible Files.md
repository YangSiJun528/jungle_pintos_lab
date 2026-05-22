# [Week05] Pintos Project4-3 Indexed and Extensible Files

Source: https://youtu.be/4Vg66wWwkXE?si=et5lvozGrkyBRALs

## Indexed File Motivation

Pintos file system에서 indexed file을 어떻게 build할 수 있는지 설명하겠습니다.

original Pintos에서는 file size가 created될 때 fixed됩니다. 이것은 어느 정도 매우 limited합니다. 그래서 이 project에서는 Pintos file system을 modify해서 file size를 dynamically change할 것입니다. maximum file size는 8 megabytes가 될 것입니다. 이것이 modify할 내용입니다.

이것이 Pintos가 file을 create할 때 block을 allocate하는 방식입니다. file을 creating할 때, file A라고 합시다. start block address와 length를 inode에 save합니다. 그리고 another file을 create할 때도 다시 start block address와 length를 inode에 save합니다. 이것이 disk입니다. 이것이 disk입니다.

하지만 file system에서는 append를 매우 frequently perform합니다. 이 new file의 end에 block을 append하고, another file을 append하고, another file을 append합니다.

좋습니다. 하지만 Pintos file system에서는 current file system structure에서 그렇게 할 수 없습니다. file이 start address와 size로 represented되기 때문입니다. 이것은 file과 associated된 모든 blocks가 disk 위의 consecutive region에 placed되어야 한다는 뜻입니다. consecutive region입니다.

하지만 이 configuration에는 file A가 있습니다. file을 extend하고 싶은데 next address가 이미 occupied되어 있다면 file을 extend할 수 없습니다. 그래서 inode가 file을 represent하는 방식을 fundamentally change해야 합니다.

modification 이후, 즉 Pintos operating system에서 file structure를 modify한 뒤에는 어떻게 될까요? 보여 드리겠습니다.

modification 이후 inode는 bunch of pointers를 가지게 되고, 각 pointer는 data blocks를 point합니다. first file을 creating할 때, 1, 2, 3, 4, 5, 6, 7, 8 blocks를 8 pointers와 함께 allocate합니다. next file, file B를 creating할 때는 another file을 8 blocks로 create하고, 8 pointers가 있습니다.

file A를 expand하고 싶으면 free block이 있는 wherever에서 another block을 allocate하고, ninth pointer가 newly allocated block을 point하도록 set합니다. 그러면 file을 extend할 수 있습니다. 이것이 요구되는 일입니다.

## Seek Beyond End of File

좋습니다. detail로 조금 들어가겠습니다.

seek operation은 file의 current offset을 change합니다. 이 case를 생각해 봅시다. 이것이 current file의 size입니다. 이것이 512 bytes times 8 blocks라고 합시다. 이것은 4 kilobytes입니다. 그래서 current file size는 4 kilobytes입니다.

그런데 application이 seek system call을 call해서 예를 들어 10 kilobytes로 set합니다. 그러면 current offset은 4 kilobytes입니다. 죄송합니다, 너무 큰 것 같습니다. 6 kilobytes라고 합시다. 좋습니다. 그러면 current offset pointer는 file blocks가 allocated되어 있지 않은 position으로 updated됩니다.

좋습니다. 그래서 file system operation을 modify할 때, 그리고 file을 indexed organization으로 modify할 때, seek system call도 check하고 modify해야 합니다. seek가 실제로 file size beyond로 seek할 때 file size를 change하지 않고, block도 allocate하지 않습니다. offset만 update합니다.

이 position에서 write가 called되면, 이 block은 여전히 free이고 어떤 file에도 allocated되지 않았습니다. 하지만 offset은 이 point로 updated되어 있습니다. 이 point에서 application이 write system call을 call하면, contents를 이 point에 place하고, 아직 accessed되지 않은 intermediate blocks를 어떤 initial value로 initialize합니다.

이것을 hole이라고 부르고, hole은 value 0으로 initialized됩니다. 이것은 seek operation의 매우 interesting한 part입니다.

## Three Things to Do

해야 할 일은 세 가지입니다.

첫째, on-disk inode structure를 modify해야 합니다. 그 다음 on-disk inode를 사용하는 code를 modify해야 합니다. 여기에는 file offset을 block address로 changing하는 것, new inode를 creating하는 것, inode를 deleting하는 것이 포함됩니다.

또한 file의 extension을 handle하는 function을 modify하고 create해야 합니다.

## Modifying `inode_disk`

좋습니다. 이것은 Pintos의 `inode_disk`입니다. 이것은 sector start address이고, 이것은 file의 length입니다. current Pintos에서 file block, file은 start address와 length로 represented됩니다. 그래서 그것을 modify할 것입니다.

file을 extend할 수 있도록 file을 위한 on-disk inode structure를 modify할 것입니다.

이것은 sample inode입니다. 몇 개의 direct index, indirect block, 그리고 double indirect block이 있습니다.

direct block에는 data blocks를 directly point하는 direct block entries, number of direct pointers가 있습니다. indirect block은 data block을 point하는 pointer block을 point합니다. 세 번째는 double indirect block입니다. 이것은 another pointer block을 point하는 pointer block을 point하고, 그 다음 그것이 data blocks를 point합니다. 그래서 이것은 two-level indirection입니다.

좋습니다. 이것은 single file에 속한 actual file의 layout입니다.

Pintos에서 각 inode는 single block을 occupy합니다. 그래서 inode는 very large할 수 있습니다. 우리는 space를 waste하고 싶지 않습니다. 그런 이유로 가능한 한 많은 direct pointers를 allocate할 것입니다.

여기에는 0부터 124까지 있습니다. 이것은 그냥 example inode structure, sample inode입니다. file organization을 위해 different idea를 사용하고 싶다면, own idea를 사용해서 implement해도 됩니다.

어쨌든 124 direct blocks가 있고, single indirect가 있으며, another double indirect가 있습니다. 이것이 inode 안의 total number of pointers입니다.

좋습니다. new inode structure에는 file length가 있고, magic number가 있으며, 그 다음 126 pointers가 있습니다. 이것은 total 126 pointers입니다.

여기부터 여기까지 128 integers가 있고, 각 integer는 4 bytes입니다. 그래서 128 integers times 4 bytes는 512 bytes입니다. 여기에서 single inode, single on-disk inode는 entire sector를 occupy합니다.

## Computing Sector from File Offset

다음으로 해야 할 일은 file offset에서 sector number를 compute하는 것입니다. file offset에서 sector number를 compute할 수 있어야 합니다.

Pintos는 이미 `block_sector_t`를 return하는 function을 define합니다. 이것이 return value type이고, 이것이 `byte_to_sector`입니다. offset을 받고 그 offset에 associated된 sector number를 return합니다. 이것을 change해야 합니다.

이것이 `byte_to_sector` function입니다. position을 sector number로 convert합니다. 이 function을 properly change해야 합니다.

또 다른 change는 inode를 creating하는 code입니다. `inode_create`라는 function이 있습니다. `inode_create`에서 function은 two parameters를 supplied받습니다. 첫 번째는 sector이고, length가 있습니다. sector는 inode가 created되어야 하는 location이고, length는 created될 때의 file size입니다.

originally Pintos는 contiguous blocks를 allocate하고 그 start address를 save합니다. 하지만 여기서는 이제 block addresses가 모두 allocated되도록 code를 modify합니다. simple합니다.

## Deleting an Inode

inode를 deleting하는 것도 slightly changed되어야 합니다. inode를 delete할 때 `inode_close`에 block deallocating code를 add해야 합니다.

이것은 inode이고, bunch of pointers가 있으며, 그와 associated된 blocks가 있습니다. inode를 delete할 때 이 blocks를 모두 deallocate해야 합니다. 이것은 이 data blocks에 대한 free block bitmap을 zero로 set해야 한다는 뜻입니다.

## Extending a File

이제 file의 extension을 handle할 차례입니다. file size가 changes되면 new block을 allocate하고 inode 안의 data block pointer를 update합니다. 그 다음 allocated blocks를 zero로 fill해야 합니다. 이것이 해야 할 basic steps라고 생각합니다.

`inode_write_at` function이 있습니다. 물론 우리는 locks를 다루었습니다. appropriate locks를 acquire해야 합니다. existing file의 size beyond로 write할 때는 some field를 update해야 합니다.

## Next Topic: Subdirectories

다음 topic은 subdirectory입니다. original Pintos에는 root directory만 있고 other subdirectories는 없습니다. 그래서 file system을 더 reasonable하게 만들기 위해 subdirectory feature의 concept를 implement할 것이고, hierarchical tree structure의 concept를 bring in할 것입니다.

이것은 Pintos file system에서 subdirectory concept를 creating하는 course에서 modify해야 할 files의 list입니다.

좋습니다. 이것은 original Pintos에서 directory의 structure입니다. root directory만 있습니다. root directory만 있으므로 directory는 flat합니다. root directory는 당연히 inode를 가집니다. 그리고 inode는 associated data block을 가지고, data block은 directory entry를 가집니다.

Pintos file system에서 모든 directory entry는 그냥 file입니다. file은 file name과 inode number pair라는 뜻입니다. 이것들에 기반해서 file inode를 allocate하고, 그 다음 inode와 associated된 data block을 포함합니다.

이제 이 flat directory structure의 concept를 hierarchical directory structure로 change할 것입니다. 여기에서 root directory는 regular file뿐 아니라 another directory도 contain할 수 있습니다. 그것이 important part입니다.

좋습니다. 그래서 directory는 regular file뿐 아니라 directory file도 contain합니다.

important thing은 hierarchical directory structure concept에서는 모든 directory structure 안에 two pre-allocated directory entries가 있다는 것입니다. 하나는 current directory이고, 그 다음은 parent directory입니다.

details를 보겠습니다. 이것이 root directory라고 합시다. 그리고 inode가 있습니다. 그 다음 inode는 directory의 data blocks를 point하고, 이것들 각각은 directory entries입니다. directory entries는 normal file을 refer할 수도 있고, another directory tree를 refer할 수도 있습니다.

directory entries에서 first two directory entries는 special purpose를 위해 reserved됩니다. current directory와 parent directory입니다. 하지만 root directory의 경우 parent directory가 없으므로 자기 자신을 point합니다. 이것들이 hierarchical directory structure를 realizing할 때 implement할 data structures입니다.
