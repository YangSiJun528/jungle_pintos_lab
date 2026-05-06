# Pintos filesys core 선행 가능 범위와 비중

## 요약

Pintos Project 4의 파일시스템 과제는 파일시스템을 완전히 처음부터 새로 만드는 것보다는, 제공된 단순 파일시스템을 확장해 실제 파일시스템에 가까운 기능을 구현하는 과제에 가깝다.

정규 점수 기준으로 보면 filesys core는 Project 4의 본체다. `Grading.with-vm` 기준으로 filesys 관련 정규 항목은 `filesys/base` 10%, `filesys/extended/functionality` 25%, `filesys/extended/robustness` 15%, `filesys/extended/persistence` 20%로 총 70%다. VM과 thread/userprog 회귀는 나머지 30%다. buffer cache는 정규 core가 아니라 extra 20%다.

결론적으로 VM 없이 먼저 진행 가능한 filesys core 작업은 Project 4의 절반 이상이며, 잘 진행하면 정규 점수의 60~70%에 해당하는 범위를 선행할 수 있다. 다만 syscall, fd table, current directory, fork/exit 정리와 연결되므로 `pintos/filesys` 디렉터리 안에서만 완전히 끝나는 독립 작업은 아니다.

## 기준 자료

- `docs/reference/pintos-kaist-kr/4_project4/0_introduction.md`
- `docs/reference/pintos-kaist-kr/4_project4/1_indexed_and_extensible_files.md`
- `docs/reference/pintos-kaist-kr/4_project4/2_subdirectories.md`
- `docs/reference/pintos-kaist-kr/4_project4/3_buffer_cache.md`
- `docs/reference/pintos-kaist-kr/4_project4/4_synchronization.md`
- `pintos/tests/filesys/Grading.with-vm`
- `pintos/tests/filesys/Grading.no-vm`
- `pintos/tests/filesys/base/Rubric`
- `pintos/tests/filesys/extended/Rubric.functionality`
- `pintos/tests/filesys/extended/Rubric.robustness`
- `pintos/tests/filesys/extended/Rubric.persistence`
- `pintos/tests/filesys/buffer-cache/Rubric`
- `pintos/filesys/Make.vars`

## Project 4에서 파일시스템이 차지하는 비중

`pintos/tests/filesys/Grading.with-vm` 기준 Project 4 정규 비중은 다음과 같다.

| 영역 | 비중 | 의미 |
|---|---:|---|
| threads 회귀 | 5% | Project 1 기능이 깨지지 않았는지 확인 |
| userprog 회귀 | 15% | Project 2 syscall과 robustness가 유지되는지 확인 |
| VM 회귀 | 10% | Project 3 VM 기능이 유지되는지 확인 |
| filesys/base | 10% | 기존 기본 파일 동작과 동시 접근 회귀 |
| filesys/extended/functionality | 25% | file growth, directory, symlink 등 핵심 기능 |
| filesys/extended/robustness | 15% | directory/path/remove edge case |
| filesys/extended/persistence | 20% | 재부팅 유사 흐름 뒤 디스크 내용 보존 |
| buffer cache | +20% extra | VM-enabled 설정에서 보는 extra credit |

따라서 정규 점수만 보면 filesys core는 총 70%다. buffer cache까지 포함하면 extra 20%가 추가되지만, 이는 VM/page cache와 강하게 연결되므로 선행 core와는 분리해서 보는 편이 좋다.

## 요구사항 큰 분류

### 1. FAT와 extensible file

기존 basic filesystem은 파일을 single extent, 즉 연속된 sector 범위에 할당한다. Project 4에서는 이 한계를 제거해야 한다.

KAIST reference는 이 부분에서 direct/indirect/doubly-indirect 방식이 아니라, 제공된 `filesys/fat.c` skeleton으로 FAT를 반드시 구현하라고 한다. 즉 file growth 설계는 FAT 기반이어야 한다.

주요 요구사항은 다음과 같다.

| 요구사항 | 내용 |
|---|---|
| FAT 초기화 | FAT layout, data start, cluster 수 관리 |
| cluster chain 관리 | 새 cluster 할당, chain 확장, chain 제거 |
| byte offset 변환 | 파일 offset을 실제 disk sector로 변환 |
| file growth | EOF 뒤 write 시 파일 확장 |
| sparse/gap 처리 | EOF와 write 위치 사이를 zero-fill처럼 읽히게 처리 |
| root directory growth | root directory도 초기 16개 제한을 넘어 성장 |

### 2. Subdirectories와 path

기존 basic filesystem은 root directory 하나만 가진다. Project 4에서는 계층형 namespace를 구현해야 한다.

주요 요구사항은 다음과 같다.

