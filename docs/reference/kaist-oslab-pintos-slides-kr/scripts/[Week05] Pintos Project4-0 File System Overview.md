# [Week05] Pintos Project4-0 File System Overview

Source: https://youtu.be/bqtjcc7-_yA?si=XcxTIi-JIpqSXjue

## Project 4 Scope

Project 4는 file system project입니다.

구현해야 할 큰 feature는 세 가지입니다. 첫 번째는 Pintos file system을 위한 buffer cache 구현입니다. 두 번째는 file representation을 indexed 방식으로 바꾸고 file을 extensible하게 만드는 것입니다. 세 번째는 subdirectory 구현입니다.

이 feature들을 구현하기 전에 Pintos file system의 기본 data structure를 이해해야 합니다.

## Inodes

inode는 disk 위의 file을 나타냅니다. 모든 file은 자기 own inode를 가집니다.

inode에는 file size와 해당 file에 속한 data block의 위치 같은 metadata가 들어 있습니다. 실제 file system에서는 permission, access time, modification time 같은 timestamp도 들어갈 수 있습니다.

구분해야 할 inode는 두 종류입니다.

첫 번째는 on-disk inode입니다. 이는 disk에 저장되는 representation입니다. 두 번째는 in-memory inode이며, in-core inode라고도 부릅니다. Operating system이 file에 접근하려면 on-disk inode를 읽고, 그것을 바탕으로 in-memory inode를 만듭니다.

in-memory inode는 on-disk inode의 superset입니다. on-disk inode data를 포함하고, inode 자체가 저장된 disk sector, file이 removed되었는지 여부, open reference count, write-denial state 같은 runtime information도 포함합니다.

## File Objects

inode가 file 자체를 나타낸다면, file object는 open된 file을 나타냅니다.

file이 open되면 operating system은 file object를 만듭니다. file object에서 가장 중요한 field는 current offset입니다. current offset은 다음 read 또는 write operation이 적용될 file 내부 위치입니다.

read 또는 write system call이 실행되면 operation은 current offset에서 시작합니다. operation이 끝나면 offset은 읽거나 쓴 byte 수만큼 증가합니다.

이 구분은 중요합니다. 여러 file object가 같은 inode를 참조할 수 있지만, 각 open file object는 자기 own current offset을 가질 수 있습니다.

## Regular Files, Directories, and Bitmap Files

regular file은 data block을 가집니다. 이 block에는 text, source code, document data, image data, video data 같은 file의 실제 contents가 들어 있습니다. inode는 file size와 data block의 위치를 기록합니다.

original Pintos file system에서 regular file은 start sector와 size로 표현됩니다. 즉 file은 disk 위의 consecutive region을 차지합니다. file이 1 GB라면, original representation은 연속된 1 GB region을 기대합니다.

directory도 file입니다. directory file은 directory entry의 array를 포함합니다. 각 directory entry는 file name을 inode sector에 mapping합니다. 즉 directory는 file-name과 inode-number pair의 set입니다.

bitmap도 Pintos에서는 file로 표현됩니다. bitmap은 bit의 array입니다. 각 bit는 대응되는 disk sector가 free인지 in use인지 알려 줍니다. bitmap이 file로 저장되므로 bitmap도 자기 own inode를 가집니다.

## Pintos File System Layout

8 MB Pintos file system partition을 가정해 봅시다. Pintos는 512-byte sector를 사용하므로, 8 MB partition에는 16,384개의 sector가 있습니다. sector number는 0부터 16,383까지입니다.

기본 layout은 다음과 같습니다.

Sector 0은 free-map bitmap file의 inode를 포함합니다.

Sector 1은 root directory의 inode를 포함합니다.

Sector 2부터 5까지는 bitmap data 자체를 포함합니다.

Sector 6은 root directory의 data block입니다.

나머지 sector들은 inode sector 또는 file data sector로 사용됩니다.

