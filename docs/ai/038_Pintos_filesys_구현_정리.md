# Pintos filesys 구현 정리

작성일: 2026-05-19

이 문서는 현재 `feat/filesys` 브랜치의 filesys 구현을 기준으로, KAIST Pintos Project 4 요구사항을 어떤 방식으로 구현했는지와 관련 개념을 정리한다.

## 기준 문서와 범위

요구사항 판단 기준은 로컬 reference 중 `docs/reference/pintos-kaist-original/4_project4` 계열 문서다.

- `0_introduction.md`: Project 4의 전체 범위와 persistence test 방식
- `1_indexed_and_extensible_files.md`: FAT 기반 indexed/extensible files, file growth
- `2_subdirectories.md`: subdirectory, current working directory, 추가 syscall, symlink
- `4_synchronization.md`: 파일시스템 동시 접근 요구사항

현재 구현은 `pintos/filesys/Make.vars`의 `Grading.no-vm` 설정으로 검증했다. VM 기반 buffer cache는 `3_buffer_cache.md`에서 extra credit으로 분류되어 있으므로 구현하지 않았다. Project 3의 COW도 extra 범위이므로 구현하지 않았다.

## 검증 결과

다음 Docker 환경에서 filesys 전체 테스트를 실행했다.

```sh
docker run --rm --platform=linux/amd64 \
  -v "$PWD:/workspace" \
  -w /workspace \
  pintos-dev:22.04 \
  bash -lc 'source /workspace/pintos/activate; make -C /workspace/pintos/filesys clean; make -C /workspace/pintos/filesys check'
```

결과:

```text
All 146 tests passed.
```

관련 커밋:

- `990c70c Implement FAT-backed filesys base support`
- `ce8d7ad Validate writable syscall buffers`

## 전체 구조

filesys 구현은 기존 단일 extent 기반 파일 저장 방식을 FAT 기반 chain 저장 방식으로 바꾸고, 그 위에 디렉터리와 경로 해석을 얹는 구조다.

주요 변경 파일:

- `pintos/filesys/fat.c`, `pintos/include/filesys/fat.h`
- `pintos/filesys/inode.c`, `pintos/include/filesys/inode.h`
- `pintos/filesys/filesys.c`, `pintos/include/filesys/filesys.h`
- `pintos/filesys/directory.c`, `pintos/include/filesys/directory.h`
- `pintos/filesys/file.c`, `pintos/include/filesys/file.h`
- `pintos/userprog/syscall.c`
- `pintos/userprog/process.c`
- `pintos/include/threads/thread.h`

## FAT 기반 파일 배치

### 요구사항

기준 문서는 기존 Pintos의 contiguous extent 방식이 external fragmentation에 취약하므로, KAIST skeleton의 FAT 방식으로 파일 block을 indexing하라고 요구한다. 멀티레벨 inode indexing, 즉 FFS 스타일 direct/indirect/doubly indirect 구현은 금지되어 있다.

### 구현 방식

`fat.c`에 FAT 메타데이터와 chain 조작을 구현했다.

- `fat_fs_init()`:
  - FAT 영역 뒤의 첫 sector를 data 영역 시작점으로 계산한다.
  - disk 크기에서 data 영역 cluster 개수를 계산해 `fat_length`와 `last_clst`를 설정한다.
  - FAT 변경 보호용 `write_lock`을 초기화한다.
- `fat_create_chain(clst)`:
  - 비어 있는 FAT entry를 찾아 새 cluster를 할당한다.
  - `clst == 0`이면 새 chain을 만들고, 아니면 기존 chain 끝에 이어 붙인다.
  - 새 cluster는 `EOChain`으로 표시한다.
- `fat_remove_chain(clst, pclst)`:
  - `clst`부터 이어지는 FAT chain을 free 상태로 되돌린다.
  - `pclst`가 있으면 `pclst`를 새 chain 끝으로 만든다.
- `cluster_to_sector()` / `sector_to_cluster()`:
  - cluster 번호와 실제 disk sector 번호를 변환한다.

