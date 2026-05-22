# Pintos 기본 filesys 구현 방식

작성일: 2026-05-22

이 문서는 Project 4 추가 기능을 붙이기 전, 즉 Project 2/3에서 이미 사용하던
Pintos 기본 파일 시스템이 어떤 방식으로 동작하는지 정리한다.

여기서 말하는 "추가 기능 전"은 다음이 아직 구현되지 않은 상태를 뜻한다.

- FAT 기반 indexed/extensible file
- subdirectory와 absolute/relative path
- process별 current working directory
- directory fd용 syscall
- symlink
- buffer cache

현재 저장소에는 Project 4 구현이 이미 섞여 있으므로, 코드를 볼 때는
`#ifndef EFILESYS` 또는 `#else /* Original FS */` 쪽 흐름을 기본 파일 시스템
구현으로 보면 된다. 이 문서는 lecture script의 설명을 참고하되, 요구사항
판단은 KAIST CS330 reference를 우선한다.

## 참고 자료

- `docs/reference/pintos-kaist-original/4_project4/0_introduction.md`
- `docs/reference/pintos-kaist-original/4_project4/1_indexed_and_extensible_files.md`
- `docs/reference/kaist-oslab-pintos-slides-kr/scripts/[Week05] Pintos Project4-0 File System Overview.md`
- `docs/reference/kaist-oslab-pintos-slides-kr/scripts/[Week05] Pintos Project4-1 File System Details.md`
- `pintos/filesys/filesys.c`
- `pintos/filesys/free-map.c`
- `pintos/filesys/inode.c`
- `pintos/filesys/directory.c`
- `pintos/filesys/file.c`

## 32-bit script와 현재 저장소의 용어 대응

강의 script는 32-bit Pintos 설명이 섞여 있어 block layer 용어를 사용한다. 파일
시스템의 개념과 계층은 거의 같지만, 현재 KAIST 64-bit 저장소에서는 이름이 일부
다르다.

| script 표현 | 현재 저장소에서 보면 |
|---|---|
| `struct block` | `struct disk` |
| `block_sector_t` | `disk_sector_t` |
| `block_read()` / `block_write()` | `disk_read()` / `disk_write()` |
| block device sector | disk sector |
| file system block | Pintos에서는 보통 512-byte disk sector와 같은 단위 |

따라서 script에서 "block을 읽는다"는 설명은 이 저장소 기준으로
`disk_read(filesys_disk, sector, buffer)`를 호출한다고 이해하면 된다.

주의할 점은 Project 4 구현 방식이다. 일부 강의 설명은 일반적인 indexed inode,
direct/indirect/doubly indirect pointer를 설명하지만, CS330 KAIST Project 4
reference는 제공된 `filesys/fat.c` skeleton으로 FAT를 구현하라고 요구한다. 이
문서는 기본 파일 시스템의 배경 설명이며, Project 4 file growth 설계 지침은
FAT 기준으로 봐야 한다.

## 전체 계층

기본 파일 시스템은 사용자에게 Unix-like file API를 제공하지만, 내부적으로는
이름, inode, sector 번호를 단계적으로 변환한다.

```text
user program
  |
  | create, open, read, write, seek, close, remove
  v
userprog/syscall.c
  |
  | fd table 조회, user buffer 검증, filesys_lock
  v
filesys/filesys.c
  |
  | file name 기준 create/open/remove
  v
filesys/directory.c
  |
  | root directory에서 name -> inode sector 변환
  v
filesys/file.c
  |
  | 열린 file object, current position 관리
  v
filesys/inode.c
  |
  | file offset -> disk sector 변환
  v
filesys/free-map.c
  |
  | 어느 sector가 비었는지 bitmap으로 관리
  v
devices/disk.c
  |
  | 512-byte sector 단위 disk_read/disk_write
  v
file system disk
```

핵심은 다음 네 가지다.

- directory는 file name을 inode sector로 바꾼다.
- inode는 파일 길이와 파일 data sector 위치를 기록한다.
- file object는 열린 파일 하나의 현재 offset을 관리한다.
- free map은 disk sector 할당 상태를 bitmap으로 관리한다.

