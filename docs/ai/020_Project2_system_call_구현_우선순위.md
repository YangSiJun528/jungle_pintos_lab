# Project2 system call 구현 우선순위

이 문서는 Pintos Project 2의 system call 구현을 어떤 순서로 진행하는 것이
합리적인지 정리한다. 기준은 로컬 reference 문서이며, 외부 자료와 테스트
파일은 근거로 사용하지 않았다.

구현 코드, pseudo-code, 자료구조 세부 설계는 다루지 않는다.

## 기준 자료

우선 기준은 CS330 64-bit KAIST Pintos 문서다.

- `docs/reference/pintos-kaist-original/2_project2/0_introduction.md`
- `docs/reference/pintos-kaist-original/2_project2/2_user_memory.md`
- `docs/reference/pintos-kaist-original/2_project2/3_system_call.md`
- `docs/reference/pintos-kaist-original/2_project2/4_process_termination.md`
- 보조 확인: `docs/reference/pintos-kaist-kr/2_project2/*`

## 우선순위 판단 기준

구현 순서는 다음 기준으로 잡는다.

- 모든 system call이 공유하는 기반을 먼저 만든다.
- 뒤 기능이 의존하는 process 종료 상태와 대기 구조를 먼저 잡는다.
- user pointer를 읽는 system call 전에 user memory 접근 정책을 정한다.
- fd `1` console 출력처럼 파일 시스템 전체 설계 없이 가능한 기능을 먼저
  붙인다.
- file descriptor table과 file system 동기화가 필요한 기능은 그 기반 뒤로
  미룬다.
- `fork`와 full `wait`는 process 관계, fd 복제, address space 복제가 모두
  걸리므로 뒤쪽에 둔다.

## 1. system call dispatch 기반

가장 먼저 `syscall_handler`가 system call number와 arguments를 해석하고,
return value를 user program에 돌려주는 틀을 만든다.

Reference 근거:

- `3_system_call.md`는 handler가 system call number와 arguments를 가져와
  적절한 action을 수행해야 한다고 설명한다.
- 같은 문서는 `%rax`가 system call number이고, arguments는 `%rdi`,
  `%rsi`, `%rdx`, `%r10`, `%r8`, `%r9` 순서라고 설명한다.
- return value가 있는 system call은 `struct intr_frame`의 `rax` member를
  수정해 반환한다고 설명한다.

이 단계의 목표:

- `SYS_EXIT`, `SYS_WRITE` 같은 system call number를 구분할 수 있는 구조
- argument register를 syscall별 의미로 해석할 수 있는 구조
- return value가 필요한 syscall에서 `rax`를 설정하는 구조

## 2. process 종료 상태와 공통 종료 경로

다음은 `exit(status)`와 kernel에 의한 process 종료가 같은 종료 경로를
공유할 수 있게 정리한다.

Reference 근거:

- `3_system_call.md`의 `exit` 설명은 현재 user program을 terminate하고
  `status`를 kernel에 넘긴다고 설명한다.
- 같은 설명은 parent가 `wait`하면 그 status가 반환된다고 말한다.
- `4_process_termination.md`는 user process가 종료될 때 process name과
  exit code를 정해진 형식으로 출력해야 한다고 설명한다.
- `3_system_call.md`의 `wait` 설명은 child가 `exit()`를 호출하지 않고
  kernel에 의해 종료되면 `wait(pid)`가 `-1`을 반환해야 한다고 설명한다.

이 단계의 목표:

- `exit(status)`가 현재 user process를 종료
- 종료 status를 나중에 parent `wait`가 회수할 수 있는 값으로 취급
- kernel kill과 명시적 `exit(status)`를 구분할 수 있는 방향 유지
- process termination message 형식 정리
- 추가 debug output을 남기지 않는 원칙 정리

## 3. initial process를 위한 process_wait 최소 경로

