# [Week05] Pintos Project4-4 Subdirectories

Source: https://youtu.be/yQqNrIiB2hU?si=D7G5jkCkcnrFNj0-

## Directory Structure

subdirectory feature는 Pintos를 flat directory structure에서 hierarchical directory structure로 바꿉니다.

original Pintos file system에는 root directory만 있습니다. 모든 file은 그 root directory에 생성됩니다.

변경 후에는 directory entry가 regular file뿐 아니라 다른 directory file도 가리킬 수 있습니다. 이렇게 directories와 files의 tree가 만들어집니다.

## `.` and `..`

각 directory는 두 개의 special entry를 가져야 합니다.

`.`은 current directory itself를 가리킵니다.

`..`은 parent directory를 가리킵니다.

new directory가 생성될 때 그 directory는 완전히 empty가 아닙니다. 최소한 이 두 entry를 포함해야 합니다.

root directory의 경우 parent가 없으므로 `.`과 `..` 모두 root directory itself를 가리킵니다.

이 entries는 항상 존재해야 합니다. 이를 remove하려는 attempt는 fail해야 합니다.

## Paths

subdirectory가 존재하면 file system에는 path concept가 필요합니다.

path는 file이나 directory에 도달하기 위한 directory name의 sequence입니다.

absolute path는 root directory에서 시작합니다. `/user/pintos/src`처럼 `/`로 시작합니다.

relative path는 process의 current directory에서 시작합니다. current directory를 의미하는 `.`과 parent directory를 의미하는 `..`을 포함할 수 있습니다.

따라서 file을 create, open, remove하기 전에 path parsing이 필요합니다.

## File Types

subdirectory가 생기면 Pintos는 ordinary file과 directory file을 구분해야 합니다.

regular file은 byte sequence로 read/write됩니다. current offset은 byte position을 따라 이동합니다.

directory file은 다릅니다. contents가 directory entries이며, directory operation은 한 번에 directory entry 하나를 읽습니다.

따라서 inode에는 해당 inode가 regular file을 나타내는지 directory를 나타내는지 기록하는 field가 필요합니다. conceptually one bit면 충분하지만 실제 structure에서는 더 큰 field를 사용할 수 있습니다.

## Current Directory

각 process는 current working directory가 필요합니다.

즉 `struct thread`에는 current directory 또는 current directory의 inode를 가리키는 field가 있어야 합니다.

child process가 생성되면 parent process의 current directory를 inherit합니다.

initial process는 ordinary parent가 없으므로 current directory를 root directory로 initialize해야 합니다.

process 또는 thread structure에 이 field를 추가하는 일은 단순하지만, relative path가 process의 current directory에 의존하게 되므로 많은 file-system operation에 영향을 줍니다.

## Modifying File Creation

original `filesys_create()`는 root directory에 file을 생성합니다.

subdirectory가 구현된 뒤에는 file creation이 먼저 path를 parse해야 합니다. path parser는 parent directory와 final file name을 식별해야 합니다.

path가 absolute이면 parsing은 root directory에서 시작합니다. path가 relative이면 current directory에서 시작합니다.

parent directory를 찾은 뒤 Pintos는 new inode를 allocate하고, 필요에 따라 regular file 또는 directory로 initialize하며, parent directory에 directory entry를 추가합니다.

implementation은 failure를 조심해서 처리해야 합니다. inode sector를 allocate했지만 directory entry 추가가 fail하면, Pintos는 allocation을 roll back하고 file system을 consistent state로 남겨야 합니다.

## Creating a Directory

directory creation은 regular file creation과 비슷하지만, inode가 directory로 표시되어야 합니다.

new directory에는 `.`과 `..` entry도 필요합니다.

`.`은 new directory 자신의 inode를 가리켜야 합니다.

`..`은 parent directory의 inode를 가리켜야 합니다.

이 entries가 initialize된 뒤, parent directory는 new directory name을 new directory inode에 mapping하는 entry를 받습니다.

## Opening a File or Directory

open도 path parsing이 필요합니다.

original implementation은 root directory에서만 찾습니다. subdirectory가 있으면 Pintos는 path를 component 단위로 walk해야 합니다.

intermediate component를 만나면 그것은 directory여야 합니다. final component는 operation에 따라 regular file 또는 directory일 수 있습니다.

path가 absolute이면 lookup은 root에서 시작합니다. relative이면 process의 current directory에서 시작합니다.

## Removing Files and Directories

regular file removal은 target directory를 path parsing으로 찾는다는 점을 제외하면 original behavior와 비슷합니다.

directory removal에는 추가 check가 필요합니다. directory는 mandatory `.`과 `..` entry를 제외하고 empty일 때만 remove되어야 합니다.

special entry `.`과 `..` 자체는 removable하면 안 됩니다.

implementation은 correct removal rule을 선택하기 전에 target inode가 regular file인지 directory인지 구분해야 합니다.

## Directory System Calls

Project 4는 directory manipulation을 위한 system call을 추가합니다.

`chdir`는 process의 current working directory를 path로 지정된 directory로 변경합니다.

`mkdir`는 new directory를 생성합니다.

`readdir`는 open directory file descriptor에서 directory entry name 하나를 읽고 directory position을 advance합니다. `.`이나 `..`은 return하면 안 됩니다.

`isdir`는 file descriptor가 directory를 참조하는지 return합니다.

`inumber`는 file descriptor와 associated된 inode number를 return합니다.

이 system call들은 system-call layer가 file descriptor가 regular file을 가리키는지 directory를 가리키는지 이해해야 합니다.

## Summary

subdirectory는 단순히 directory entry를 추가하는 일보다 더 많은 변경을 요구합니다.

file system은 file type을 구분하고, process마다 current directory를 유지하며, absolute/relative path를 parse하고, `.`과 `..`을 initialize하고, create/open/remove operation을 수정하고, new directory system call을 노출해야 합니다.

이 조각들이 함께 구현되면 Pintos는 single flat root directory 대신 hierarchical directory tree를 지원할 수 있습니다.