## Disk layout

기본 파일 시스템은 disk를 sector 번호가 붙은 배열처럼 사용한다. `filesys.h`에는
중요한 metadata inode sector가 고정되어 있다.

```c
#define FREE_MAP_SECTOR 0
#define ROOT_DIR_SECTOR 1
```

format 직후의 배치는 개념적으로 다음과 같다.

```text
sector 0  : free-map file의 inode
sector 1  : root directory file의 inode
sector 2+ : free-map bitmap data sector들
그 이후   : root directory data sector, 일반 file inode, 일반 file data
```

8 MB partition 예시에서는 sector가 16,384개다. sector마다 bit 하나가 필요하므로
free-map bitmap은 16,384 bit, 즉 2,048 byte가 된다. 512-byte sector 네 개면 이
bitmap을 저장할 수 있다. lecture script의 "sector 2부터 5까지 free-map data,
sector 6이 root directory data" 예시는 이 계산에서 나온다.

실제 위치는 `free_map_allocate()`가 빈 sector를 연속으로 찾는 결과에 따라
정해지지만, 처음 format한 깨끗한 disk에서는 위와 같은 순서로 배치된다고 보면
된다.

## Format과 initialization

`filesys_init(format)`은 파일 시스템을 사용할 준비를 한다.

기본 파일 시스템 경로는 다음 순서다.

```text
filesys_init(format)
  filesys_disk = disk_get(0, 1)
  lock_init(&filesys_lock)
  inode_init()
  free_map_init()
  if format:
    do_format()
  free_map_open()
```

각 단계의 의미는 다음과 같다.

| 단계 | 역할 |
|---|---|
| `disk_get(0, 1)` | 파일 시스템용 disk를 가져온다. |
| `inode_init()` | open inode list를 초기화한다. |
| `free_map_init()` | memory 안에 sector bitmap을 만들고 sector 0, 1을 사용 중으로 표시한다. |
| `do_format()` | disk 위에 free-map file과 root directory를 새로 만든다. |
| `free_map_open()` | disk에 저장된 free-map file을 열고 bitmap 내용을 읽는다. |

`do_format()`의 기본 파일 시스템 흐름은 더 단순하다.

```text
do_format()
  free_map_create()
  dir_create(ROOT_DIR_SECTOR, 16)
  free_map_close()
```

`free_map_create()`는 sector 0에 free-map inode를 만들고, bitmap 내용을
free-map file data로 쓴다. `dir_create(ROOT_DIR_SECTOR, 16)`은 sector 1에 root
directory inode를 만들고, 16개 directory entry를 담을 수 있는 data 영역을
할당한다.

## Free map: sector 할당표

기본 파일 시스템의 free map은 disk sector마다 bit 하나를 두는 bitmap이다.

```text
bit == true   -> 해당 sector는 사용 중
bit == false  -> 해당 sector는 비어 있음
```

`free_map_allocate(cnt, &sector)`는 `cnt`개의 연속된 빈 sector를 찾는다. 찾으면
그 bit들을 used로 바꾸고, 시작 sector 번호를 `sector`에 돌려준다.

```text
free_map_allocate(3)

bitmap:
  used used free free free used ...
            ^
            여기서부터 3개 연속 sector 할당
```

중요한 특징은 "연속된 sector"를 요구한다는 점이다. 기본 파일 시스템에서 파일
하나는 single extent, 즉 disk 위의 연속 구간 하나로 저장되기 때문이다.

할당 상태는 memory 안의 bitmap만 바꾸면 부족하다. 재부팅 뒤에도 파일 시스템이
같은 상태를 보려면 bitmap을 free-map file에 다시 써야 한다. 그래서
`free_map_allocate()`와 `free_map_release()`는 가능한 경우 `bitmap_write()`로
disk에 free map을 반영한다.

## Inode: 파일의 실제 위치와 길이

inode는 파일의 실체를 나타내는 metadata다. 파일 이름은 inode 안에 들어 있지
않다. 이름은 directory entry에 있고, directory entry가 inode sector를 가리킨다.

Project 4 확장 전 on-disk inode는 개념적으로 다음 정보를 가진다.