### 관련 개념

FAT(File Allocation Table)는 "이 cluster 다음 cluster가 어디인지"를 별도의 배열에 저장하는 방식이다. 파일 inode에는 첫 cluster만 저장하고, 나머지는 FAT entry를 따라가며 찾는다. 각 cluster는 현재 skeleton 기준 `SECTORS_PER_CLUSTER == 1`이라 disk sector 하나와 대응된다.

이 방식은 한 파일의 block들이 disk에 연속으로 붙어 있을 필요가 없어서 external fragmentation 문제를 피한다. 대신 임의 offset 접근 시 FAT chain을 따라가야 하므로, chain 탐색 비용이 생긴다.

## Inode와 확장 파일

### 요구사항

기준 문서는 파일이 생성 시점의 크기에 고정되지 않고, EOF 이후 write로 확장되어야 한다고 요구한다. seek 자체는 파일을 확장하지 않고, EOF를 넘긴 위치에 write하면 그 사이의 gap은 0으로 채워져야 한다.

### 구현 방식

`inode.c`의 on-disk inode 구조를 FAT에 맞게 바꿨다.

```c
struct inode_disk {
  cluster_t start;
  off_t length;
  unsigned magic;
  uint32_t type;
  uint32_t unused[124];
};
```

- `start`: 파일 데이터의 첫 cluster
- `length`: 파일 길이
- `type`: 일반 파일, 디렉터리, symlink 구분

핵심 함수:

- `byte_to_sector()`:
  - file offset을 sector로 바꾼다.
  - offset / sector size 만큼 FAT chain을 따라가서 해당 cluster를 찾는다.
- `inode_create_typed()`:
  - 일반 파일, 디렉터리, symlink용 inode를 공통 생성한다.
  - 초기 크기가 0이면 data cluster 없이 inode만 만든다.
  - 초기 크기가 있으면 필요한 cluster를 FAT chain으로 할당하고 0으로 초기화한다.
- `inode_extend()`:
  - write가 기존 EOF를 넘어서면 필요한 cluster를 추가 할당한다.
  - 새로 할당한 cluster는 0으로 초기화한다.
  - 중간에 할당 실패가 나면 이번 확장에서 새로 붙인 chain을 되돌린다.
- `inode_write_at()`:
  - `offset + size`가 현재 파일 길이를 넘으면 먼저 `inode_extend()`를 호출한다.
  - 이후 기존 sector 단위 write 로직으로 실제 데이터를 쓴다.

### 관련 개념

파일의 "길이"와 "할당된 block 수"는 다르다. 현재 구현은 sparse file을 특별히 압축 저장하지 않고, EOF를 넘어 write할 때 gap 구간에 해당하는 cluster도 실제로 할당하고 0으로 채운다. 기준 문서는 sparse 방식과 실제 zero block 할당 방식 중 어느 쪽이든 허용한다.

## 계층 디렉터리와 경로 해석

### 요구사항

기준 문서는 단일 root directory만 있는 구조를 계층 namespace로 바꾸고, `/` 구분자를 쓰는 absolute/relative path를 지원하라고 요구한다. 또한 `.`과 `..`를 지원하고, 각 프로세스가 독립적인 current working directory를 가져야 한다.

### 구현 방식

`directory.c`의 directory는 inode를 backing store로 사용하는 파일이다. directory file 안에는 `struct dir_entry` record가 연속으로 저장된다.

- `dir_create_with_parent(sector, parent_sector)`:
  - 디렉터리 inode를 만든 뒤 `.`과 `..` entry를 추가한다.
  - root directory는 parent도 자기 자신으로 설정한다.
- `dir_add()`:
  - 같은 이름이 없는지 확인하고, 빈 slot 또는 EOF 위치에 새 entry를 쓴다.
  - directory도 일반 파일처럼 inode write를 사용하므로 필요 시 확장된다.
- `dir_readdir()`:
  - `.`과 `..`는 사용자에게 반환하지 않는다.
