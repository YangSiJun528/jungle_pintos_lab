# Pintos filesys 초심자용 구현 흐름

작성일: 2026-05-20

이 문서는 파일 시스템을 거의 모르는 사람이 Pintos Project 4 filesys 구현을 이해할 수 있도록, "무엇을 구현했는지", "왜 구현해야 했는지", "실제로 어떤 순서로 동작하는지"를 단계별로 설명한다.

기준 자료는 다음 문서다.

- `docs/reference/pintos-kaist-original/4_project4/0_introduction.md`
- `docs/reference/pintos-kaist-original/4_project4/1_indexed_and_extensible_files.md`
- `docs/reference/pintos-kaist-original/4_project4/2_subdirectories.md`
- `docs/reference/pintos-kaist-original/4_project4/3_buffer_cache.md`
- `docs/reference/pintos-kaist-original/4_project4/4_synchronization.md`
- `docs/ai/038_Pintos_filesys_구현_정리.md`

현재 구현은 no-vm filesys 테스트 기준으로 정리되어 있다. Buffer cache는 Project 4 extra credit이고, COW는 Project 3 extra 기능이므로 현재 범위에서는 구현하지 않았다.

## 1. 파일 시스템이 하는 일

디스크는 아주 단순하게 보면 "번호가 붙은 저장 칸들의 배열"이다. Pintos에서 디스크는 sector 단위로 읽고 쓴다.

예를 들어 디스크는 이런 식으로 보일 수 있다.

```text
sector 0
sector 1
sector 2
sector 3
...
```

하지만 사용자 프로그램은 보통 이렇게 생각하지 않는다.

```c
fd = open("/home/a.txt");
write(fd, "hello", 5);
close(fd);
```

사용자 프로그램은 "몇 번 sector에 써야 하는지"를 알고 싶어 하지 않는다. 대신 다음과 같은 개념을 원한다.

- `/home/a.txt` 같은 이름과 경로
- 파일을 열고 닫는 기능
- 파일 안의 byte를 읽고 쓰는 기능
- 디렉터리를 만들고 탐색하는 기능
- 전원을 껐다 켜도 데이터가 남아 있는 persistence

파일 시스템은 이 간격을 메우는 계층이다.

```text
사용자 프로그램
  |
  | open, read, write, mkdir, readdir 같은 syscall
  v
Pintos filesys
  |
  | 이름, 경로, inode, FAT, sector 계산
  v
디스크 sector 읽기/쓰기
```

한 줄로 말하면, 파일 시스템은 "사람이 이해하는 파일 이름과 디렉터리 구조"를 "디스크 sector 읽기/쓰기"로 번역하는 코드다.

## 2. 먼저 알아야 할 핵심 용어

### Sector

sector는 디스크가 읽고 쓰는 기본 단위다. Pintos에서는 보통 512바이트 단위로 디스크 I/O를 한다.

파일이 10바이트여도 디스크 입장에서는 sector 일부를 읽거나 sector 전체를 읽은 뒤 필요한 byte만 골라 써야 한다.

### Cluster

cluster는 파일 시스템이 파일 데이터를 할당하는 단위다. KAIST Pintos skeleton에서는 `SECTORS_PER_CLUSTER == 1`이므로 cluster 하나가 sector 하나와 대응된다.

```text
cluster 5 == 어떤 data sector 하나
cluster 6 == 다음 data sector 하나
```

### Inode

inode는 파일 또는 디렉터리의 실제 정보를 담는 메타데이터다.

중요한 점은 파일 이름이 inode 안에 직접 들어 있는 것이 아니라는 점이다. 이름은 디렉터리가 관리하고, inode는 그 이름이 가리키는 "파일의 실체"를 설명한다.

현재 구현의 on-disk inode는 대략 다음 정보를 가진다.

```c
struct inode_disk {
  cluster_t start;   /* 파일 데이터의 첫 cluster. */
  off_t length;      /* 파일 길이. */
  unsigned magic;
  uint32_t type;     /* 일반 파일, 디렉터리, symlink 구분. */
  uint32_t unused[124];
};
```

즉 inode는 다음 질문에 답한다.

- 이 파일의 데이터는 어디서 시작하는가?
- 이 파일의 길이는 몇 byte인가?
- 이 inode는 일반 파일인가, 디렉터리인가, symlink인가?

