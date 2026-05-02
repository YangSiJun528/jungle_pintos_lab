# System Calls infra와 user memory 요구사항

이 문서는 Pintos Project 2에서 `System Calls infra`와 `user memory`
접근/관리와 관련해 충족해야 하는 요구사항만 정리한다.

구현 방법, 자료구조 설계, 코드 조각, 구체적인 구현 순서는 다루지
않는다.

## 기준 자료

우선 기준은 CS330 64-bit KAIST Pintos 문서다.

- `docs/reference/pintos-kaist-original/2_project2/0_introduction.md`
- `docs/reference/pintos-kaist-original/2_project2/2_user_memory.md`
- `docs/reference/pintos-kaist-original/2_project2/3_system_call.md`
- `docs/reference/pintos-kaist-original/2_project2/4_process_termination.md`
- `docs/reference/pintos-kaist-original/2_project2/5_deny_write.md`
- `docs/reference/pintos-kaist-original/2_project2/6_dup.md`
- 보조 확인: `pintos/tests/userprog/Rubric.functionality`,
  `pintos/tests/userprog/Rubric.robustness`

## 전체 범위

Project 2의 목표는 user program이 OS와 상호작용할 수 있도록 system
call interface를 제공하는 것이다. base code는 user program을 load하고
실행할 수 있지만, I/O와 상호작용은 system call 구현 전까지 제대로
동작하지 않는다.

이번 범위에서 핵심은 다음이다.

- user process가 호출한 system call을 kernel이 받아 처리해야 한다.
- system call number와 arguments를 올바르게 해석해야 한다.
- return value가 있는 system call은 user program으로 결과를 돌려줘야
  한다.
- user program이 넘긴 pointer를 통해 user virtual address space의
  데이터를 안전하게 읽거나 써야 한다.
- 잘못된 user input이 kernel crash, panic, assertion failure, resource
  leak로 이어지면 안 된다.
- file system 관련 system call은 동시 실행되는 user process들 사이에서
  안전해야 한다.

## System call interface 요구사항

Pintos user program은 x86-64의 `syscall` instruction을 통해 system
call을 요청한다. 문서 기준으로 system call handler는 다음 계약을
만족해야 한다.

- user program이 요청한 system call number를 식별해야 한다.
- system call arguments를 정해진 x86-64 syscall ABI에 맞게 읽어야 한다.
- system call이 값을 반환해야 하는 경우, user program이 그 값을 받을 수
  있게 해야 한다.
- skeleton handler처럼 모든 system call을 단순히 종료시키는 상태에서
  벗어나, system call number에 맞는 동작을 수행해야 한다.
- user-level wrapper는 `lib/user/syscall.c`에 제공되어 있으며, user
  program은 이 interface를 통해 system call을 호출한다.

문서에 정의된 system call number는 `include/lib/syscall-nr.h`에 있고,
user program이 보는 prototype은 `include/lib/user/syscall.h` 기준이다.

## User memory 요구사항

system call 구현에서는 user virtual address space에 있는 데이터를
읽거나 써야 한다. 특히 system call argument로 전달된 pointer가
가리키는 데이터를 사용할 때 user memory access가 필요하다.

요구사항은 다음이다.

- kernel은 user가 제공한 pointer를 신뢰하면 안 된다.
- user pointer가 null pointer일 수 있음을 고려해야 한다.
- user pointer가 unmapped user virtual address를 가리킬 수 있음을
  고려해야 한다.
- user pointer가 kernel virtual memory를 가리킬 수 있음을 고려해야
  한다.
- user가 제공한 memory block의 일부만 invalid region에 걸쳐 있을 수도
  있음을 고려해야 한다.
- invalid pointer나 invalid memory block은 kernel이나 다른 process에
  해를 주지 않고 처리해야 한다.
- 이런 invalid user memory 접근은 offending user process를 terminate하는
  방식으로 처리해야 한다.
- user memory 문제를 처리하는 동안 이미 잡은 resource가 있다면 leak가
  남지 않아야 한다.

Project 2 introduction은 user virtual memory와 kernel virtual memory의
경계를 `KERN_BASE` 기준으로 설명한다. user program은 자신의 user virtual
memory에만 접근할 수 있고, kernel virtual memory 접근이나 unmapped user
address 접근은 안전하게 거부되어야 한다.