```text
start   : 파일 data가 시작되는 disk sector
length  : 파일 길이(byte)
magic   : inode인지 확인하기 위한 값
unused  : sector 크기를 맞추기 위한 padding
```

현재 저장소의 `struct inode_disk`에는 Project 4 구현 때문에 `type` field가
추가되어 있지만, 기본 파일 시스템의 핵심은 `start + length`다.

```text
inode.start = 100
inode.length = 1500 bytes

파일 data sector:
  sector 100  첫 512 bytes
  sector 101  다음 512 bytes
  sector 102  나머지 476 bytes
```

`byte_to_sector(inode, pos)`는 file offset을 disk sector로 바꾼다. 기본 파일
시스템에서는 파일이 연속 sector에 있으므로 계산이 간단하다.

```text
sector = inode.start + pos / 512
```

이 구조의 장점은 구현이 쉽다는 것이다. 단점은 파일 중간 block이 따로 떨어져
있을 수 없다는 것이다. 그래서 큰 파일을 만들려면 그 크기만큼의 연속된 빈 sector
구간이 필요하다.

## Single extent allocation

기본 파일 시스템에서 파일 생성 시 `inode_create(sector, length)`는 파일 크기에
필요한 sector 수를 계산한다.

```text
sectors = DIV_ROUND_UP(length, DISK_SECTOR_SIZE)
```

그 뒤 `free_map_allocate(sectors, &inode.start)`로 data sector를 한 번에
할당한다. 즉 파일 하나는 다음처럼 저장된다.

```text
inode
  start = 20
  length = 2048

data
  sector 20
  sector 21
  sector 22
  sector 23
```

파일의 모든 data sector가 연속되어 있으므로, offset에서 sector를 찾을 때
별도의 index table이 필요 없다. 대신 이 방식은 external fragmentation에 취약하다.

예를 들어 빈 sector가 총 10개 있어도, 4개가 연속된 구간이 없으면 4-sector
파일을 만들 수 없다.

```text
free sector 총량은 충분함:

[free][used][free][used][free][used][free][free][used][free]

하지만 4개 연속 free 구간이 없으므로 4-sector 파일 생성 실패 가능
```

Project 4에서 FAT 기반 file allocation을 구현하는 이유가 여기에 있다.

## Directory: 이름을 inode sector로 바꾸는 파일

directory도 disk에 저장되는 파일이다. 다만 그 내용이 일반 byte data가 아니라
directory entry 배열이라는 점이 다르다.

기본 directory entry는 다음 정보를 가진다.

```text
inode_sector : 이 이름이 가리키는 inode의 sector 번호
name         : 파일 이름
in_use       : 이 entry가 사용 중인지 여부
```

`dir_lookup(dir, name, &inode)`는 directory file을 처음부터 끝까지 읽으면서
사용 중인 entry 중 이름이 같은 것을 찾는다. 찾으면 `inode_sector`로
`inode_open()`을 호출해 inode를 연다.

```text
root directory data:

entry 0: name="a.txt", inode_sector=10, in_use=true
entry 1: name="b.txt", inode_sector=20, in_use=true
entry 2: in_use=false
```

기본 파일 시스템에는 root directory만 있다. 그래서 `filesys_open("a.txt")`는
항상 root directory를 열고, 그 안에서 `"a.txt"`를 찾는다.

```text
filesys_open("a.txt")
  dir = dir_open_root()
  dir_lookup(dir, "a.txt", &inode)
  dir_close(dir)
  return file_open(inode)
```

`/a/b/c` 같은 경로를 component별로 해석하지 않는다. process별 current working
directory도 없다. `.`과 `..` entry도 기본 구조에서는 의미 있게 사용되지 않는다.

## File object: 열린 파일 인스턴스

inode가 disk 위 파일의 실체라면, `struct file`은 "열린 파일 하나"를 나타낸다.

기본적으로 `struct file`이 관리하는 중요한 상태는 다음이다.

- 어떤 inode를 가리키는지
- 다음 read/write가 시작될 현재 위치 `pos`
- write deny 상태

같은 파일을 두 번 열면 같은 inode를 가리키는 `struct file`이 두 개 생길 수
있다. 이때 file object마다 `pos`는 별도다.

