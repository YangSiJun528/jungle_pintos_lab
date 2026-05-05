# filesys와 file API 가벼운 명세

이 문서는 Pintos Project 2에서 file 관련 system call을 구현할 때 자주
사용하는 `filesys.h`와 `file.h` API를 가볍게 정리한다.

기준은 현재 저장소의 header와 CS330 KAIST Pintos reference다. 구현 코드나
새 자료구조 설계는 다루지 않는다.

## 기준 자료

- `pintos/include/filesys/filesys.h`
- `pintos/include/filesys/file.h`
- `docs/reference/pintos-kaist-original/2_project2/0_introduction.md`
- `docs/reference/pintos-kaist-original/2_project2/3_system_call.md`

Reference 주의:

- Project 2에서는 file system code를 수정할 필요가 없고, 수정하지 않는 것을
  권장한다.
- file system code는 내부 동기화가 없으므로, system call layer에서 file
  system 접근을 critical section으로 다뤄야 한다.
- Project 2의 기본 file system은 file growth를 구현하지 않는다.

## 큰 그림

`filesys.h`와 `file.h`는 역할이 다르다.

| Header | 역할 | 대표 사용 위치 |
|---|---|---|
| `filesys.h` | file name을 기준으로 file system에 접근 | `create`, `open`, `remove` |
| `file.h` | 이미 열린 `struct file *`를 조작 | `read`, `write`, `filesize`, `seek`, `tell`, `close` |

흐름은 보통 다음과 같다.

1. `filesys_open(name)`으로 file name을 열어 `struct file *`를 얻는다.
2. process별 fd table에 `fd -> struct file *` mapping을 저장한다.
3. 이후 `read/write/seek/tell/close`는 fd table에서 `struct file *`를 찾아
   `file_*` API를 호출한다.

## filesys.h

`filesys.h`는 file system 전체에 대한 name-based API다.

### `filesys_init`

| 항목 | 내용 |
|---|---|
| Signature | `void filesys_init (bool format)` |
| 목적 | file system module 초기화 |
| 입력 | `format`: true이면 file system을 format |
| 반환 | 없음 |
| Project 2 syscall 연결 | 직접 syscall 구현에서 호출하는 API는 아님 |

일반적으로 Pintos boot/init 경로에서 사용된다. Project 2 syscall 구현자가
직접 호출할 대상은 아니다.

### `filesys_done`

| 항목 | 내용 |
|---|---|
| Signature | `void filesys_done (void)` |
| 목적 | file system 종료 처리, 아직 쓰지 않은 data를 disk에 반영 |
| 입력 | 없음 |
| 반환 | 없음 |
| Project 2 syscall 연결 | 직접 syscall 구현에서 호출하는 API는 아님 |

일반적인 process 종료나 fd close에 쓰는 함수가 아니다. fd close는
`file_close()`가 담당한다.

### `filesys_create`

| 항목 | 내용 |
|---|---|
| Signature | `bool filesys_create (const char *name, off_t initial_size)` |
| 목적 | `name`이라는 file을 `initial_size` 크기로 생성 |
| 입력 | file name, 초기 file size |
| 반환 | 성공 시 `true`, 실패 시 `false` |
| Project 2 syscall 연결 | `create(const char *file, unsigned initial_size)` |

주의:

- file을 생성할 뿐 open하지 않는다.
- 생성한 file을 사용하려면 별도 `open` syscall이 필요하다.
- file name pointer는 user pointer이므로 syscall layer에서 검증해야 한다.

### `filesys_open`

| 항목 | 내용 |
|---|---|
| Signature | `struct file *filesys_open (const char *name)` |
| 목적 | `name`이라는 file을 open |
| 입력 | file name |
| 반환 | 성공 시 새 `struct file *`, 실패 시 `NULL` |
| Project 2 syscall 연결 | `open(const char *file)` |

fd table과의 관계:

- `filesys_open()`은 user-visible fd 번호를 만들지 않는다.
- syscall layer가 반환된 `struct file *`를 process fd table에 등록하고,
  새 fd 번호를 user program에 반환해야 한다.
- `open`은 fd `0`, `1`을 반환하면 안 된다. 이 둘은 console용 예약 번호다.

### `filesys_remove`

| 항목 | 내용 |
|---|---|
| Signature | `bool filesys_remove (const char *name)` |
| 목적 | `name`이라는 file 삭제 |
| 입력 | file name |
| 반환 | 성공 시 `true`, 실패 시 `false` |
| Project 2 syscall 연결 | `remove(const char *file)` |

주의:

- reference는 open file도 remove될 수 있다고 설명한다.
- remove가 open fd를 자동으로 close하는 것은 아니다.
- 이미 열린 fd는 file name이 사라진 뒤에도 계속 사용할 수 있어야 한다.