그다음 `process_wait()`의 최소 대기 경로를 잡는다. full `wait` syscall을
완성하기 전이라도, initial process가 끝나기 전에 Pintos가 종료되지 않게
하는 요구사항이 먼저 걸린다.

Reference 근거:

- `3_system_call.md`는 Pintos가 initial process exit 전 terminate되지
  않도록 보장해야 한다고 설명한다.
- 같은 문서는 제공 코드가 `threads/init.c`의 `main()`에서
  `userprog/process.c`의 `process_wait()`를 호출해 이를 시도한다고 설명한다.
- 또한 `wait`는 child가 살아 있으면 종료될 때까지 기다리고, 종료 뒤 exit
  status를 반환해야 한다고 설명한다.

이 단계의 목표:

- parent가 child 종료 전까지 기다릴 수 있는 최소 동기화 구조
- child 종료 status를 parent가 회수할 수 있는 최소 경로
- initial user program 실행이 끝나기 전에 Pintos가 먼저 종료되지 않게 함

주의할 점:

- 이 단계는 full `wait` syscall 완성이 아니다.
- direct child 검증, child당 1회 wait, orphan 처리, 여러 child 순서 대기는
  뒤 단계에서 완성해도 된다.
- 다만 나중에 full `wait`로 확장할 수 없는 임시 구조는 피해야 한다.

## 4. user memory 접근 정책

이제 pointer argument를 가진 syscall을 다루기 전에 user memory 접근 정책을
정한다. `write(1, buffer, size)`도 `buffer`를 kernel이 읽어야 하므로 이
단계가 선행 조건이다.

Reference 근거:

- `0_introduction.md`는 user가 null pointer, unmapped virtual memory,
  kernel virtual address space pointer를 넘길 수 있다고 설명한다.
- invalid pointer는 kernel이나 다른 process에 피해 없이 거부되어야 하며,
  offending process를 terminate하고 resource를 free해야 한다고 설명한다.
- `2_user_memory.md`는 pointer가 invalid하거나 kernel memory를 가리키거나,
  block 일부가 invalid region에 걸친 경우 user process termination으로
  처리해야 한다고 설명한다.
- `0_introduction.md`는 invalid pointer를 만났을 때 이미 잡은 lock이나
  할당한 memory를 leak하지 않아야 한다고 설명한다.

이 단계의 목표:

- user pointer와 user memory range를 다루는 일관된 정책
- invalid user pointer 처리 시 kernel crash, panic, resource leak 방지
- 이후 `write`, `read`, `open`, `exec`, `create`, `remove`의 string/buffer
  검증에 재사용 가능한 기준

## 5. write(1) console 출력

그다음 fd `1` 대상 `write`를 구현 대상으로 둔다. regular file write는 아직
뒤로 미룬다.

Reference 근거:

- `3_system_call.md`는 `write(fd, buffer, size)`가 `buffer`에서 `size`
  bytes를 `fd`에 쓰고, 실제 쓴 byte 수를 반환해야 한다고 설명한다.
- 같은 문서는 fd `1`이 console에 쓰는 descriptor라고 설명한다.
- console output은 size가 몇백 byte보다 크지 않으면 `putbuf()` 한 번으로
  전체 buffer를 써야 한다고 설명한다.
- `open` 설명은 fd `0`과 fd `1`이 console을 위해 예약되어 있고, `open`은
  이 둘을 반환하지 않는다고 설명한다.

이 단계의 목표:

- `write(1, buffer, size)`를 console 출력으로 처리
- 실제 출력 byte 수를 return value로 반환
- 작은 출력이 여러 process 사이에서 불필요하게 interleave되지 않게 함
- invalid `buffer`는 user memory 정책에 따라 처리

## 6. file descriptor table과 close-on-exit 기반

console 출력 다음에는 regular file syscall을 위해 process별 file descriptor
set을 만든다.

Reference 근거:

- `3_system_call.md`의 `open` 설명은 각 process가 독립적인 file descriptor
  set을 가진다고 설명한다.