### Directory

디렉터리는 이름을 inode로 바꿔 주는 표다.

예를 들어 `/home` 디렉터리 안에 `a.txt`가 있다면, 디렉터리 파일 안에는 대략 이런 entry가 들어 있다.

```text
name: "a.txt" -> inode sector: 123
```

그래서 `open("/home/a.txt")`를 하면 파일 시스템은 다음 일을 한다.

1. root directory `/`에서 `home`이라는 이름을 찾는다.
2. `home`이 가리키는 inode를 연다.
3. 그 inode가 디렉터리인지 확인한다.
4. `home` 디렉터리 안에서 `a.txt`라는 이름을 찾는다.
5. `a.txt`가 가리키는 inode를 연다.

### FAT

FAT은 File Allocation Table의 약자다. 파일 데이터가 여러 cluster에 흩어져 있을 때, "다음 cluster가 어디인지"를 별도의 표에 저장한다.

예를 들어 어떤 파일의 데이터가 cluster 5, 9, 12에 저장되어 있다면 FAT은 이렇게 생긴다.

```text
inode.start = 5

FAT[5]  = 9
FAT[9]  = 12
FAT[12] = EOChain
```

`EOChain`은 End Of Chain, 즉 "여기서 파일 데이터 chain이 끝난다"는 뜻이다.

### File Descriptor

file descriptor, 줄여서 fd는 프로세스가 열어 둔 파일을 가리키는 작은 정수다.

```c
int fd = open("a.txt");
write(fd, "hello", 5);
close(fd);
```

사용자 프로그램은 kernel 내부의 `struct file *` 포인터를 직접 볼 수 없다. 대신 fd를 넘긴다. Pintos kernel은 현재 프로세스의 fd table에서 fd에 해당하는 열린 파일 객체를 찾아 실제 작업을 한다.

### Current Working Directory

current working directory, 줄여서 cwd는 상대 경로의 시작점이다.

```text
cwd = /home

open("a.txt")      -> /home/a.txt
open("/tmp/a.txt") -> /tmp/a.txt
```

절대 경로는 `/`에서 시작하고, 상대 경로는 현재 프로세스의 cwd에서 시작한다.

## 3. Project 4 이전 Pintos 파일 시스템의 한계

Project 4 이전의 기본 파일 시스템은 이미 `open`, `read`, `write` 같은 기본 syscall을 지원한다. 하지만 구조적으로 한계가 크다.

### 한계 1: 파일이 연속된 공간에 저장된다

기본 파일 시스템은 파일을 하나의 contiguous extent로 저장한다. 즉 파일 하나가 디스크의 연속된 sector 구간을 차지해야 한다.

문제는 빈 공간의 총량은 충분한데, 연속된 큰 구간이 없을 수 있다는 점이다.

```text
빈 sector: 10개
필요한 파일 크기: 4 sector

하지만 빈 공간이 이렇게 흩어져 있으면:

[free][used][free][used][free][used][free] ...

연속된 4 sector를 못 찾아서 파일 생성이 실패할 수 있다.
```

이 문제가 external fragmentation이다.

### 한계 2: 파일 크기가 생성 시점에 고정된다

현대 파일 시스템에서는 보통 파일을 처음 만들 때 크기 0으로 만들고, write를 하면서 파일이 커진다.

하지만 기본 Pintos 파일 시스템은 파일 생성 시 지정한 크기에서 자연스럽게 확장되지 않는다. Project 4에서는 EOF 뒤에 write하면 파일이 확장되어야 한다.

### 한계 3: 디렉터리 구조가 단층이다

기본 파일 시스템은 root directory 하나만 있는 구조에 가깝다.

Project 4에서는 다음 같은 계층 구조가 필요하다.

```text
/
├── home
│   └── a.txt
└── tmp
    └── log.txt
```

### 한계 4: 프로세스별 cwd가 없다

상대 경로를 제대로 지원하려면 프로세스마다 현재 디렉터리를 가져야 한다.

```text
process A cwd = /home
process B cwd = /tmp

둘 다 open("a.txt")를 호출해도 서로 다른 파일을 열 수 있다.
```