| 요구사항 | 내용 |
|---|---|
| directory inode 구분 | file inode와 directory inode를 구분할 metadata 필요 |
| absolute path | `/a/b/c` 같은 경로 처리 |
| relative path | current directory 기준 경로 처리 |
| `.`과 `..` | 자기 자신과 부모 directory entry 처리 |
| process별 cwd | 각 process가 별도 current directory를 유지 |
| fork cwd 상속 | child가 parent의 cwd를 상속하되 이후 독립적으로 관리 |
| directory open | `open`이 directory도 열 수 있어야 함 |
| directory remove | empty directory만 제거, root 제거 금지 |

새 syscall은 다음과 같다.

| syscall | 역할 |
|---|---|
| `chdir` | current directory 변경 |
| `mkdir` | directory 생성 |
| `readdir` | directory entry 읽기 |
| `isdir` | fd가 directory인지 확인 |
| `inumber` | fd가 가리키는 inode number 반환 |

### 3. Symlink

Project 4에는 symbolic link도 포함된다.

주요 요구사항은 다음과 같다.

| 요구사항 | 내용 |
|---|---|
| `symlink` syscall | target path 문자열을 담는 symbolic link 생성 |
| file symlink | symlink를 통해 regular file 접근 |
| directory symlink | symlink를 통해 directory 하위 경로 접근 |
| symlink chain | link가 다른 link를 가리키는 경우 처리 |
| loop 방어 | cycle에서 무한 path resolution이 나지 않게 제한 필요 |
| persistence | symlink target 문자열이 디스크에 안정적으로 남아야 함 |

### 4. Synchronization

Project 2에서는 보통 전역 `filesys_lock`으로 파일시스템 접근을 감쌌다. Project 4 reference는 더 fine-grained한 synchronization을 요구한다.

핵심은 다음과 같다.

| 요구사항 | 내용 |
|---|---|
| 독립 파일 동시 접근 | 서로 다른 파일 작업은 불필요하게 막지 않는 방향 |
| 같은 파일 read | 여러 read는 동시에 가능해야 함 |
| file extension atomicity | 파일 확장과 새 영역 write는 깨진 중간 상태가 보이면 안 됨 |
| directory 동시성 | 서로 다른 directory 작업은 병렬 가능해야 함 |
| lock ordering | path traversal, inode, directory, file lock 사이 deadlock 방지 |

### 5. Persistence

Project 4 filesys 테스트는 단순히 한 번 실행하고 끝나지 않는다. 테스트 후 두 번째 Pintos 실행에서 `tar fs.tar /`로 파일시스템 내용을 꺼내고, host 쪽 checker가 그 내용을 검증한다.

따라서 memory 안에서만 맞는 구현은 부족하다. inode metadata, FAT, directory entry, symlink target, file length 등이 disk에 일관되게 기록되어야 한다.

Persistence는 별도 기능처럼 보이지만 실제로는 앞의 구현 전체가 disk state로 올바르게 남는지 확인하는 마무리 검증이다.

## 선행 가능한 범위

VM 없이 먼저 진행 가능한 영역은 다음과 같다.

| 선행 작업 | 선행 가능성 | Project 4 체감 비중 |
|---|---:|---:|
| FAT layout과 cluster chain | 높음 | 10~15% |
| file growth와 sparse/gap 처리 | 높음 | 10~15% |
| inode metadata 설계 | 높음 | 여러 항목에 영향 |
| directory inode와 directory growth | 높음 | 10% 내외 |
| path parser 설계 | 높음 | 10% 내외 |
| `mkdir`, `chdir`, `readdir`, `isdir`, `inumber` 요구사항 연결 | 중간 | 10~15% |
| symlink 저장 구조와 path resolution | 중간 | 8~10% |
| persistence 대응 | 중간 | 최대 20%, 단 앞 구현 결과 성격 |
| fine-grained synchronization | 중간 | 10~15% |

합치면 VM 없이도 Project 4 정규 점수의 60~70%에 해당하는 핵심 구조를 먼저 밀 수 있다.

다만 `pintos/filesys` 내부만 고쳐서 완료되는 부분은 이보다 작다. syscall layer, fd table, `struct thread`의 current directory, fork/exit cleanup과 연결되는 부분이 있기 때문이다. 순수 filesys 디렉터리 중심 작업으로 좁히면 체감상 40~50% 정도로 보는 편이 현실적이다.

## 다른 파트와의 의존성

### Project 2 userprog/syscall

파일 이름을 받는 기존 syscall은 Project 4에서 path를 받아야 한다.

영향을 받는 syscall은 다음과 같다.

- `create`
- `remove`
- `open`
- `read`
- `write`
- `seek`
- `tell`
- `close`
- `filesize`

특히 `open`은 regular file뿐 아니라 directory도 열 수 있어야 한다. 그러면 fd table은 단순히 `struct file *`만 담는 구조로는 부족할 수 있다. fd entry가 file인지 directory인지 구분할 수 있어야 하고, `close`는 둘 다 처리해야 한다.

### Process/thread 상태

Project 4는 process별 current directory를 요구한다. 따라서 `struct thread` 또는 process 관련 구조에 cwd reference가 필요하다.