- file descriptor `0`과 `1`은 console용으로 예약되어 있고, `open`은 둘을
  반환하지 않는다고 설명한다.
- child process는 file descriptor를 inherit한다고 설명한다.
- 같은 file을 여러 번 열면 각각 새 file descriptor가 반환되고, 서로 file
  position을 공유하지 않는다고 설명한다.
- `close` 설명은 process가 exit하거나 terminate되면 열린 file descriptor가
  각각 `close`된 것처럼 암묵적으로 닫혀야 한다고 설명한다.

이 단계의 목표:

- fd `0`, fd `1`, regular file fd의 의미 분리
- process별 fd table 준비
- `close`와 process 종료 시 fd 정리 기준 마련
- 나중에 `fork`에서 fd inheritance를 구현할 수 있는 형태 유지

## 7. file system 동기화 기반

regular file syscall을 붙이기 전에 file system critical section 기준을
정한다.

Reference 근거:

- `3_system_call.md`는 여러 user process가 동시에 system call을 만들 수
  있어야 한다고 설명한다.
- file system code는 여러 thread가 동시에 호출하기에 safe하지 않다고
  설명한다.
- system call implementation은 file system code를 critical section으로
  취급해야 한다고 설명한다.
- `process_exec()`도 file에 접근한다는 점을 잊지 말라고 설명한다.

이 단계의 목표:

- file system 접근 syscall의 공통 동기화 기준
- `process_exec()`와 file syscall 사이의 동기화 충돌 방지
- file system code 자체 수정 없이 syscall layer에서 보호

## 8. 기본 file syscall 묶음

fd table과 file system 동기화가 준비된 뒤 regular file syscall을 붙인다.

Reference 근거:

- `create`는 새 file을 만들고 성공 여부를 반환해야 한다.
- `remove`는 file 삭제 성공 여부를 반환해야 하며, open file 삭제는 file을
  닫지 않는다.
- `open`은 file descriptor 또는 `-1`을 반환해야 한다.
- `filesize`는 열린 fd의 file size를 반환해야 한다.
- `read`는 file 또는 fd `0`에서 읽고, 실제 읽은 byte 수, EOF의 `0`, 실패
  시 `-1`을 반환해야 한다.
- file 대상 `write`는 가능한 만큼 쓰고 실제 쓴 byte 수 또는 `0`을 반환해야
  한다.
- `seek`, `tell`, `close`는 열린 fd의 file position과 close 동작을 다룬다.

권장 구현 묶음:

1. `create`, `remove`
2. `open`, `close`
3. `filesize`
4. file 대상 `read`, fd `0` 대상 `read`
5. file 대상 `write`
6. `seek`, `tell`

이 단계의 목표:

- Project 2 기본 file syscall 계약 충족
- invalid fd와 invalid pointer에 대한 일관된 처리
- process exit 시 열린 fd 정리

## 9. exec

file syscall 기반과 user memory string 검증이 준비된 뒤 `exec`를 다룬다.

Reference 근거:

- `3_system_call.md`는 `exec(cmd_line)`이 현재 process를 주어진 executable로
  바꾸고, 성공하면 반환하지 않는다고 설명한다.
- program을 load하거나 run할 수 없으면 process가 exit state `-1`로
  terminate된다고 설명한다.
- `exec`는 호출 thread의 이름을 바꾸지 않는다고 설명한다.
- file descriptors는 `exec` 이후에도 open 상태로 남는다고 설명한다.
- file system 동기화 설명은 `process_exec()`도 file에 접근한다고 지적한다.

이 단계의 목표:

- user string `cmd_line` 검증
- load 성공 시 현재 process image 교체
- load 실패 시 exit status `-1` 종료
- fd table 유지
- file system 동기화 기준 적용

## 10. fork

`fork`는 뒤쪽에 둔다. process 상태, fd table, address space, parent-child
동기화가 모두 걸리는 기능이기 때문이다.