### 한계 5: 디렉터리를 fd로 다루는 syscall이 부족하다

Project 4에서는 디렉터리도 `open()`할 수 있어야 하고, 열린 디렉터리에 대해 `readdir`, `isdir`, `inumber` 같은 syscall이 동작해야 한다.

## 4. 우리가 구현한 전체 방향

현재 구현의 핵심 방향은 다음이다.

```text
기존:
  파일 하나 = 디스크의 연속된 sector 구간
  디렉터리 = 거의 root 하나
  파일 크기 = 생성 시점에 고정

변경:
  파일 하나 = FAT chain으로 연결된 여러 cluster
  디렉터리 = inode를 backing store로 쓰는 특수한 파일
  파일 크기 = write에 따라 확장
  경로 = root 또는 cwd에서 component를 하나씩 해석
```

구현 계층은 대략 이렇게 나뉜다.

```text
user program
  |
  v
userprog/syscall.c
  - 사용자 포인터 검증
  - fd table 조회
  - filesys_lock 획득
  |
  v
filesys/filesys.c
  - create/open/remove/mkdir/chdir/symlink
  - path resolution
  |
  +--> filesys/directory.c
  |     - 이름을 inode sector로 변환
  |     - '.', '..', readdir 처리
  |
  +--> filesys/file.c
  |     - 열린 일반 파일 또는 열린 디렉터리 handle 관리
  |     - read/write/close/readdir/isdir/inumber
  |
  v
filesys/inode.c
  - inode metadata 관리
  - byte offset을 disk sector로 변환
  - 파일 확장
  |
  v
filesys/fat.c
  - FAT table 관리
  - cluster chain 생성, 확장, 제거
  |
  v
disk sector I/O
```

## 5. FAT 기반 파일 배치

### 왜 FAT이 필요한가

Project 4 reference는 기존 contiguous extent 방식의 external fragmentation 문제를 없애라고 요구한다. KAIST Pintos에서는 direct, indirect, doubly indirect block을 쓰는 FFS 스타일 구현을 금지하고, skeleton의 FAT 방식을 사용하라고 한다.

FAT을 쓰면 파일 데이터가 디스크에 연속으로 붙어 있지 않아도 된다.

```text
파일 A:
  cluster 5 -> cluster 9 -> cluster 12

파일 B:
  cluster 6 -> cluster 20
```

파일 A의 데이터가 5, 9, 12처럼 떨어져 있어도 FAT table이 연결 관계를 알고 있으므로 하나의 파일처럼 읽을 수 있다.

### FAT이 저장하는 것

FAT entry 하나는 "이 cluster 다음 cluster가 무엇인가"를 저장한다.

```text
FAT[cluster] = next cluster
```

예시는 다음과 같다.

```text
inode.start = 5

cluster 5의 다음 = 9
cluster 9의 다음 = 12
cluster 12의 다음 = EOChain
```

이러면 파일 시스템은 파일의 첫 cluster만 알면 전체 데이터를 따라갈 수 있다.

### 현재 구현의 핵심 함수

`fat.c`에서 핵심적으로 구현한 함수는 다음이다.

- `fat_fs_init()`: FAT 영역과 data 영역의 위치, cluster 개수 같은 메타데이터를 초기화한다.
- `fat_create_chain(clst)`: 새 cluster를 하나 할당한다. `clst == 0`이면 새 chain을 만들고, 아니면 기존 chain 뒤에 붙인다.
- `fat_remove_chain(clst, pclst)`: 특정 cluster부터 이어지는 chain을 free 상태로 되돌린다.
- `cluster_to_sector()`: cluster 번호를 실제 disk sector 번호로 바꾼다.
- `sector_to_cluster()`: disk sector 번호를 cluster 번호로 바꾼다.

### 파일이 커질 때 FAT은 어떻게 바뀌는가

처음에는 파일이 비어 있을 수 있다.

```text
inode.start = 0
inode.length = 0
```

여기에 처음으로 데이터를 쓰면 cluster 하나를 할당한다.

```text
inode.start = 5
inode.length = 3

FAT[5] = EOChain
```

파일이 더 커져서 cluster 하나로 부족해지면 뒤에 cluster를 붙인다.