주의할 점은 다음과 같다.

- initial process의 cwd는 root로 시작한다.
- fork 시 child는 parent cwd를 상속한다.
- parent와 child의 cwd는 이후 독립적으로 관리된다.
- process exit 시 cwd reference를 닫아야 한다.
- remove된 directory를 cwd로 쓰는 정책을 정하고 일관되게 처리해야 한다.

### Project 3 VM

Project 4는 Project 2 위에서도 빌드할 수 있지만, 최종 with-VM 경로에서는 Project 3 기능도 계속 통과해야 한다.

특히 VM 쪽은 file-backed page와 `mmap`에서 file API를 사용한다. 따라서 filesys core 구현 중 다음 의미를 깨면 VM 통합에서 문제가 생길 수 있다.

- `file_read_at`
- `file_write_at`
- `file_reopen`
- `file_close`
- `filesys_remove`의 open file 처리
- executable deny-write
- file length와 offset의 일관성

### Buffer cache extra

Buffer cache는 Project 4 extra다. Reference는 VM subsystem을 통해 page cache를 구현할 수 있다고 설명하고, 제공된 `page_cache.c` 템플릿도 VM 기능을 전제로 한다.

따라서 buffer cache는 filesys core를 먼저 끝낸 뒤, VM-enabled 설정에서 별도 단계로 보는 것이 자연스럽다.

## 추천 진행 순서

1. `filesys/base` 회귀 유지

   Project 4 구현을 시작해도 기존 작은/큰 파일 read/write, full 상황, 동시 read/write/remove가 깨지지 않아야 한다. base가 깨진 상태에서 extended로 넘어가면 원인 분리가 어려워진다.

2. FAT와 file growth

   Project 4의 가장 밑바닥이다. 파일과 directory 모두 결국 inode와 data cluster 할당 위에서 동작한다. FAT chain, file length, zero-fill semantics를 먼저 안정화하는 것이 좋다.

3. Directory와 path

   directory inode 구분, `.`/`..`, absolute/relative path, cwd를 붙인다. 이 단계부터 syscall/thread와 본격적으로 연결된다.

4. 새 syscall과 fd table 확장

   `mkdir`, `chdir`, `readdir`, `isdir`, `inumber`를 user-visible syscall로 연결한다. fd table은 file과 directory를 모두 표현할 수 있어야 한다.

5. Symlink

   target path 문자열 저장, symlink chain resolution, loop 방어를 추가한다. path parser와 강하게 연결되므로 directory/path가 잡힌 뒤 하는 편이 낫다.

6. Synchronization 정리

   전역 lock만으로는 요구사항을 만족하기 어렵다. inode/directory/cache lock ordering을 정리하고, file growth atomicity를 확인한다.

7. Persistence

   원본 테스트가 통과한 뒤 `*-persistence`를 본다. 이 단계는 on-disk metadata 기록 시점, close/shutdown flush, remove lifetime 문제를 잡는 단계다.

8. VM 통합과 buffer cache extra

   `filesys/Make.vars`에서 VM 관련 줄을 켠 뒤 Project 3 VM 회귀와 Project 4 filesys를 함께 확인한다. buffer cache는 이 다음 extra로 분리한다.

## 테스트 비중으로 본 작업 묶음

`pintos/tests/filesys/extended/Rubric.functionality`의 raw 점수 기준으로 보면 기능별 무게는 다음과 같다.

| 기능 묶음 | raw 점수 | 의미 |
|---|---:|---|
| directory support | 13 | `dir-mkdir`, `dir-mk-tree`, `dir-rmdir`, `dir-rm-tree`, `dir-vine` |
| file growth | 13 | `grow-create`, `grow-seq-*`, `grow-sparse`, `grow-two-files`, `grow-tell`, `grow-file-size` |
| directory growth | 3 | root/subdirectory가 많은 entry를 담을 수 있는지 |
| synchronization | 5 | `syn-rw` |
| symlink | 15 | `symlink-file`, `symlink-dir`, `symlink-link` |

Symlink raw 점수가 커 보이지만, symlink는 path parser와 directory 구현이 되어 있어야 제대로 붙는다. 독립 첫 작업으로 잡기보다는 directory/path 이후에 붙이는 편이 안정적이다.

## 핵심 판단

Filesys core는 선행할 가치가 매우 크다. Project 4 정규 점수 70%를 차지하고, VM 없이도 상당 부분 구현과 설계가 가능하다.

다만 완전 독립 파트는 아니다. 특히 directory fd, cwd, path syscall, fork/exit cleanup, VM의 file-backed page와 연결된다. 따라서 선행 구현을 할 때는 `file_*`, `inode_*`, `filesys_*` API의 의미를 자주 바꾸기보다, Project 2 syscall과 Project 3 VM이 기대하는 동작을 유지하는 방향으로 보수적으로 설계해야 한다.