- `dir_is_empty()`:
  - `.`과 `..` 외 entry가 없는지 확인한다.

경로 해석은 `filesys.c`에서 처리한다.

- absolute path는 root directory에서 시작한다.
- relative path는 현재 thread의 `cwd`에서 시작한다.
- `resolve_path()`는 전체 경로를 inode로 해석한다.
- `resolve_parent()`는 생성, 삭제, symlink 생성처럼 마지막 component의 부모 directory와 basename이 필요한 작업에 사용한다.
- component 길이는 기존 Pintos의 `NAME_MAX` 제한을 유지한다. 전체 path는 component를 나누어 처리하므로 14자를 넘는 전체 path를 사용할 수 있다.

### 관련 개념

디렉터리는 특별한 형태의 파일이다. 파일 내용이 byte 배열이라면, directory 내용은 "이 이름은 이 inode sector를 가리킨다"는 entry 배열이다. `.`은 자기 자신, `..`은 부모 directory를 가리키는 entry라서 경로 탐색 중 위아래 이동을 같은 lookup 로직으로 처리할 수 있다.

## 프로세스별 current working directory

### 요구사항

기준 문서는 프로세스마다 별도의 current directory를 유지하고, fork 시 자식이 부모의 current directory를 상속하되 이후에는 독립적으로 움직여야 한다고 요구한다.

### 구현 방식

`struct thread`에 `struct dir *cwd`를 추가했다.

- `process_init()`:
  - 초기 프로세스의 `cwd`가 없으면 root directory로 설정한다.
- fork 경로:
  - 부모의 `cwd`가 있으면 `dir_reopen()`으로 같은 directory inode를 참조하는 새 `struct dir`을 만든다.
  - 부모와 자식은 같은 inode를 보지만 `struct dir` 객체는 별도라 directory read position 같은 상태는 공유하지 않는다.
- `process_cleanup_files()`:
  - 프로세스 종료 시 `cwd`를 닫는다.
- `filesys_chdir()`:
  - 경로가 directory로 resolve되는지 확인하고, 성공하면 현재 thread의 `cwd`를 교체한다.

### 관련 개념

current working directory는 "상대 경로의 시작점"이다. 같은 directory를 가리키더라도 프로세스별 handle은 독립적이어야 하므로, fork 시 포인터를 그대로 공유하지 않고 reopen한 directory 객체를 둔다.

## 파일 descriptor에서 directory 지원

### 요구사항

기준 문서는 `open()`이 directory도 열 수 있어야 하며, 기존 syscall 중 `close()`는 directory fd도 받아들여야 한다고 요구한다. 새 syscall인 `readdir`, `isdir`, `inumber`도 필요하다.

### 구현 방식

`struct file`에 directory 여부를 나타내는 `struct dir *dir` 필드를 추가했다.

- `file_open()`:
  - inode type이 directory이면 `dir_open()`으로 directory handle을 만든다.
  - 일반 파일이면 기존처럼 inode를 직접 보관한다.
- `file_is_dir()`:
  - fd가 directory인지 판별한다.
- `file_readdir()`:
  - directory fd에 대해서만 `dir_readdir()`을 호출한다.
- `file_read()` / `file_write()`:
  - directory fd에 대한 일반 read/write는 실패하도록 한다.
- `file_close()`:
  - directory file이면 `dir_close()`, 일반 file이면 `inode_close()`를 호출한다.

`syscall.c`에는 다음 handler를 추가했다.

- `chdir(const char *dir)`
- `mkdir(const char *dir)`
- `readdir(int fd, char *name)`
- `isdir(int fd)`
- `inumber(int fd)`
- `symlink(const char *target, const char *linkpath)`

### 관련 개념

Pintos의 fd table은 원래 "열린 파일"만 가리키지만, Unix 계열 인터페이스에서는 directory도 open 가능한 object다. 그래서 `struct file`을 "일반 파일 또는 directory handle"을 감싸는 공통 descriptor object로 확장했다.