## file.h

`file.h`는 이미 열린 file object인 `struct file *`를 다루는 API다.

`struct file`은 header에서 opaque type처럼 사용된다. syscall layer는 내부
field에 직접 접근하지 말고 `file_*` API를 통해 조작하는 것이 맞다.

### `file_open`

| 항목 | 내용 |
|---|---|
| Signature | `struct file *file_open (struct inode *)` |
| 목적 | inode를 받아 새 open file object 생성 |
| 입력 | inode pointer |
| 반환 | 성공 시 `struct file *`, 실패 시 `NULL` |
| Project 2 syscall 연결 | 보통 직접 사용하지 않음 |

일반 syscall 구현에서는 file name을 기준으로 여는 `filesys_open()`을 쓰는
쪽이 자연스럽다.

### `file_reopen`

| 항목 | 내용 |
|---|---|
| Signature | `struct file *file_reopen (struct file *)` |
| 목적 | 같은 inode에 대한 새 open file object 생성 |
| 입력 | 기존 open file object |
| 반환 | 성공 시 새 `struct file *`, 실패 시 `NULL` |
| Project 2 syscall 연결 | 특수한 재open이 필요할 때 고려 |

새 file object는 기존 file과 같은 inode를 가리키지만, 별도 open object다.

### `file_duplicate`

| 항목 | 내용 |
|---|---|
| Signature | `struct file *file_duplicate (struct file *file)` |
| 목적 | file object의 attributes를 포함해 복제 |
| 입력 | 기존 open file object |
| 반환 | 성공 시 새 `struct file *`, 실패 시 `NULL` |
| Project 2 syscall 연결 | `fork`에서 fd resource duplicate |

현재 저장소의 `process.c` TODO는 fork에서 file object 복제에
`file_duplicate()`를 사용하라고 힌트를 준다.

Project 2 reference의 `fork` 요구사항은 child가 file descriptor를 포함한
resource를 duplicated해야 한다고 설명한다. 따라서 fd table을 설계할 때
나중에 `file_duplicate()` 실패를 처리할 수 있어야 한다.

### `file_close`

| 항목 | 내용 |
|---|---|
| Signature | `void file_close (struct file *)` |
| 목적 | open file object 닫기 |
| 입력 | open file object |
| 반환 | 없음 |
| Project 2 syscall 연결 | `close(fd)`, process exit cleanup |

사용 시점:

- `close(fd)`에서 fd table entry가 가진 `struct file *`를 닫을 때
- process가 exit/terminate되어 열린 fd를 모두 암묵적으로 close할 때
- `open` 이후 fd table 등록 실패 등으로 file object를 버려야 할 때

### `file_get_inode`

| 항목 | 내용 |
|---|---|
| Signature | `struct inode *file_get_inode (struct file *)` |
| 목적 | file object가 감싼 inode 반환 |
| 입력 | open file object |
| 반환 | inode pointer |
| Project 2 syscall 연결 | 일반 syscall 구현에서는 보통 직접 필요 없음 |

Project 2 기본 syscall 구현에서는 대부분 `file_read/write/seek/tell/length`
수준에서 충분하다.

### `file_read`

| 항목 | 내용 |
|---|---|
| Signature | `off_t file_read (struct file *, void *, off_t)` |
| 목적 | file의 current position부터 읽기 |
| 입력 | open file object, kernel buffer, size |
| 반환 | 실제 읽은 byte 수 |
| 상태 변화 | 읽은 byte 수만큼 file position 증가 |
| Project 2 syscall 연결 | regular file 대상 `read(fd, buffer, size)` |

주의:

- user buffer 검증과 kernel/user memory copy 정책은 syscall layer 책임이다.
- fd `0` stdin read는 `file_read()`가 아니라 `input_getc()` 경로다.

### `file_read_at`

| 항목 | 내용 |
|---|---|
| Signature | `off_t file_read_at (struct file *, void *, off_t size, off_t start)` |
| 목적 | file의 특정 offset부터 읽기 |
| 입력 | open file object, buffer, size, 시작 offset |
| 반환 | 실제 읽은 byte 수 |
| 상태 변화 | file current position은 바꾸지 않음 |
| Project 2 syscall 연결 | 기본 `read` syscall에는 보통 `file_read()`가 대응 |

`read(fd, ...)`의 reference 의미는 fd의 current position을 사용하는 것이므로,
일반 구현에서는 `file_read()`가 더 직접적으로 맞는다.

### `file_write`