```text
inode.start = 5
inode.length = 700

FAT[5] = 9
FAT[9] = EOChain
```

이 구조 때문에 파일은 디스크에서 꼭 연속된 공간을 차지할 필요가 없다.

## 6. Inode와 파일 확장

### inode가 하는 일

inode는 파일 이름을 저장하지 않는다. 대신 파일의 내용이 어디에 있고, 길이가 얼마고, 타입이 무엇인지 저장한다.

현재 구현에서는 inode 안의 `start`가 FAT chain의 시작점이다.

```text
directory entry:
  "a.txt" -> inode sector 123

inode sector 123:
  start = 5
  length = 700
  type = INODE_FILE

FAT:
  5 -> 9 -> EOChain
```

이렇게 이름, inode, FAT, data sector가 분리되어 있다.

### byte offset을 sector로 바꾸는 과정

사용자 프로그램은 byte 단위로 읽고 쓴다.

```c
read(fd, buffer, 10);
write(fd, buffer, 10);
```

하지만 디스크는 sector 단위로 읽고 쓴다. 그래서 `inode.c`는 파일 안의 byte offset을 실제 disk sector로 바꿔야 한다.

예를 들어 sector 크기가 512바이트라고 하면:

```text
offset 0     -> 파일의 0번째 cluster
offset 511   -> 파일의 0번째 cluster
offset 512   -> 파일의 1번째 cluster
offset 1024  -> 파일의 2번째 cluster
```

`byte_to_sector()`는 이 계산을 한다.

1. offset이 파일 길이 안에 있는지 확인한다.
2. `offset / BLOCK_SECTOR_SIZE`로 몇 번째 cluster인지 계산한다.
3. inode의 `start`에서 FAT chain을 그만큼 따라간다.
4. 찾은 cluster를 disk sector 번호로 바꾼다.

### EOF 뒤에 write하면 어떻게 되는가

Project 4에서는 파일 끝, 즉 EOF 뒤에 write하면 파일이 확장되어야 한다.

예를 들어 현재 파일 길이가 3바이트라고 하자.

```text
현재 파일 내용:
offset 0: a
offset 1: b
offset 2: c
EOF = 3
```

여기서 offset 10에 `X`를 쓰면 파일은 offset 11까지 커져야 한다.

```text
offset 0:  a
offset 1:  b
offset 2:  c
offset 3:  0
offset 4:  0
offset 5:  0
offset 6:  0
offset 7:  0
offset 8:  0
offset 9:  0
offset 10: X
EOF = 11
```

중간 gap은 0으로 채워져야 한다. 현재 구현은 sparse file 방식으로 gap을 생략하지 않고, 필요한 cluster를 실제로 할당하고 0으로 초기화한다.

### 현재 구현의 흐름

`inode_write_at()`은 write 범위가 현재 파일 길이를 넘는지 확인한다.

```text
write_end = offset + size

if write_end > inode.length:
  inode_extend(write_end)
```

`inode_extend()`는 다음 일을 한다.

1. 새 길이에 필요한 cluster 수를 계산한다.
2. 기존 FAT chain에 부족한 cluster를 추가한다.
3. 새로 할당한 cluster를 0으로 초기화한다.
4. inode의 `length`를 새 길이로 갱신한다.
5. 중간에 실패하면 이번 확장에서 새로 붙인 chain을 되돌린다.

그 다음 기존 sector 단위 write 로직으로 실제 데이터를 쓴다.

## 7. 디렉터리와 경로 해석

### 디렉터리는 특수한 파일이다

현재 구현에서 디렉터리도 inode를 가진다. 디렉터리 inode의 data 영역에는 `struct dir_entry`들이 저장된다.

개념적으로는 다음과 같다.

```text
directory "/":
  "."     -> root inode
  ".."    -> root inode
  "home"  -> inode 10
  "tmp"   -> inode 20

directory "/home":
  "."     -> inode 10
  ".."    -> root inode
  "a.txt" -> inode 30
```

`"."`은 자기 자신이고, `".."`은 부모 디렉터리다. root directory의 `".."`는 자기 자신을 가리킨다.

### 왜 디렉터리도 확장되어야 하는가

디렉터리 안에 파일이 많아지면 directory entry도 많아진다. Project 4 reference는 root directory도 기존 16개 제한을 넘어서 확장될 수 있어야 한다고 요구한다.