## 삭제 규칙

### 요구사항

기준 문서는 일반 파일뿐 아니라 빈 directory도 삭제 가능해야 한다고 요구한다. root directory는 삭제되면 안 되고, directory가 비어 있지 않으면 삭제하면 안 된다.

### 구현 방식

`filesys_remove()`는 다음 순서로 동작한다.

1. `resolve_parent()`로 parent directory와 basename을 찾는다.
2. `.` 또는 `..` 삭제 요청은 거부한다.
3. 대상 inode가 root directory이면 거부한다.
4. 대상이 directory이면 `dir_is_empty()`로 `.`과 `..` 외 entry가 없는지 확인한다.
5. directory가 다른 곳에서 열려 있으면 `inode_open_count()` 기준으로 삭제를 거부한다.
6. 조건을 통과하면 parent directory entry를 지우고 inode를 removed 상태로 표시한다.

실제 FAT chain 반환은 마지막 opener가 inode를 닫는 `inode_close()` 시점에 수행된다.

### 관련 개념

directory entry를 지우는 것과 inode/data block을 즉시 해제하는 것은 다르다. 열려 있는 파일이 삭제되어도 기존 opener가 닫기 전까지 데이터 구조를 유지해야 하는 경우가 있으므로, `inode_remove()`는 removed flag만 세우고 마지막 close에서 실제 cluster를 반환한다.

## Symlink

### 요구사항

KAIST Project 4 문서는 symbolic link를 요구한다. symlink는 target path 문자열을 담는 pseudo file이며, open/read 시 target을 따라간 것처럼 동작해야 한다.

### 구현 방식

`INODE_SYMLINK` inode type을 추가했다.

- `filesys_symlink(target, linkpath)`:
  - `linkpath`의 parent directory를 찾는다.
  - 새 inode를 `INODE_SYMLINK`로 만들고, target 문자열을 inode data에 저장한다.
  - parent directory에 link name을 entry로 추가한다.
- `resolve_path_from()`:
  - 경로 탐색 중 symlink inode를 만나면 target 문자열을 읽어 다시 resolve한다.
  - 중간 component의 symlink는 항상 follow한다.
  - final component는 caller가 `follow_final`을 true로 준 경우에 follow한다.
  - 재귀 깊이는 `SYMLINK_MAX_DEPTH`로 제한해서 순환 link로 인한 무한 탐색을 막는다.

### 관련 개념

Hard link와 달리 symlink는 inode 자체가 target 파일을 직접 공유하지 않는다. symlink inode에는 단순히 문자열 path가 저장된다. 따라서 target이 없어지거나 이동하면 dangling symlink가 될 수 있다. 현재 구현도 path resolution 시점에 target을 다시 찾아간다.

## 사용자 syscall과 버퍼 검증

### 요구사항

Project 2의 syscall 안정성 요구사항은 filesys에서도 유지된다. 커널은 사용자 포인터가 유효한지 확인해야 하며, 특히 커널이 사용자 버퍼에 쓰는 syscall은 해당 페이지가 writable인지 확인해야 한다.

### 구현 방식

`syscall.c`에서 filesys syscall이 `filesys_*` 계층으로 연결된다. 파일시스템 진입 전후에는 `filesys_lock`을 사용해 공유 자료구조 접근을 보호한다.

`ce8d7ad` 커밋에서는 사용자 버퍼 검증을 read/write 방향에 맞게 보강했다.

- `read(fd, buffer, size)`:
  - 커널이 사용자 `buffer`에 쓰므로 writable buffer여야 한다.
- `readdir(fd, name)`:
  - 커널이 사용자 `name`에 파일명을 쓰므로 writable buffer여야 한다.
- `write(fd, buffer, size)`:
  - 커널이 사용자 `buffer`에서 읽기만 하므로 readable이면 된다.

VM build에서는 supplemental page table의 `page->writeable`과 PML4 entry의 writable bit를 확인한다. 이 검사는 COW를 구현하지 않고도 "쓰기 대상 버퍼가 실제로 writable이어야 한다"는 기본 syscall 안전성을 맞추기 위한 것이다.