| 항목 | 내용 |
|---|---|
| Signature | `off_t file_write (struct file *, const void *, off_t)` |
| 목적 | file의 current position부터 쓰기 |
| 입력 | open file object, kernel buffer, size |
| 반환 | 실제 쓴 byte 수 |
| 상태 변화 | 쓴 byte 수만큼 file position 증가 |
| Project 2 syscall 연결 | regular file 대상 `write(fd, buffer, size)` |

주의:

- fd `1` stdout write는 `file_write()`가 아니라 `putbuf()` 경로다.
- Project 2 기본 file system은 file growth를 구현하지 않으므로, EOF 이후
  write는 제한될 수 있다.

### `file_write_at`

| 항목 | 내용 |
|---|---|
| Signature | `off_t file_write_at (struct file *, const void *, off_t size, off_t start)` |
| 목적 | file의 특정 offset부터 쓰기 |
| 입력 | open file object, buffer, size, 시작 offset |
| 반환 | 실제 쓴 byte 수 |
| 상태 변화 | file current position은 바꾸지 않음 |
| Project 2 syscall 연결 | 기본 `write` syscall에는 보통 `file_write()`가 대응 |

### `file_deny_write`

| 항목 | 내용 |
|---|---|
| Signature | `void file_deny_write (struct file *)` |
| 목적 | 해당 file의 underlying inode에 대한 write 방지 |
| 입력 | open file object |
| 반환 | 없음 |
| Project 2 syscall 연결 | 실행 중인 executable write deny |

Project 2의 executable write deny 요구사항과 연결된다. 실행 중인 executable에
write가 들어가지 않도록 열어 둔 executable file에 적용하는 용도다.

### `file_allow_write`

| 항목 | 내용 |
|---|---|
| Signature | `void file_allow_write (struct file *)` |
| 목적 | `file_deny_write()`로 막은 write 허용 복구 |
| 입력 | open file object |
| 반환 | 없음 |
| Project 2 syscall 연결 | process 종료 시 executable cleanup |

`file_close()`는 내부적으로 write deny 상태를 해제하는 경로와 연결된다.

### `file_seek`

| 항목 | 내용 |
|---|---|
| Signature | `void file_seek (struct file *, off_t)` |
| 목적 | file current position 변경 |
| 입력 | open file object, 새 position |
| 반환 | 없음 |
| 상태 변화 | file position이 지정한 offset으로 바뀜 |
| Project 2 syscall 연결 | `seek(fd, position)` |

reference는 seek past EOF 자체는 error가 아니라고 설명한다. 다만 Project 2
기본 file system에서는 file growth가 없으므로, 이후 write의 결과는 file
system 제한을 따른다.

### `file_tell`

| 항목 | 내용 |
|---|---|
| Signature | `off_t file_tell (struct file *)` |
| 목적 | file current position 조회 |
| 입력 | open file object |
| 반환 | file 시작 기준 current position |
| Project 2 syscall 연결 | `tell(fd)` |

### `file_length`

| 항목 | 내용 |
|---|---|
| Signature | `off_t file_length (struct file *)` |
| 목적 | file size 조회 |
| 입력 | open file object |
| 반환 | file 크기, byte 단위 |
| Project 2 syscall 연결 | `filesize(fd)` |

## syscall별 API 연결 요약

| System call | fd 특수 처리 | 연결 API |
|---|---|---|
| `create(file, size)` | 없음 | `filesys_create()` |
| `remove(file)` | 없음 | `filesys_remove()` |
| `open(file)` | fd `0`, `1` 반환 금지 | `filesys_open()` 후 fd table 등록 |
| `close(fd)` | 기본 범위에서 fd `0`, `1`은 console 예약 | regular fd는 `file_close()` |
| `read(fd, buffer, size)` | fd `0`은 `input_getc()` | regular fd는 `file_read()` |
| `write(fd, buffer, size)` | fd `1`은 `putbuf()` | regular fd는 `file_write()` |
| `filesize(fd)` | regular fd 필요 | `file_length()` |
| `seek(fd, pos)` | regular fd 필요 | `file_seek()` |
| `tell(fd)` | regular fd 필요 | `file_tell()` |
| `fork()` | fd set inherit/duplicate | `file_duplicate()` 고려 |
| process exit | 열린 fd 모두 close | fd table 순회 후 `file_close()` |

## 구현 시 책임 경계

`filesys_*`와 `file_*` API가 해주지 않는 일:

- user pointer 검증
- fd 번호 배정
- process별 fd table 관리
- fd `0`, `1` console 특수 처리
- file system 동시 접근 동기화
- syscall return value 변환
- process exit 시 모든 fd close
- fork 시 fd table 복제 실패 처리

이 부분은 Project 2 system call layer와 process resource 관리 쪽에서
채워야 한다.