현재 구현에서는 디렉터리도 inode 기반 파일이므로, entry가 늘어나면 일반 파일처럼 `inode_write_at()`을 통해 확장된다.

### 경로를 해석하는 기본 규칙

경로는 `/`로 나뉜 component들의 목록이다.

```text
/home/user/a.txt

component:
  home
  user
  a.txt
```

절대 경로는 root directory에서 시작한다.

```text
/home/a.txt
^
root에서 시작
```

상대 경로는 현재 프로세스의 cwd에서 시작한다.

```text
cwd = /home

open("a.txt") -> /home/a.txt
```

### `resolve_path()` 흐름

`resolve_path()`는 전체 경로가 가리키는 inode를 찾는 함수로 볼 수 있다.

예를 들어 `open("/home/a.txt")`는 대략 이렇게 진행된다.

```text
1. path가 "/"로 시작하므로 root directory에서 시작한다.
2. "home" component를 root directory에서 찾는다.
3. "home"이 directory inode인지 확인하고 그 directory로 이동한다.
4. "a.txt" component를 /home directory에서 찾는다.
5. "a.txt"의 inode를 반환한다.
```

### `resolve_parent()`가 따로 필요한 이유

파일을 여는 경우에는 전체 경로가 이미 존재해야 한다.

```text
open("/home/a.txt")
  -> /home/a.txt inode를 찾으면 됨
```

하지만 파일을 만드는 경우에는 마지막 이름이 아직 없어야 한다.

```text
create("/home/new.txt")
  -> /home directory는 있어야 함
  -> new.txt는 아직 없어야 함
  -> 새 inode를 만들고 /home에 "new.txt" entry를 추가해야 함
```

그래서 생성, 삭제, symlink 생성 같은 작업에서는 마지막 component의 부모 directory와 basename을 찾는 `resolve_parent()`가 필요하다.

```text
resolve_parent("/home/new.txt")
  parent = /home directory
  basename = "new.txt"
```

## 8. 프로세스별 current working directory

Project 4에서는 프로세스마다 별도의 cwd가 있어야 한다.

현재 구현은 `struct thread`에 `struct dir *cwd`를 추가해서 이를 관리한다.

### 초기 상태

처음 프로세스의 cwd는 root directory로 설정된다.

```text
initial process:
  cwd = /
```

### `chdir()` 호출

사용자가 다음 syscall을 호출한다고 하자.

```c
chdir("/home");
```

흐름은 다음과 같다.

```text
1. "/home" 경로를 inode로 해석한다.
2. 찾은 inode가 directory인지 확인한다.
3. 현재 thread의 cwd를 새 directory로 교체한다.
4. 기존 cwd handle은 닫는다.
```

이후 같은 프로세스에서 상대 경로를 쓰면 `/home`을 기준으로 해석된다.

```text
cwd = /home

open("a.txt") -> /home/a.txt
```

### fork와 cwd

fork 시 자식 프로세스는 부모의 cwd를 상속한다. 하지만 이후에는 독립적으로 움직여야 한다.

```text
fork 직후:
  parent cwd = /home
  child cwd  = /home

child가 chdir("/tmp") 호출 후:
  parent cwd = /home
  child cwd  = /tmp
```

이를 위해 현재 구현은 부모의 `cwd` 포인터를 그대로 공유하지 않고 `dir_reopen()`으로 별도 directory handle을 만든다. 같은 inode를 가리킬 수는 있지만, 열린 directory 객체 자체는 부모와 자식이 따로 가진다.

## 9. 파일 descriptor에서 디렉터리 지원

Project 4에서는 `open()`이 일반 파일뿐 아니라 디렉터리도 열 수 있어야 한다.

```c
int fd = open("/home");
bool yes = isdir(fd);
readdir(fd, name);
close(fd);
```

기존의 `struct file`은 일반 파일만 표현한다고 보면 된다. 현재 구현에서는 `struct file`이 일반 파일 또는 directory handle을 감쌀 수 있게 확장되었다.

개념적으로는 다음과 같다.

```text
fd table entry
  |
  v
struct file
  - 일반 파일이면 inode를 직접 사용
  - 디렉터리이면 struct dir *dir 사용
```