## 견고성 요구사항

Project 2 system call 이후 Pintos는 user program의 악의적이거나 잘못된
입력에도 견고해야 한다.

- user program이 어떤 system call argument를 넘기더라도 OS가 crash,
  panic, assertion failure, 기타 malfunction을 일으키면 안 된다.
- user program이 OS 전체를 종료시킬 수 있는 정상 경로는 `halt` system
  call뿐이어야 한다.
- invalid argument가 전달된 경우, 문서상 허용되는 처리는 system call별
  error value 반환, undefined value 반환, 또는 process termination이다.
- 다만 invalid user memory 접근은 user process termination 대상으로
  정리되어 있다.
- process가 kernel에 의해 종료된 경우, `wait` 관점에서는 exit status
  `-1`로 관찰되어야 한다.

보조 테스트 rubric은 bad pointer, page boundary에 걸친 buffer, invalid
file descriptor, invalid child pid, killed child 등을 robustness 범위로
확인한다.

## 동시성 및 file system 요구사항

Project 2에서는 여러 user process가 동시에 system call을 호출할 수
있다. file system code는 내부 동기화를 제공하지 않는 것으로 설명되어
있으므로, system call 계층에서 안전성을 보장해야 한다.

요구사항은 다음이다.

- 여러 user process가 동시에 system call을 수행할 수 있어야 한다.
- `filesys` directory의 file system code는 동시에 여러 thread가 호출하면
  안전하지 않다.
- file system code에 접근하는 system call은 file system code를 critical
  section으로 취급해야 한다.
- user program load와 관련된 file 접근도 같은 제약을 고려해야 한다.
- Project 2 범위에서는 file system 자체를 수정할 필요가 없으며, 문서는
  file system code 수정에 반대하는 방향을 권장한다.

## 구현 대상 system call

다음 system call들이 Project 2의 기본 구현 대상이다.

| System call | 요구 동작 |
|---|---|
| `halt` | Pintos를 종료한다. 일반적으로 자주 쓰면 안 되는 호출이다. |
| `exit` | 현재 user program을 종료하고 status를 kernel에 전달한다. |
| `fork` | 현재 process를 clone한 새 process를 만든다. child는 duplicated resource를 가져야 하고, parent는 child clone 성공 여부를 알기 전까지 반환하면 안 된다. |
| `exec` | 현재 process를 `cmd_line`이 가리키는 executable로 바꾼다. 성공하면 반환하지 않고, 실패하면 process가 exit status `-1`로 종료된다. file descriptor는 `exec` 이후에도 열린 상태로 남는다. |
| `wait` | direct child process의 종료를 기다리고 exit status를 회수한다. 조건에 맞지 않으면 즉시 `-1`을 반환해야 한다. |
| `create` | 주어진 이름과 초기 크기로 새 file을 만든다. 성공 여부를 반환한다. |
| `remove` | 주어진 이름의 file을 삭제한다. open된 file 삭제는 Unix-like semantics를 따른다. |
| `open` | 주어진 이름의 file을 열고 file descriptor를 반환한다. 실패하면 `-1`을 반환한다. |
| `filesize` | 열린 file descriptor가 가리키는 file의 byte 크기를 반환한다. |
| `read` | 열린 file 또는 stdin에서 지정한 byte 수만큼 읽어 buffer에 저장한다. 실제 읽은 byte 수, EOF의 `0`, 또는 실패 시 `-1`을 반환한다. |
| `write` | buffer의 데이터를 열린 file 또는 stdout에 쓴다. 실제 쓴 byte 수를 반환한다. |
| `seek` | 열린 file의 다음 read/write 위치를 지정한 byte offset으로 바꾼다. |
| `tell` | 열린 file의 다음 read/write 위치를 byte offset으로 반환한다. |
| `close` | file descriptor를 닫는다. process 종료 시 열린 file descriptor들은 암묵적으로 닫혀야 한다. |

`include/lib/syscall-nr.h`에는 다른 system call number도 정의되어 있지만,
Project 2 문서는 나머지를 지금은 무시하고 Project 3 또는 Project 4에서
구현하라고 설명한다.