free-map bitmap은 sector마다 bit 하나가 필요합니다. 네 개 sector는 2,048 bytes이고, 이는 16,384 bits입니다. 따라서 네 개 bitmap sector면 16,384 sectors를 가진 8 MB file system partition을 표현하기 충분합니다.

file system partition이 더 커지면 bitmap도 더 커져야 합니다.

## On-Disk Inodes in the Original File System

original Pintos file system에서 각 on-disk inode는 하나의 512-byte sector를 차지합니다.

original `struct inode_disk`는 file의 start sector, file length, magic number를 저장합니다. 512-byte sector 대부분은 unused padding입니다.

이 representation은 단순하지만 중요한 limitation이 있습니다. inode가 start sector와 length만 저장하므로 file의 모든 data block이 disk 위에서 consecutive해야 합니다. 따라서 file을 생성한 뒤 확장하기 어렵습니다.

Project 4의 목표 중 하나는 이 single-extent representation을 indexed representation으로 바꾸는 것입니다.

## Directory Entries

directory file은 directory entry들을 포함합니다.

각 directory entry는 associated inode의 sector number, file name, 그리고 해당 entry가 in use인지 나타내는 flag를 저장합니다.

Pintos에서 file name은 `NAME_MAX`, 즉 14 characters로 제한되며 null terminator를 위해 1 byte가 추가됩니다. 따라서 directory는 fixed-size entry의 linear array입니다.

file name을 찾으려면 Pintos는 directory entry를 linear scan합니다. 이는 단순하지만 directory에 file이 많아지면 비싸질 수 있습니다.

중요한 점은 file name이 inode에 저장되지 않는다는 것입니다. file name은 directory entry에 속합니다. directory가 human-readable name을 file을 나타내는 inode sector에 mapping합니다.

## Free Map

free map은 file system partition에서 어떤 sector가 free이고 어떤 sector가 already in use인지 기록합니다.

Pintos에서 free map은 bitmap으로 저장됩니다. bitmap 자체도 file로 저장됩니다. 즉 free map도 다른 file처럼 inode와 data block을 가집니다.

Pintos가 new inode나 data block을 위한 sector를 allocate하면 free map을 update합니다. sector가 release되면 대응되는 bit를 다시 free로 표시합니다.

## Struct File

`struct file`은 file이 open될 때 생성됩니다.

이는 associated inode를 가리키는 pointer, current file position, write를 deny하는 데 쓰이는 flag를 포함합니다. position field는 read와 write operation에 사용되는 current offset입니다.

이 object는 open file에 대해서만 존재합니다. closed file은 disk 위에 inode를 가지고 있지만, 그 open instance에 대한 live `struct file` object는 없습니다.

## Project 4 Features

첫 번째 feature는 buffer cache입니다. buffer cache는 memory 일부를 disk block의 cache로 사용합니다. 이는 virtual memory와 반대 방향입니다. virtual memory는 disk를 memory의 확장처럼 사용할 수 있고, buffer cache는 memory를 disk의 cache처럼 사용합니다.

이 project에서 cache는 64 disk block을 보관해야 합니다. block을 read하거나 write할 때 operation은 buffer cache를 거쳐야 합니다. Dirty cached block은 결국 disk에 write back되어야 합니다.

두 번째 feature는 indexed and extensible files입니다. original Pintos inode는 start sector와 length를 저장하므로, 각 file은 하나의 consecutive extent입니다. Project 4에서는 inode가 direct, indirect, double-indirect pointer를 통해 block을 가리키도록 바꿉니다. 이렇게 하면 file 바로 다음 sector가 이미 사용 중이어도 file을 grow할 수 있습니다.

세 번째 feature는 subdirectory입니다. original Pintos file system에는 root directory만 있습니다. Project 4에서는 hierarchical directory structure를 추가하여 directory가 regular file과 다른 directory를 포함할 수 있게 합니다.