디렉터리 fd에 대해 일반 `read()`나 `write()`를 허용하면 디렉터리 내부 포맷이 깨질 수 있다. 그래서 현재 구현에서는 디렉터리 fd에 대한 일반 read/write는 실패하도록 한다. 디렉터리는 `readdir()`로 읽는다.

## 10. 새 syscall과 기존 syscall 변경

Project 4에서 중요하게 다루는 syscall은 다음과 같다.

| syscall | 현재 구현에서 하는 일 |
| --- | --- |
| `open(path)` | 일반 파일 또는 디렉터리를 연다. symlink는 필요한 경우 따라간다. |
| `close(fd)` | 일반 파일 fd와 디렉터리 fd를 모두 닫는다. |
| `read(fd, buf, size)` | 일반 파일에서 읽는다. 디렉터리 fd면 실패한다. |
| `write(fd, buf, size)` | 일반 파일에 쓴다. 필요하면 파일을 확장한다. 디렉터리 fd면 실패한다. |
| `remove(path)` | 일반 파일 또는 빈 디렉터리를 삭제한다. |
| `chdir(path)` | 현재 프로세스의 cwd를 바꾼다. |
| `mkdir(path)` | 새 디렉터리를 만든다. |
| `readdir(fd, name)` | 디렉터리 fd에서 다음 entry 이름을 읽는다. `.`과 `..`는 반환하지 않는다. |
| `isdir(fd)` | fd가 디렉터리인지 확인한다. |
| `inumber(fd)` | fd가 가리키는 inode number를 반환한다. Pintos에서는 inode sector 번호를 써도 된다. |
| `symlink(target, linkpath)` | target path 문자열을 담는 symbolic link를 만든다. |

`syscall.c`에서는 사용자 포인터 검증도 중요하다.

- kernel이 사용자 buffer에서 읽기만 하면 readable이어야 한다.
- kernel이 사용자 buffer에 써야 하면 writable이어야 한다.

예를 들어 `read(fd, buffer, size)`는 kernel이 사용자 `buffer`에 데이터를 써 주는 syscall이므로 writable buffer 검증이 필요하다. `readdir(fd, name)`도 kernel이 이름을 써 주므로 writable 검증이 필요하다.

## 11. Symlink 동작

symlink는 다른 경로를 가리키는 특수한 파일이다.

예시는 다음과 같다.

```text
/
├── file
└── a
    ├── link1 -> /file
    └── link2 -> ../file
```

`link1`은 절대 경로 `/file`을 가리킨다. `link2`는 `/a` 기준 상대 경로 `../file`을 가리킨다.

현재 구현에서는 symlink inode의 data 영역에 target path 문자열을 저장한다.

```text
symlink inode:
  type = INODE_SYMLINK
  data = "/file"
```

경로 해석 중 symlink를 만나면 그 안의 target 문자열을 읽고 다시 경로 해석을 수행한다.

무한 순환을 막기 위해 symlink follow 깊이에는 제한을 둔다.

```text
a -> b
b -> a

이런 경우 계속 따라가면 끝나지 않으므로 depth limit이 필요하다.
```

## 12. 삭제는 왜 조심해야 하는가

파일 삭제는 단순히 disk sector를 바로 지우는 일이 아니다.

디렉터리 안에는 이름에서 inode로 가는 entry가 있다.

```text
"/home/a.txt" entry -> inode 30
```

`remove("/home/a.txt")`를 하면 먼저 directory entry를 제거한다. 하지만 이미 누군가 그 파일을 열어 두고 있을 수 있다.

```text
process A:
  fd = open("/home/a.txt")

process B:
  remove("/home/a.txt")
```

이 경우 process A의 열린 fd가 아직 inode를 사용 중일 수 있으므로, inode와 data cluster를 즉시 반환하면 위험하다.

현재 구현은 inode에 removed 표시를 하고, 마지막 opener가 닫는 시점에 실제 FAT chain을 반환하는 방식으로 처리한다.

디렉터리는 더 조심해야 한다.

- root directory는 삭제하면 안 된다.
- `.` 또는 `..` 삭제 요청은 거부한다.
- 비어 있지 않은 directory는 삭제하면 안 된다.
- 현재 구현은 다른 곳에서 열린 directory 삭제도 `inode_open_count()` 기준으로 거부한다.