### 관련 개념

사용자 주소는 커널이 신뢰하면 안 된다. 주소가 user range에 있는지, mapping이 존재하는지, 그리고 쓰기 대상이면 writable mapping인지 확인해야 한다. 읽기용 포인터와 쓰기용 포인터의 조건은 다르다.

## 동기화

### 요구사항

기준 문서는 파일시스템 내부에서 동시 접근을 안전하게 처리해야 하며, 가능하면 독립적인 파일/디렉터리 작업은 서로 기다리지 않아야 한다고 설명한다.

### 구현 방식

현재 구현은 정확성을 우선해 coarse-grained locking을 사용한다.

- `filesys_lock`:
  - syscall handler, process cleanup, load path 등 파일시스템 공유 자료구조에 접근하는 구간을 직렬화한다.
- `fat_fs->write_lock`:
  - FAT entry 할당과 반환을 보호한다.

이 구조는 테스트 통과와 자료구조 일관성에는 충분하지만, reference가 이상적으로 요구하는 fine-grained concurrency를 성능 관점에서 완전히 구현한 것은 아니다. 예를 들어 서로 다른 directory 작업도 syscall 경계의 `filesys_lock` 때문에 동시에 진행되지 않는다.

### 관련 개념

파일시스템에는 FAT table, open inode list, directory file, inode metadata처럼 여러 thread가 동시에 건드릴 수 있는 공유 상태가 많다. coarse-grained lock은 구현이 단순하고 일관성 확보가 쉽지만 병렬성이 낮다. fine-grained lock은 병렬성을 높일 수 있지만 lock 순서, deadlock, atomic file extension 같은 문제가 더 어려워진다.

## 구현하지 않은 extra 기능

다음 기능은 현재 범위에서 제외했다.

- Buffer Cache:
  - `docs/reference/pintos-kaist-original/4_project4/3_buffer_cache.md`에 extra credit으로 명시되어 있다.
  - 따라서 64-sector cache, clock replacement, write-behind, read-ahead는 구현하지 않았다.
- COW:
  - `docs/reference/pintos-kaist-original/3_project3/6_cow.md`의 Project 3 extra 기능이다.
  - 현재 filesys 목표와 무관하므로 구현하지 않았다.
- VM 기반 filesys grading:
  - 현재 `pintos/filesys/Make.vars`는 `Grading.no-vm` 설정이다.
  - filesys 검증은 no-vm 146개 테스트 기준으로 수행했다.

## 요구사항별 요약

| 요구사항 | 구현 위치 | 현재 상태 |
| --- | --- | --- |
| FAT 기반 cluster chain | `fat.c`, `inode.c` | 구현 |
| 파일 확장 | `inode_write_at()`, `inode_extend()` | 구현 |
| EOF 이후 write gap zero fill | 새 cluster 0 초기화 | 구현 |
| root directory 확장 | directory도 inode write로 확장 | 구현 |
| absolute/relative path | `filesys.c` path resolver | 구현 |
| `.`, `..` | `dir_create_with_parent()` | 구현 |
| 프로세스별 cwd | `thread.h`, `process.c`, `filesys_chdir()` | 구현 |
| directory open/close | `file.c` | 구현 |
| `mkdir` | `filesys_mkdir()`, syscall handler | 구현 |
| `readdir` | `file_readdir()`, `dir_readdir()` | 구현 |
| `isdir` | `file_is_dir()` | 구현 |
| `inumber` | `inode_get_inumber()` | 구현 |
| `symlink` | `filesys_symlink()`, path resolver | 구현 |
| 빈 directory 삭제 | `filesys_remove()`, `dir_is_empty()` | 구현 |
| 파일시스템 동기화 | `filesys_lock`, FAT write lock | correctness 중심 구현 |
| Buffer cache | extra | 미구현 |
| COW | Project 3 extra | 미구현 |