Reference 근거:

- `3_system_call.md`는 `fork(thread_name)`이 현재 process의 clone을 새로
  만들어야 한다고 설명한다.
- child pid를 반환해야 하고, child에서는 return value가 `0`이어야 한다.
- child는 file descriptor와 virtual memory space를 포함한 resource를
  duplicated 상태로 가져야 한다.
- parent는 child가 성공적으로 clone되었는지 알기 전에는 `fork`에서
  return하면 안 된다.
- child resource duplicate 실패 시 parent의 `fork`도 `TID_ERROR`를
  반환해야 한다.
- 같은 문서는 template이 `pml4_for_each()`를 이용해 전체 user memory space와
  page table structure를 copy한다고 설명한다.

이 단계의 목표:

- parent-child 관계 등록
- child clone 성공/실패를 parent가 알 수 있는 동기화
- child return value `0`
- parent return value child pid 또는 failure
- fd 복제
- user virtual memory 복제

## 11. wait syscall full semantics

마지막으로 `wait(pid)`의 전체 semantics를 완성한다. 최소 `process_wait`
경로는 앞에서 잡았지만, full `wait`는 `fork` 이후의 parent-child 관계와
child status lifetime이 필요하다.

Reference 근거:

- `wait`는 child process를 기다리고 child exit status를 회수해야 한다.
- child가 살아 있으면 terminate될 때까지 기다려야 한다.
- child가 이미 종료된 뒤 parent가 wait해도 status를 회수할 수 있어야 한다.
- kernel에 의해 종료된 child는 `-1`로 관찰되어야 한다.
- direct child가 아니면 즉시 `-1`을 반환해야 한다.
- 같은 child에 대해 이미 wait했다면 즉시 `-1`을 반환해야 한다.
- process는 여러 child를 만들고, 어떤 순서로든 wait할 수 있고, 일부 또는
  전부를 wait하지 않고 exit할 수도 있다.
- parent가 wait했는지와 무관하게, child가 parent보다 먼저 죽든 나중에 죽든
  모든 resource는 free되어야 한다.

이 단계의 목표:

- direct child 검증
- child당 wait 1회 제한
- 이미 종료된 child status 보존과 회수
- parent 선종료, child 선종료 모두에서 resource cleanup
- orphan을 새 parent에게 재배정하지 않음

## 12. robustness 정리

마지막으로 모든 syscall 경로를 대상으로 invalid argument와 resource cleanup을
점검한다.

Reference 근거:

- `3_system_call.md`는 user program의 어떤 동작도 OS crash, panic,
  assertion failure, malfunction을 일으켜서는 안 된다고 설명한다.
- user program이 OS 전체를 halt할 수 있는 유일한 정상 경로는 `halt`
  syscall이어야 한다고 설명한다.
- invalid argument 처리 방식으로 error value 반환, undefined value 반환,
  process termination이 허용된다고 설명한다.
- user memory reference는 invalid pointer를 offending process termination과
  resource cleanup으로 처리해야 한다고 설명한다.

이 단계의 목표:

- 모든 syscall의 invalid pointer 처리 점검
- invalid fd, invalid pid, load failure, duplicate wait 등 오류 경로 점검
- lock, allocated memory, open file leak 점검
- 추가 debug output 제거

## 전체 순서 요약

1. system call dispatch 기반
2. process 종료 상태와 공통 종료 경로
3. initial process를 위한 `process_wait` 최소 경로
4. user memory 접근 정책
5. `write(1)` console 출력
6. file descriptor table과 close-on-exit 기반
7. file system 동기화 기반
8. 기본 file syscall 묶음
9. `exec`
10. `fork`
11. `wait` syscall full semantics
12. robustness 정리

핵심은 `exit`와 `wait`의 status 전달 계약을 먼저 잡고, 그 위에 observable
output인 `write(1)`을 얹은 뒤, fd와 file system, process 복제로 확장하는
순서다.