## 13. 동기화

파일 시스템 자료구조는 여러 thread가 동시에 접근할 수 있다.

예를 들어 두 프로세스가 동시에 파일을 만들면 다음 자료구조를 같이 건드릴 수 있다.

- FAT table
- open inode list
- directory entry
- inode length
- fd table과 file object

동기화가 없으면 두 thread가 같은 cluster를 동시에 할당하거나, directory entry를 서로 덮어쓸 수 있다.

현재 구현은 정확성을 우선해서 coarse-grained locking을 사용한다.

- `filesys_lock`: syscall handler, process cleanup, load path 등 파일 시스템 공유 자료구조 접근을 직렬화한다.
- `fat_fs->write_lock`: FAT entry 할당과 반환을 보호한다.

이 방식은 병렬성은 낮지만 구조가 단순하고 테스트에서 요구하는 일관성을 맞추기 쉽다. 서로 다른 디렉터리의 독립 작업까지 완전히 병렬로 처리하는 fine-grained locking은 구현하지 않았다.

## 14. 실제 동작 예시

### 예시 1: `mkdir("/home")`

```text
사용자 프로그램
  |
  | mkdir("/home")
  v
syscall.c
  - 사용자 문자열 포인터 검증
  - filesys_lock 획득
  |
  v
filesys_mkdir()
  - resolve_parent("/home")
    parent = root directory
    basename = "home"
  - "home"이 이미 있는지 확인
  - 새 directory inode 생성
  - 새 directory 안에 "."과 ".." entry 추가
  - root directory에 "home" entry 추가
  |
  v
filesys_lock 해제
```

결과:

```text
/
└── home
```

### 예시 2: `create("/home/a.txt", 0)`

```text
1. "/home/a.txt"의 parent를 찾는다.
   parent = /home
   basename = "a.txt"

2. 새 일반 파일 inode를 만든다.
   length = 0
   start = 0
   type = INODE_FILE

3. /home directory에 entry를 추가한다.
   "a.txt" -> 새 inode
```

결과:

```text
/
└── home
    └── a.txt
```

파일 크기가 0이므로 아직 data cluster가 없을 수 있다.

### 예시 3: `write(fd, "hello", 5)`

```text
user write(fd, "hello", 5)
  |
  v
syscall.c
  - fd table에서 struct file 찾기
  - buffer readable 검증
  |
  v
file_write()
  |
  v
inode_write_at()
  - offset + size가 inode.length보다 큰지 확인
  - 크면 inode_extend() 호출
  |
  v
inode_extend()
  - 필요한 cluster 수 계산
  - fat_create_chain()으로 cluster 할당
  - 새 cluster 0 초기화
  - inode.length 갱신
  |
  v
sector write
```

결과:

```text
inode:
  start = 5
  length = 5

FAT:
  FAT[5] = EOChain

data cluster 5:
  "hello"
```

### 예시 4: `open("/home/a.txt")`

```text
1. "/"에서 시작한다.
2. root directory에서 "home"을 찾는다.
3. /home directory에서 "a.txt"를 찾는다.
4. a.txt inode를 연다.
5. struct file을 만들고 fd table에 넣는다.
6. 사용자에게 fd를 반환한다.
```

사용자 프로그램은 이후 fd만 사용한다.

```c
read(fd, buffer, size);
close(fd);
```

### 예시 5: `readdir(open("/home"), name)`

```text
1. open("/home")으로 directory fd를 얻는다.
2. readdir(fd, name)을 호출한다.
3. fd가 directory인지 확인한다.
4. directory file의 다음 dir_entry를 읽는다.
5. "."과 ".."는 건너뛴다.
6. 실제 entry 이름을 사용자 buffer name에 써 준다.
```

`readdir`은 사용자 buffer에 문자열을 써 주므로 `name`은 writable해야 한다.

### 예시 6: `symlink("/home/a.txt", "/home/link")`

```text
1. linkpath "/home/link"의 parent를 찾는다.
   parent = /home
   basename = "link"

2. type이 INODE_SYMLINK인 inode를 만든다.

3. symlink inode data에 target 문자열 "/home/a.txt"를 저장한다.

4. /home directory에 "link" entry를 추가한다.
```

