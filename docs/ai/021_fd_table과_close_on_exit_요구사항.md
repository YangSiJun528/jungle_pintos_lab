# fd table과 close-on-exit 요구사항

이 문서는 Pintos Project 2에서 regular file system call을 구현하기 전에
준비해야 하는 file descriptor table과 close-on-exit 기반 요구사항을
정리한다.

기준은 로컬 reference 문서와 현재 저장소의 공개 header/API다. 구현 코드,
자료구조 확정안, pseudo-code는 다루지 않는다.

## 기준 자료

우선 기준은 CS330 64-bit KAIST Pintos 문서다.

- `docs/reference/pintos-kaist-original/2_project2/0_introduction.md`
- `docs/reference/pintos-kaist-original/2_project2/3_system_call.md`
- `docs/reference/pintos-kaist-original/2_project2/6_dup.md`
- `docs/reference/pintos-kaist-original/2_project2/7_FAQ.md`
- 보조 확인: `docs/reference/pintos-kaist-kr/2_project2/*`

현재 저장소 API 확인:

- `pintos/include/filesys/filesys.h`
- `pintos/include/filesys/file.h`
- `pintos/include/lib/stdio.h`
- `pintos/include/lib/kernel/stdio.h`

## fd table이 필요한 이유

Project 2 reference는 file descriptor를 단순한 전역 번호가 아니라 process별
open file handle로 다룬다.

Reference 근거:

- `open`은 file을 열고 nonnegative integer handle인 fd를 반환한다.
- 각 process는 독립적인 file descriptor set을 가진다.
- 같은 file을 여러 번 열면 각 `open`은 새 fd를 반환한다.
- 서로 다른 fd는 별도의 `close` 대상이며, file position을 공유하지 않는다.

따라서 fd table은 적어도 다음 질문에 답할 수 있어야 한다.

- 현재 process에서 fd 번호가 valid한가?
- 이 fd가 console 예약 fd인가, regular file fd인가?
- regular file fd라면 어떤 open file object를 가리키는가?
- fd를 닫았을 때 해당 entry와 file resource를 어떻게 정리하는가?
- process가 종료될 때 남은 fd들을 어떻게 모두 닫는가?
- 나중에 `fork`할 때 fd set을 child에게 어떻게 inherit/duplicate할 것인가?

## fd 번호 요구사항

Reference 근거:

> "File descriptors numbered 0 and 1 are reserved for the console: fd 0
> (`STDIN_FILENO`) is standard input, fd 1 (`STDOUT_FILENO`) is standard
> output."
>
> - `docs/reference/pintos-kaist-original/2_project2/3_system_call.md`

> "The `open` system call will never return either of these file descriptors"
>
> - `docs/reference/pintos-kaist-original/2_project2/3_system_call.md`

정리:

- fd `0`은 stdin이다.
- fd `1`은 stdout이다.
- fd `0`, `1`은 console용으로 예약되어 있다.
- regular file을 여는 `open()`은 fd `0` 또는 fd `1`을 반환하면 안 된다.
- 기본 Project 2 범위에서는 regular file fd를 fd `2` 이상에서 배정하는
  구조가 자연스럽다.

현재 저장소 확인:

- `pintos/include/lib/stdio.h`는 `STDIN_FILENO`를 `0`, `STDOUT_FILENO`를
  `1`로 정의한다.

## process별 fd set 요구사항

Reference 근거:

> "Each process has an independent set of file descriptors."
>
> - `docs/reference/pintos-kaist-original/2_project2/3_system_call.md`

정리:

- fd table은 process별 상태여야 한다.
- 같은 fd 번호라도 process가 다르면 다른 open file을 의미할 수 있다.
- 전역 fd table 하나로 모든 process fd를 관리하는 방식은 reference의
  "independent set" 요구와 맞지 않는다.
- process 생성/종료 lifecycle과 함께 fd table init/cleanup 시점이 있어야
  한다.

구현상 필요한 상태:

- 현재 process가 소유한 fd entry들의 collection
- 다음에 배정할 fd 번호 또는 비어 있는 fd 번호 탐색 기준
- fd 번호로 entry를 찾는 lookup 경로
- entry가 비어 있는지, 열려 있는지 구분하는 상태

## open 동작 요구사항

Reference 근거:

> "Opens the file called `file`. Returns a nonnegative integer handle called a
> 'file descriptor' (fd), or `-1` if the file could not be opened."
>
> - `docs/reference/pintos-kaist-original/2_project2/3_system_call.md`

정리:

- `open(file)`은 file을 열고 fd를 반환해야 한다.
- 실패하면 `-1`을 반환해야 한다.
- 성공한 fd는 현재 process의 fd set에 등록되어야 한다.
- `open`은 fd `0` 또는 fd `1`을 반환하지 않아야 한다.

현재 저장소에서 연결되는 API:

- `filesys_open(const char *name)`은 file 이름으로 `struct file *`을 반환한다.
- fd table entry는 regular file fd와 `struct file *` 사이의 mapping을
  저장할 수 있어야 한다.

## 같은 file을 여러 번 열 때의 요구사항