```text
fd 3 -> struct file A -> inode 10, pos = 0
fd 4 -> struct file B -> inode 10, pos = 100
```

따라서 한 fd에서 `read()`를 해서 position이 변해도, 다른 fd의 position은 같이
움직이지 않는다.

## Read/write 흐름

사용자 프로그램이 fd로 read/write를 호출하면 syscall layer는 fd table에서
`struct file *`를 찾고 `file_read()` 또는 `file_write()`를 호출한다.

read 흐름은 다음과 같다.

```text
file_read(file, buffer, size)
  inode_read_at(file->inode, buffer, size, file->pos)
  file->pos += bytes_read
```

write 흐름도 비슷하다.

```text
file_write(file, buffer, size)
  inode_write_at(file->inode, buffer, size, file->pos)
  file->pos += bytes_written
```

실제 sector I/O는 `inode_read_at()`과 `inode_write_at()`이 한다.

```text
inode_read_at(inode, buffer, size, offset)
  while 남은 byte가 있으면:
    sector = byte_to_sector(inode, offset)
    sector 안에서 시작 offset 계산
    이번 sector에서 처리할 chunk 크기 계산
    full-sector이면 caller buffer로 바로 disk_read
    partial-sector이면 bounce buffer에 disk_read 후 필요한 부분만 복사
```

partial-sector write는 기존 sector의 나머지 byte를 보존해야 한다. 그래서 먼저
sector 전체를 bounce buffer로 읽고, 쓰려는 부분만 덮어쓴 뒤 sector 전체를 다시
쓴다.

```text
sector 100:
  [기존 앞부분][새로 쓸 부분][기존 뒷부분]

partial write:
  disk_read(sector 100, bounce)
  memcpy(bounce + sector_ofs, input, chunk_size)
  disk_write(sector 100, bounce)
```

## File growth가 없는 이유

기본 파일 시스템의 `inode_write_at()` 주석에는 EOF에서 write하면 보통 파일을
키우지만, growth는 아직 구현되어 있지 않다고 적혀 있다.

이 말은 다음 상황에서 중요하다.

```text
file length = 100 bytes
write offset = 100
write size = 10
```

현대 파일 시스템이라면 파일 길이가 110 byte가 된다. 하지만 기본 Pintos 파일
시스템은 생성 시점에 할당한 single extent만 알고 있다. 바로 뒤 sector가 비어
있는지, 아니면 다른 파일이 쓰고 있는지 알 수 없고, 파일을 다른 위치로 옮기는
로직도 없다.

그래서 기본 구현에서는 EOF 이후 write가 파일을 자연스럽게 확장하지 않는다.
`seek()`로 EOF 뒤 위치로 이동하는 것 자체는 file object의 `pos`만 바꿀 수
있지만, 실제 write는 inode 길이를 넘으면 더 이상 진행되지 않거나 일부만 쓴다.

Project 4의 extensible file 요구사항은 이 한계를 제거하는 작업이다.

## Create/open/remove 흐름

### Create

기본 `filesys_create(name, initial_size)`는 root directory에만 파일을 만든다.

```text
filesys_create(name, initial_size)
  dir = dir_open_root()
  free_map_allocate(1, &inode_sector)
  inode_create(inode_sector, initial_size)
  dir_add(dir, name, inode_sector)
  dir_close(dir)
```

여기서 `free_map_allocate(1, &inode_sector)`는 새 파일의 inode를 저장할 sector
하나를 할당한다. `inode_create()`는 `initial_size`만큼의 data sector를 추가로
할당하고, 새 inode를 disk에 쓴다. `dir_add()`는 root directory file에
`name -> inode_sector` entry를 추가한다.

실패하면 이미 할당한 inode sector를 release하는 rollback이 필요하다. 다만
기본 구현은 여러 metadata sector를 수정하므로, 중간 실패 처리와 일관성 유지가
항상 중요한 포인트다.

### Open

기본 `filesys_open(name)`은 root directory에서 name을 찾는다.

```text
filesys_open(name)
  dir = dir_open_root()
  dir_lookup(dir, name, &inode)
  dir_close(dir)
  return file_open(inode)
```