이후 `open("/home/link")`를 하면 path resolver가 symlink target인 `/home/a.txt`를 따라가서 실제 파일을 연다.

## 15. 구현한 것과 구현하지 않은 것

### 구현한 것

| 기능 | 이유 |
| --- | --- |
| FAT 기반 cluster chain | contiguous extent의 external fragmentation 문제를 해결하기 위해 |
| 파일 확장 | EOF 이후 write가 파일을 키워야 하므로 |
| gap zero fill | EOF를 넘어 write한 중간 구간은 0으로 보여야 하므로 |
| root directory 확장 | directory entry 수가 기존 고정 제한을 넘을 수 있어야 하므로 |
| 계층 디렉터리 | `/a/b/c` 같은 path namespace를 지원하기 위해 |
| absolute/relative path | root 기준 경로와 cwd 기준 경로를 모두 지원하기 위해 |
| `.`, `..` | 현재 directory와 부모 directory 탐색을 지원하기 위해 |
| 프로세스별 cwd | 프로세스마다 상대 경로 기준이 달라질 수 있으므로 |
| fork 시 cwd 상속 | 자식 프로세스가 부모의 작업 디렉터리에서 시작해야 하므로 |
| directory fd | directory도 open/close/readdir/isdir/inumber 대상으로 다루기 위해 |
| `chdir`, `mkdir`, `readdir`, `isdir`, `inumber` | Project 4 subdirectory syscall 요구사항 |
| symlink | Project 4 soft link 요구사항 |
| 빈 directory 삭제 | 일반 파일뿐 아니라 빈 directory도 remove 대상이어야 하므로 |
| syscall buffer 검증 | kernel이 잘못된 user pointer를 믿으면 안 되므로 |
| 파일 시스템 동기화 | FAT, inode, directory 같은 공유 상태를 보호하기 위해 |

### 구현하지 않은 것

| 기능 | 구현하지 않은 이유 |
| --- | --- |
| Buffer cache | Project 4 extra credit |
| 64-sector cache 제한 | buffer cache 하위 요구사항 |
| clock replacement | buffer cache 하위 요구사항 |
| write-behind | buffer cache 하위 요구사항 |
| read-ahead | buffer cache 하위 요구사항 |
| bounce buffer 제거 | buffer cache 구현 시 요구되는 개선 |
| COW | Project 3 extra 기능 |
| VM 기반 filesys grading | 현재 검증은 `Grading.no-vm` 기준 |
| fine-grained filesystem concurrency | 현재는 correctness 중심의 coarse-grained lock 구현 |

## 16. 한 장으로 요약

파일 시스템을 처음 볼 때는 다음 흐름만 잡으면 된다.

```text
사용자는 path와 fd로 요청한다.
  |
  v
syscall.c가 요청을 검증하고 filesys로 넘긴다.
  |
  v
filesys.c가 path를 해석한다.
  |
  v
directory.c가 이름을 inode로 바꾼다.
  |
  v
file.c가 열린 파일/디렉터리 객체를 관리한다.
  |
  v
inode.c가 파일 길이와 데이터 위치를 관리한다.
  |
  v
fat.c가 cluster chain을 할당하고 연결한다.
  |
  v
disk sector에 실제 byte가 저장된다.
```

핵심 아이디어는 세 가지다.

1. 이름은 directory가 관리한다.
2. 파일의 실체와 길이는 inode가 관리한다.
3. 파일 데이터의 여러 cluster 연결은 FAT이 관리한다.

그래서 `open("/home/a.txt")` 같은 간단한 호출도 내부적으로는 다음 질문들을 차례로 해결한다.

```text
"/"에서 "home"은 어떤 inode인가?
"home"은 directory인가?
"/home"에서 "a.txt"는 어떤 inode인가?
"a.txt"는 일반 파일인가?
이 파일의 데이터는 어느 cluster에서 시작하는가?
offset N은 FAT chain의 몇 번째 cluster인가?
그 cluster는 실제 disk sector 몇 번인가?
```

Project 4 filesys 구현은 이 질문들에 답할 수 있도록 Pintos의 기존 단순 파일 시스템을 확장한 작업이다.
