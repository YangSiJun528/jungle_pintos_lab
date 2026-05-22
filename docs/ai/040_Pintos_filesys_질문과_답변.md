# Pintos filesys 질문과 답변

작성일: 2026-05-20

이 문서는 Pintos 파일 시스템을 공부하면서 나온 질문과 답변을 계속 축적하는 Q&A 문서다. 긴 설명 문서가 아니라, 나중에 다시 볼 수 있도록 질문의 의도와 핵심 답변을 매끄럽게 요약해 남기는 것을 목표로 한다.

## 운영 규칙

- 이 문서에는 Pintos filesys, FAT, inode, directory, path resolution, file growth, symlink, filesys syscall, filesys synchronization처럼 파일 시스템과 직접 관련된 내용만 남긴다.
- VM, thread, scheduler, userprog 같은 다른 프로젝트 내용은 filesys 이해에 직접 필요한 경우에만 짧게 언급한다.
- 답변은 원래 대화보다 압축해서 기록한다.
- 구현 판단은 로컬 reference 중 `docs/reference/pintos-kaist-original/4_project4`와 현재 구현 정리 문서인 `docs/ai/038_Pintos_filesys_구현_정리.md`를 우선한다.
- 초심자용 전체 흐름은 `docs/ai/039_Pintos_filesys_초심자_구현_흐름.md`를 참고한다.

## 질문과 답변

## Q1. sector 단위로 읽고 쓰는 이유는 하드웨어가 그렇게 지원하기 때문인가요? Pintos의 디스크 모델은 무엇이고, 드라이버 기능까지 수행하나요?

네. filesys가 sector 단위로 읽고 쓰는 가장 직접적인 이유는 Pintos가 다루는 디스크 장치 인터페이스가 sector 단위 I/O를 제공하기 때문이다. 현재 저장소의 `DISK_SECTOR_SIZE`는 512바이트이고, `disk_read()`와 `disk_write()`는 정확히 한 sector를 읽거나 쓰는 인터페이스다.

Pintos 안에서 디스크는 ATA/IDE 디스크처럼 보인다. 실제 실행은 QEMU/Bochs 같은 가상 머신 위에서 이루어지고, host의 `filesys.dsk` 같은 파일이 가상 디스크로 연결된다. 하지만 Pintos kernel 입장에서는 단순 파일이 아니라 legacy ATA 채널에 붙은 디스크 장치로 보이며, `devices/disk.c`가 이 장치를 조작한다.

계층은 다음처럼 나뉜다.

```text
filesys/file.c, inode.c
  - 파일 offset과 길이를 보고 필요한 disk sector를 결정한다.
  - disk_read(), disk_write()를 호출한다.

devices/disk.c
  - ATA/IDE 디스크 드라이버 역할을 한다.
  - I/O port에 ATA register 값을 쓰고 읽는다.
  - READ SECTOR, WRITE SECTOR command를 발행한다.
  - 디스크 interrupt를 기다린다.
  - data port를 통해 512바이트 sector를 전송한다.

가상 머신의 ATA 디스크
  - host의 disk image 파일을 실제 저장소처럼 제공한다.
```

따라서 Pintos의 filesys 과제에서 구현하는 것은 디스크 드라이버가 아니라 그 위의 파일 시스템이다. 드라이버 수준 기능은 이미 `devices/disk.c`가 수행한다. filesys는 sector 번호를 계산하고 `disk_read()`/`disk_write()`를 호출할 뿐, ATA port나 interrupt를 직접 다루지 않는다.

현재 구현에서 중요한 연결은 다음이다.

- `filesys_init()`은 `disk_get(0, 1)`로 파일 시스템용 디스크를 얻는다.
- Pintos의 디스크 번호 convention에서 `0:1`은 file system disk다.
- `inode.c`는 byte offset을 sector 번호로 바꾼 뒤 `disk_read()` 또는 `disk_write()`를 호출한다.
- `disk.c`는 내부적으로 channel lock, ATA command, interrupt wait, PIO data transfer를 처리한다.

정리하면, sector는 filesys가 마음대로 정한 단위가 아니라 디스크 장치 인터페이스가 제공하는 block I/O 단위다. Pintos에서는 `devices/disk.c`가 간단한 ATA/IDE 디스크 드라이버 역할을 하고, filesys는 그 위에서 파일과 디렉터리 의미를 구현한다.

## Q2. reopen과 duplicate의 차이는 무엇인가요?

범용적으로 `reopen`은 같은 파일을 "다시 여는 것"에 가깝고, `duplicate`는 이미 열려 있는 handle 또는 fd를 "복제하는 것"에 가깝다.

`reopen`은 보통 같은 underlying file/inode를 가리키는 새 open object를 만든다. 새 object는 독립적인 현재 위치, 상태 flag, directory read position을 갖는 쪽이 자연스럽다. 따라서 한쪽에서 `seek()`를 해도 다른 쪽의 position은 변하지 않는다. Pintos의 `file_reopen()`도 같은 inode에 대해 새 `struct file`을 만들며, 새 file position은 0으로 시작한다. `dir_reopen()`도 같은 directory inode를 다시 열지만 directory read position은 새로 시작한다.