Reference 근거:

> "When a single file is opened more than once, whether by a single process or
> different processes, each open returns a new file descriptor."
>
> "Different file descriptors for a single file are closed independently in
> separate calls to close and they do not share a file position."
>
> - `docs/reference/pintos-kaist-original/2_project2/3_system_call.md`

정리:

- 같은 process가 같은 file을 여러 번 열어도 fd는 매번 새로 배정되어야 한다.
- 다른 process가 같은 file을 열어도 각 process는 자기 fd를 따로 가진다.
- 서로 다른 fd는 `close`도 독립적으로 처리되어야 한다.
- 서로 다른 fd는 file position을 공유하면 안 된다.

현재 저장소에서 연결되는 API/상태:

- `struct file`은 열린 file object의 current position을 가진다.
- `file_read()`와 `file_write()`는 file의 current position을 사용하고
  갱신한다.
- 따라서 fd entry는 단순 file name이 아니라 open file object 단위 상태를
  가리켜야 한다.

## read/write/filesize/seek/tell에서 fd table이 제공해야 하는 것

Reference 근거:

- `filesize(fd)`는 fd로 열린 file size를 반환한다.
- `read(fd, buffer, size)`는 fd에서 읽고 실제 읽은 byte 수, EOF의 `0`, 실패
  시 `-1`을 반환한다.
- fd `0`은 keyboard에서 `input_getc()`로 읽는다.
- `write(fd, buffer, size)`는 fd에 쓰고 실제 쓴 byte 수를 반환한다.
- fd `1`은 console에 쓴다.
- `seek(fd, position)`은 fd의 다음 read/write 위치를 바꾼다.
- `tell(fd)`은 fd의 다음 read/write 위치를 반환한다.

정리:

- fd table lookup은 syscall마다 fd가 valid한지 판단할 수 있어야 한다.
- fd `0`과 fd `1`은 regular file lookup과 별도로 처리해야 한다.
- regular file fd라면 `struct file *`을 찾아 file API에 넘길 수 있어야 한다.
- 닫힌 fd나 존재하지 않는 fd는 invalid fd로 취급되어야 한다.

현재 저장소에서 연결되는 API:

- console output: `putbuf(const char *, size_t)`
- stdin input: `input_getc(void)`
- regular file read/write: `file_read()`, `file_write()`
- file size/position: `file_length()`, `file_seek()`, `file_tell()`

## close 요구사항

Reference 근거:

> "Closes file descriptor `fd`."
>
> - `docs/reference/pintos-kaist-original/2_project2/3_system_call.md`

정리:

- `close(fd)`는 현재 process의 fd table에서 해당 fd를 닫아야 한다.
- regular file fd를 닫으면 연결된 open file object도 정리되어야 한다.
- 닫힌 fd는 더 이상 valid fd로 동작하면 안 된다.
- 서로 다른 fd는 독립적으로 닫혀야 한다.

현재 저장소에서 연결되는 API:

- regular file object 정리는 `file_close(struct file *)`가 담당한다.

## close-on-exit 요구사항

Reference 근거:

> "Exiting or terminating a process implicitly closes all its open file
> descriptors, as if by calling this function for each one."
>
> - `docs/reference/pintos-kaist-original/2_project2/3_system_call.md`

정리:

- process가 `exit`를 호출해 종료될 때 열린 fd를 모두 닫아야 한다.
- process가 exception 등으로 terminate될 때도 열린 fd를 모두 닫아야 한다.
- close-on-exit은 `close(fd)`를 각 fd에 호출한 것과 같은 효과를 가져야 한다.
- fd table cleanup은 process resource cleanup 경로와 연결되어야 한다.

구현상 필요한 상태:

- process 종료 시 fd table 전체를 순회할 수 있어야 한다.
- 열린 regular file fd마다 `file_close()`에 해당하는 정리가 가능해야 한다.
- 이미 닫힌 fd를 중복 close하지 않도록 entry 상태를 관리해야 한다.

## fork inheritance 요구사항

Reference 근거:

> "File descriptors are inherited by child processes."
>
> - `docs/reference/pintos-kaist-original/2_project2/3_system_call.md`

> "The child should have DUPLICATED resources including file descriptor and
> virtual memory space."
>
> - `docs/reference/pintos-kaist-original/2_project2/3_system_call.md`

정리:

- 나중에 `fork`를 구현하면 child process는 parent의 fd set을 inherit해야 한다.
- child는 fd 번호도 parent와 대응되게 가져야 한다.
- child resource duplicate가 실패하면 parent의 `fork`도 실패해야 한다.
- parent는 child가 fd를 포함한 resource duplicate에 성공했는지 알기 전
  `fork`에서 return하면 안 된다.

현재 저장소에서 연결되는 API/힌트:

- `pintos/include/filesys/file.h`에는 `file_duplicate(struct file *)`가 있다.
- `pintos/userprog/process.c`의 fork 관련 TODO는 file object 복제에
  `file_duplicate()`를 사용하라고 힌트를 준다.

주의:

- 기본 Project 2 reference의 "다른 fd는 file position을 공유하지 않는다"는
  open 설명과, fork 시 "resource duplicated" 요구를 함께 보면, fork의 fd
  inheritance는 parent의 open file object를 child 쪽에 복제하는 방향으로
  해석하는 것이 자연스럽다.
- 단, 정확한 자료구조와 reference count 방식은 reference가 지정하지 않는다.

## file system 동기화 요구사항

Reference 근거:

> "It is not safe to call into the file system code provided in the `filesys`
> directory from multiple threads at once."
>
> "Your system call implementation must treat the file system code as a
> critical section."
>
> - `docs/reference/pintos-kaist-original/2_project2/3_system_call.md`

정리:

- fd table syscall 중 file system code를 호출하는 경로는 동기화가 필요하다.
- `open`, `close`, `read`, file 대상 `write`, `filesize`, `seek`, `tell`,
  `remove`, `create` 등이 file system/file API와 연결된다.
- `process_exec()`도 file에 접근하므로 같은 동기화 기준을 고려해야 한다.
- Project 2 reference는 file system code 자체를 수정하지 않는 것을 권장한다.

## removed open file 요구사항

Reference 근거:

> "when a file is removed any process which has a file descriptor for that file
> may continue to use that descriptor."
>
> - `docs/reference/pintos-kaist-original/2_project2/7_FAQ.md`

정리:

- open된 file이 remove되어도 이미 그 file을 가리키는 fd는 계속 사용할 수
  있어야 한다.
- 해당 fd로 read/write가 가능해야 한다.
- file name은 사라져 새로 open할 수 없지만, 마지막 fd가 닫힐 때까지 file
  내용은 유지되어야 한다.
- 이 동작은 `filesys_remove()`의 Unix-like semantics와 연결된다.

구현상 의미:

- fd table entry는 file name lookup을 매번 다시 하는 방식이면 안 된다.
- `open` 시점에 얻은 open file object를 fd entry가 보유해야 한다.

## fd 개수 제한

Reference 근거:

> "It is better not to set an arbitrary limit. You may impose a limit of 128
> open files per process, if necessary."
>
> - `docs/reference/pintos-kaist-original/2_project2/7_FAQ.md`

정리:

- 임의 제한을 두지 않는 것이 더 좋다.
- 필요하면 process당 open file 128개 제한은 허용된다.
- extra requirements를 구현하려면 제한이 없어야 한다고 FAQ가 설명한다.

## extra: stdin/stdout close와 dup2는 기본 범위와 분리

Reference 근거:

- `6_dup.md`는 stdin/stdout close 허용과 `dup2`를 extra 요구사항으로 설명한다.
- extra에서는 stdin을 close하면 입력을 읽지 않아야 하고, stdout을 close하면
  아무것도 출력하지 않아야 한다고 설명한다.
- `dup2(oldfd, newfd)`는 `oldfd`를 `newfd` 번호로 복제하고 성공 시 `newfd`를
  반환해야 한다.
- `newfd`가 이미 열려 있으면 조용히 닫은 뒤 재사용한다.
- `oldfd`가 invalid이면 `-1`을 반환하고 `newfd`는 닫지 않는다.
- `oldfd == newfd`이면 아무것도 하지 않고 `newfd`를 반환한다.
- `dup2`로 만들어진 서로 다른 fd는 같은 open file description을 가리켜
  file offset과 status flags를 공유해야 한다.
- dup된 fd semantics는 fork 뒤에도 보존되어야 한다.

정리:

- 기본 Project 2 fd table은 fd `0`, `1`을 console 예약 번호로 다루면 된다.
- stdin/stdout close 가능 여부와 `dup2` 공유 offset semantics는 extra 범위다.
- extra까지 고려한다면 fd table entry가 단순히 `fd -> struct file *`인
  구조보다 open file description 공유를 표현할 수 있어야 한다.

## 구현 전 체크리스트

구현 전에 다음 질문에 답할 수 있어야 한다.

- fd table은 `struct thread` 또는 process 관련 상태 중 어디에서
  lifecycle을 가질 것인가?
- fd `0`, `1`은 fd table에 실제 entry로 둘 것인가, syscall에서 특별 취급할
  것인가?
- regular file fd는 어떤 번호부터 배정할 것인가?
- 닫힌 fd 번호를 재사용할 것인가, 계속 증가시킬 것인가?
- fd lookup 실패를 syscall별로 어떤 return value나 termination으로 처리할
  것인가?
- `close(fd)`와 process exit cleanup이 같은 정리 경로를 공유할 수 있는가?
- fork 시 fd table 복제 실패를 parent에게 전달할 수 있는가?
- file system lock을 fd table lock과 어떻게 분리하거나 순서화할 것인가?
- invalid user pointer가 file system lock 획득 후 발견될 때 leak 없이
  정리할 수 있는가?

## 이번 문서에서 의도적으로 제외한 것

- 구체적인 fd table 자료구조 선택
- 코드 patch 또는 pseudo-code
- 특정 테스트 실행 명령
- `fork` 전체 구현 절차
- `dup2` extra 구현 절차
- Project 4 directory fd 요구사항