`file_open()`은 inode를 받아 새 `struct file`을 만들고 position을 0으로 둔다.
이후 syscall layer가 이 `struct file *`을 process의 fd table에 넣고 fd 번호를
사용자에게 반환한다.

### Remove

기본 `filesys_remove(name)`도 root directory에서만 동작한다.

```text
filesys_remove(name)
  dir = dir_open_root()
  dir_remove(dir, name)
  dir_close(dir)
```

`dir_remove()`는 directory entry를 찾아 `in_use=false`로 바꾼 뒤, 해당 inode에
`inode_remove()`를 호출한다.

중요한 점은 remove가 곧바로 disk sector를 모두 해제하지 않을 수 있다는 것이다.
다른 process나 fd가 그 inode를 아직 열고 있을 수 있기 때문이다. 그래서 inode는
`removed=true`로 표시되고, 마지막 opener가 `inode_close()`를 호출해 open count가
0이 되었을 때 inode sector와 data sector를 free map에 돌려준다.

```text
unlink/remove name:
  directory entry 제거
  inode->removed = true

last close:
  free_map_release(inode sector)
  free_map_release(data sectors)
```

이 구조 덕분에 이름이 지워진 파일도 이미 열린 fd로는 한동안 계속 접근할 수
있다.

## Persistence

파일 시스템은 memory 자료구조만 맞으면 안 된다. 재부팅 후에도 같은 파일을 볼 수
있어야 하므로 disk에 metadata가 남아야 한다.

기본 파일 시스템에서 persistence에 직접 관련되는 disk state는 다음이다.

| Disk state | 의미 |
|---|---|
| free-map file | 어떤 sector가 사용 중인지 |
| inode sector | 파일의 시작 sector, 길이, magic |
| directory data sector | 이름과 inode sector mapping |
| file data sector | 실제 파일 내용 |

Project 4 filesys test가 Pintos를 두 번 실행하는 이유도 이 때문이다. 첫 번째
실행에서 만든 파일과 directory가 두 번째 실행에서도 disk에서 복구되어야 한다.

## 기본 구현의 한계와 Project 4의 연결

기본 파일 시스템은 작고 단순한 테스트에는 충분하지만, 구조적으로 다음 한계가
있다.

| 한계 | 원인 | Project 4에서 필요한 개선 |
|---|---|---|
| external fragmentation | 파일이 single extent로만 저장됨 | FAT 기반 cluster chain |
| file growth 불가 | inode가 start sector와 length만 알고 있음 | EOF 이후 write 시 chain 확장 |
| root directory 크기 제한 | root directory도 고정 크기 파일 | directory file도 확장 가능해야 함 |
| subdirectory 없음 | name lookup이 root directory 하나에서 끝남 | path parser, `.`/`..`, cwd |
| directory fd 없음 | `open()` 대상이 regular file 중심 | `readdir`, `isdir`, `inumber` |
| symlink 없음 | inode type과 path 재해석 구조 없음 | symlink inode와 loop 방어 |
| buffer cache 없음 | read/write마다 disk sector 접근 | VM 기반 page cache 또는 buffer cache |

따라서 Project 4 구현은 기존 계층을 버리는 작업이 아니라, 이 계층들을 유지한
채로 아래 부분을 바꾸거나 확장하는 작업이다.

- `free_map`의 contiguous allocation 한계를 FAT allocation으로 바꾼다.
- `inode`가 single extent 대신 FAT chain을 따라가게 한다.
- `directory`가 root 하나가 아니라 여러 directory inode를 다루게 한다.
- `filesys_open/create/remove`가 단순 name이 아니라 path를 해석하게 한다.
- `file`과 syscall layer가 directory, symlink, cwd를 표현할 수 있게 한다.

한 줄로 요약하면, 기본 Pintos filesys는 "root directory의 이름을 inode sector로
바꾸고, inode의 `start + length`를 이용해 연속 sector를 읽고 쓰는 파일 시스템"이다.
Project 4는 이 단순한 single-extent 구조를 FAT와 계층 directory 기반으로 확장하는
과제다.