## File descriptor 요구사항

file descriptor 관련 요구사항은 다음과 같다.

- file descriptor `0`은 stdin, `1`은 stdout으로 예약되어 있다.
- `open`은 `0` 또는 `1`을 반환하면 안 된다.
- 각 process는 독립적인 file descriptor set을 가진다.
- child process는 file descriptor를 inherit한다.
- 같은 file을 여러 번 열면 각각 별도의 file descriptor가 반환된다.
- 별도의 file descriptor들은 별도 `close` 대상이다.
- 같은 file을 여러 번 연 descriptor들은 file position을 공유하지 않는다.
- process 종료 또는 termination은 열린 file descriptor들을 닫는 것과
  같은 효과를 가져야 한다.
- 임의 제한은 피하는 것이 좋지만, 필요하면 process당 128개 open file
  제한은 허용된다고 FAQ가 설명한다.

## Process 관계 및 wait 요구사항

`wait`는 다음 요구사항을 만족해야 한다.

- direct child에 대해서만 성공할 수 있다.
- direct child란 성공한 `fork`의 return value로 받은 process를 뜻한다.
- child는 상속되지 않는다. grandchild는 direct child가 아니다.
- orphaned process는 다른 parent에게 재배정되지 않는다.
- 같은 child에 대해 `wait`는 한 번만 성공할 수 있다.
- child가 이미 종료된 뒤 parent가 `wait`해도 exit status를 회수할 수
  있어야 한다.
- child가 `exit`을 호출하지 않고 kernel에 의해 종료되면 parent의
  `wait`는 `-1`을 반환해야 한다.
- parent가 child를 기다리지 않고 종료하더라도 모든 process resource는
  결국 해제되어야 한다.
- initial process가 종료되기 전에는 Pintos가 종료되면 안 된다.

## Process termination message 요구사항

user process가 종료될 때는 process 이름과 exit code를 정해진 형식으로
출력해야 한다.

- `exit` 호출로 종료된 경우와 다른 이유로 종료된 경우 모두 대상이다.
- 출력되는 이름은 `fork()`에 전달된 full name 기준이다.
- kernel thread가 종료될 때는 출력하지 않는다.
- `halt` system call로 종료될 때는 출력하지 않는다.
- load 실패 시의 message는 optional이다.
- grading script를 혼란스럽게 하므로, Pintos 기본 출력 외의 추가 debug
  message는 남기면 안 된다.

## Executable write deny 요구사항

실행 중인 executable file에 대한 write는 금지되어야 한다.

- process가 실행 중인 executable file은 write가 deny되어야 한다.
- process가 실행 중인 동안 이 deny 상태가 유지되어야 한다.
- process 실행이 끝난 뒤에는 deny 상태가 해제될 수 있어야 한다.
- 이 요구사항은 Project 3의 virtual memory와도 관련되지만 Project 2에서도
  적용된다.

## Optional extra: `dup2`

Project 2 extra 요구사항은 기본 범위와 별도다.

- stdin/stdout close를 Linux와 비슷하게 허용하는 extra 요구가 있다.
- `dup2(oldfd, newfd)`는 `oldfd`를 `newfd` 번호로 복제해야 한다.
- `oldfd`가 invalid이면 실패하고 `newfd`는 닫히지 않아야 한다.
- `oldfd == newfd`이면 아무 동작 없이 `newfd`를 반환해야 한다.
- 성공 후 두 descriptor는 같은 open file description을 가리키며 file
  offset과 status flags를 공유해야 한다.
- `dup`된 descriptor semantics는 `fork` 뒤에도 보존되어야 한다.
- extra는 all-or-nothing 성격이라고 문서가 설명한다.

## 이번 문서에서 의도적으로 제외한 것

다음은 요구사항 이해에는 중요할 수 있지만, 이 문서의 목적상 제외한다.

- system call dispatch 구조 설계
- user pointer 검증 방식 선택
- file descriptor table 자료구조
- process/child 관리 자료구조
- lock 배치와 resource cleanup 구현 방식
- code snippet 또는 pseudo-code
- 특정 test를 통과하기 위한 구현 순서