`duplicate`는 범용 Unix/POSIX 의미에서는 fd 번호만 하나 더 만들고, 두 fd가 같은 open file description을 가리키게 하는 것이다. 그래서 `dup`, `dup2`로 만들어진 fd들은 서로 다른 fd 번호이지만 file offset과 status flag를 공유한다. 한 fd에서 `seek()`하면 다른 fd의 offset도 같이 바뀌는 것이 핵심이다.

다만 현재 저장소의 `file_duplicate()`는 Unix의 `dup2` 의미처럼 같은 open file description을 공유하는 구현은 아니다. 내부적으로는 `file_reopen()`처럼 같은 inode에 대한 새 `struct file`을 만들고, 그 순간의 `pos`와 `deny_write` 상태를 복사한다. 따라서 복제 직후 상태는 같지만 이후 position 변화는 자동 공유되지 않는다. 이 구현은 현재 `fork`에서 부모의 file descriptor와 executable handle을 자식 쪽에 별도 객체로 복제하기 위해 사용된다.

요약하면 다음과 같다.

| 개념 | 범용 의미 | 현재 Pintos 코드에서의 의미 |
| --- | --- | --- |
| `reopen` | 같은 파일을 새로 연다. 독립적인 open 상태를 가진다. | 같은 inode에 대한 새 `struct file` 또는 `struct dir`을 만들고 position은 새로 시작한다. |
| `duplicate` | 기존 fd를 복제한다. 보통 file offset/status flag를 공유한다. | `file_duplicate()`는 새 `struct file`을 만들고 현재 `pos`, `deny_write`를 복사한다. 이후 offset은 공유하지 않는다. |

따라서 "처음부터 독립적인 handle이 필요하면 reopen", "같은 열린 상태를 공유하는 fd alias가 필요하면 duplicate"라고 이해하면 된다. 단, 이 저장소의 `file_duplicate()`는 이름은 duplicate지만 POSIX `dup2`의 공유 offset semantics와는 다르다.

## Q3. filesys 구현을 계층적으로 보면 어떤 흐름인가요?

Pintos filesys는 아래에서 위로 쌓이는 계층으로 이해하면 좋다.

```text
disk.c
  -> sector 단위 read/write 제공

fat.c
  -> sector 위에 cluster chain과 FAT table 제공

inode.c
  -> FAT chain을 파일 offset과 파일 길이 관리로 변환

file.c
  -> 열린 파일 객체, 현재 위치, directory fd, read/write API 제공

syscall.c
  -> 사용자 syscall을 검증하고 fd를 file 객체로 바꿔 file/filesys API 호출
```

중요한 점은 `file.c`가 `fat.c`를 직접 사용하는 구조가 아니라는 것이다. `file.c`는 `inode_read_at()`과 `inode_write_at()`에 read/write를 위임하고, `inode.c`가 `fat_get()`, `fat_create_chain()`, `cluster_to_sector()` 같은 FAT API를 사용한다.

이 흐름을 자세히 설명한 별도 자료는 `docs/ai/041_Pintos_filesys_계층별_구현_흐름.md`에 정리했다.

## Q4. 실제 구현 코드에서 앞서 설명한 함수들은 어떻게 이어지나요?

대표적인 write 흐름은 실제 코드에서 다음처럼 이어진다.

```text
syscall.c: handle_write()
  -> fd_lookup(fd)
  -> file_write(file, buffer, size)

file.c: file_write()
  -> inode_write_at(file->inode, buffer, size, file->pos)
  -> file->pos += bytes_written

inode.c: inode_write_at()
  -> EOF를 넘으면 inode_extend()
  -> byte_to_sector()로 file offset을 disk sector로 변환
  -> disk_write()

inode.c: inode_extend()
  -> fat_create_chain()으로 부족한 cluster를 FAT chain 뒤에 붙임
  -> 새 cluster를 zero fill
  -> inode.length 갱신

fat.c
  -> fat_create_chain(), fat_get(), cluster_to_sector()로 cluster chain을 관리
```

즉 실제 코드에서도 `file.c`가 FAT을 직접 호출하지 않는다. `file.c`는 `inode.c`에 read/write를 맡기고, `inode.c`가 FAT API를 사용해서 파일 offset을 sector로 바꾸거나 파일을 확장한다.

실제 코드 조각은 `docs/ai/041_Pintos_filesys_계층별_구현_흐름.md`의 Disk, FAT, Inode, File, Syscall 각 계층 섹션 안에 배치했다. 별도 코드 모음 섹션으로 분리하지 않고, 각 함수 설명 바로 아래에서 구현과 사용 흐름을 확인할 수 있게 정리했다.
