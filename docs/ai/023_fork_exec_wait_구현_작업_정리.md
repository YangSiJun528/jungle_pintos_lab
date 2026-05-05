# fork/exec/wait 구현 작업 정리

이 문서는 현재 저장소 상태에서 Pintos Project 2의 `fork`, `exec`, `wait`를
구현하려면 어떤 작업을 해야 하는지 정리한다.

기준은 로컬 reference 문서와 현재 저장소 코드다. 외부 Pintos 구현이나 해답은
근거로 사용하지 않았다. 구현 코드, pseudo-code, 붙여 넣을 수 있는 세부
구현안은 다루지 않는다.

## 기준 자료

우선 기준은 CS330 64-bit KAIST Pintos 문서다.

- `docs/reference/pintos-kaist-original/2_project2/1_argument_passing.md`
- `docs/reference/pintos-kaist-original/2_project2/2_user_memory.md`
- `docs/reference/pintos-kaist-original/2_project2/3_system_call.md`
- `docs/reference/pintos-kaist-original/2_project2/4_process_termination.md`
- `docs/reference/pintos-kaist-original/2_project2/5_deny_write.md`
- 보조 확인: `docs/reference/pintos-kaist-kr/2_project2/*`

현재 코드 확인 지점:

- `pintos/userprog/syscall.c`
- `pintos/userprog/process.c`
- `pintos/userprog/exception.c`
- `pintos/include/threads/thread.h`
- `pintos/threads/thread.c`
- `pintos/include/filesys/file.h`
- `pintos/include/threads/mmu.h`

## Reference 요구사항 요약

### fork

`fork(const char *thread_name)`은 현재 process의 clone을 만든다.

Reference상 핵심 요구사항:

- child process의 pid를 parent에게 반환해야 한다.
- 실패하면 valid pid가 아닌 값을 반환해야 한다.
- child process에서는 `fork()`의 return value가 `0`이어야 한다.
- file descriptor와 virtual memory space를 포함한 resource를 duplicate해야
  한다.
- parent는 child가 resource duplicate에 성공했는지 알기 전까지 `fork()`에서
  반환하면 안 된다.
- child가 resource duplicate에 실패하면 parent의 `fork()` call은
  `TID_ERROR`를 반환해야 한다.
- callee-saved register인 `%RBX`, `%RSP`, `%RBP`, `%R12`-`%R15`는 보존
  대상이다. KAIST skeleton은 syscall 시점의 `intr_frame`을 child에게
  전달하라는 힌트를 준다.

### exec

`exec(const char *cmd_line)`은 현재 process를 새 executable로 바꾼다.

Reference상 핵심 요구사항:

- `cmd_line`의 executable 이름과 arguments를 사용해 새 program을 실행한다.
- 성공하면 user program으로 전환되므로 syscall 호출자에게 돌아오지 않는다.
- load 또는 run에 실패하면 process는 exit status `-1`로 terminate된다.
- `exec`은 호출한 thread의 이름을 바꾸지 않는다.
- file descriptor는 `exec` 이후에도 open 상태로 유지되어야 한다.

### wait

`wait(pid_t pid)`는 direct child의 종료를 기다리고 exit status를 회수한다.

Reference상 핵심 요구사항:

- child가 살아 있으면 종료될 때까지 기다린다.
- child가 `exit(status)`로 종료했다면 그 status를 반환한다.
- child가 exception 등으로 kernel에 의해 종료되었다면 `-1`을 반환한다.
- parent가 wait를 호출하기 전에 child가 이미 종료되어도 status 회수가
  가능해야 한다.
- `pid`가 direct child가 아니면 즉시 `-1`을 반환해야 한다.
- 같은 child에 대해 두 번 wait하면 두 번째부터 즉시 `-1`을 반환해야 한다.
- child는 상속되지 않는다. parent가 죽어도 orphan child가 새 parent에게
  재할당되지 않는다.
- parent가 일부 child를 wait하지 않고 종료해도 모든 resource는 반드시
  정리되어야 한다.
- initial process가 종료되기 전 Pintos가 먼저 terminate되지 않아야 한다.

## 현재 코드 상태

### syscall layer

`pintos/userprog/syscall.c`의 상태:

- `handle_fork()`는 `ASSERT(false)` 상태다.
- `handle_exec()`는 `ASSERT(false)` 상태다.
- `handle_wait()`는 `ASSERT(false)` 상태다.
- `syscall_handler()`는 `struct intr_frame *f`를 받지만, 현재
  `struct syscall_entry`에는 원본 `intr_frame` pointer가 저장되지 않는다.

따라서 `fork` 구현에 필요한 syscall 시점의 register context가 handler
내부에서 사라지는 구조다. `fork`를 연결하려면 syscall entry가 원본
`intr_frame`에 접근할 수 있어야 한다.

### process layer

`pintos/userprog/process.c`의 상태:

- `process_fork()`는 `thread_create()`만 호출한다.
- `__do_fork()`에는 `parent_if` 전달 TODO가 남아 있다.
- `duplicate_pte()`에는 user page 복제 관련 TODO가 남아 있다.
- fd table 복제 작업이 아직 없다.
- `process_wait()`는 yield loop 후 `-1`만 반환한다.
- `process_exit()`는 종료 메시지와 fd cleanup만 수행한다.
- `process_cleanup()`이 fd table까지 닫으므로, 현재 그대로는 `exec`에서 fd가
  유지되지 않는다.
- `load()`는 실행 파일을 열고 마지막에 닫는다. executable deny-write를
  유지하려면 실행 중인 동안 executable file object를 보관해야 한다.

### thread/process 상태

`pintos/include/threads/thread.h`의 현재 userprog 관련 상태:

- `pml4`
- `exit_status`
- `file_descriptors`

아직 없는 상태:

- parent-child 관계를 추적하는 목록 또는 링크
- child별 종료 status를 보관하는 별도 metadata
- child load/fork 성공 여부를 parent에게 알리는 동기화 수단
- child 종료를 기다리는 동기화 수단
- executable deny-write를 유지하기 위한 실행 파일 handle

`struct thread`는 thread 종료 뒤 scheduler 경로에서 해제될 수 있다. 따라서
parent가 child 종료 후에도 status를 회수해야 한다면, wait용 metadata는
`struct thread *` 자체와 분리해서 설계하는 편이 안전하다.

## 구현 작업 목록

## 1. syscall entry에 intr_frame 접근 경로 추가

목표:

- `handle_fork()`가 syscall 진입 시점의 `struct intr_frame`을
  `process_fork()`에 넘길 수 있게 한다.

근거:

- reference는 system call arguments와 return value가 register와
  `struct intr_frame`을 통해 오간다고 설명한다.
- skeleton 주석은 `__do_fork()`가 `process_fork()`의 두 번째 인자인
  `intr_frame`을 받아야 한다고 말한다.

현재 코드 기준 작업:

- `syscall_handler()`에서 받은 `struct intr_frame *f`를 syscall dispatch
  중에도 참조할 수 있게 구조를 조정한다.
- `handle_fork()`에서 user string 검증 후 `process_fork(thread_name, f)`에
  연결한다.
- `handle_exec()`와 `handle_wait()`도 return value 설정 규칙에 맞춰
  syscall entry를 채운다.

주의:

- user pointer 검증 실패 시 현재 process를 종료하는 정책과 일관되어야 한다.
- `fork`의 `thread_name`이 page boundary에 걸친 경우도 user string 검증에서
  다룰 수 있어야 한다.

## 2. wait/exit용 child metadata 설계

목표:

- parent가 direct child를 찾고, 한 번만 wait하며, child 종료 status를
  회수할 수 있게 한다.

근거:

- reference는 direct child가 아니면 wait가 실패해야 한다고 설명한다.
- 같은 child에 대해 wait는 최대 한 번만 가능하다.
- child가 이미 죽은 뒤 parent가 wait해도 status를 회수할 수 있어야 한다.
- parent가 wait하지 않아도 모든 resource는 정리되어야 한다.

현재 코드 기준 작업:

- `struct thread`에 child 목록을 관리할 상태를 추가한다.
- child마다 pid, exit status, 종료 여부, wait 여부, parent 생존 여부,
  동기화 primitive를 담는 별도 metadata를 둔다.
- `process_fork()` 성공 시 parent의 child 목록에 metadata를 등록한다.
- `process_exit()`에서 자신의 종료 status를 metadata에 기록하고 대기 중인
  parent를 깨운다.
- parent가 wait하면 metadata를 찾아 기다리고, status 회수 후 중복 wait가
  실패하도록 표시한다.
- parent가 exit할 때 wait하지 않은 child metadata와 parent-child 관계를
  정리한다.

주의:

- child의 `struct thread`는 child 종료 후 계속 참조하면 안 된다.
- parent가 먼저 죽는 경우와 child가 먼저 죽는 경우를 모두 처리해야 한다.
- `process_wait(initd)` 경로 때문에 initial process도 이 구조 위에서
  정상적으로 기다릴 수 있어야 한다.

## 3. process_exit와 exception 종료 상태 정리

목표:

- 명시적 `exit(status)`와 kernel kill이 wait 결과에 올바르게 반영되게 한다.

근거:

- `exit(status)`의 status는 parent `wait`의 반환값이 된다.
- child가 `exit()`을 호출하지 않고 exception 등으로 종료되면 `wait()`는
  `-1`을 반환해야 한다.
- user process 종료 시 `printf("%s: exit(%d)\n", ...)` 형식의 메시지를
  출력해야 한다.

현재 코드 기준 작업:

- syscall `exit(status)`는 `thread_current()->exit_status`를 설정한 뒤
  종료한다.
- page fault나 user exception으로 종료되는 경로는 exit status를 `-1`로
  남기도록 정리한다.
- `process_exit()`에서 종료 status를 child metadata에 기록한다.
- user process가 아닌 kernel thread나 halt syscall에는 종료 메시지를
  출력하지 않는 조건을 검토한다.

주의:

- `process_exit()`의 추가 debug 출력은 grading script를 깨뜨릴 수 있으므로
  reference가 요구한 종료 메시지 외 출력은 피해야 한다.

## 4. exec용 cleanup과 exit용 cleanup 분리

목표:

- `exec`가 기존 주소 공간은 버리되 file descriptor는 유지하게 한다.

근거:

- reference는 file descriptor가 `exec` call을 지나도 open 상태로 유지된다고
  명시한다.

현재 코드 기준 문제:

- `process_exec()`는 현재 `process_cleanup()`을 호출한다.
- `process_cleanup()`은 `file_descriptors`를 전부 닫는다.
- 따라서 현재 구조 그대로 `exec`를 연결하면 fd 유지 요구사항과 충돌한다.

현재 코드 기준 작업:

- 주소 공간 정리와 fd 정리를 분리한다.
- `exec`는 기존 pml4 또는 supplemental page table만 정리한다.
- process exit은 주소 공간, fd table, executable handle, child metadata를
  모두 정리한다.
- `exec(cmd_line)`의 user string은 기존 주소 공간을 정리하기 전에 커널
  메모리로 복사해야 한다.

주의:

- `exec` 실패 시 현재 process는 exit status `-1`로 종료되어야 한다.
- 성공 시 호출자에게 반환하지 않고 새 user program으로 진입한다.

## 5. exec syscall 연결과 load 실패 처리

목표:

- user `exec(cmd_line)` syscall을 `process_exec()`에 안전하게 연결한다.

근거:

- reference는 `exec` 성공 시 반환하지 않고, 실패 시 exit status `-1`로
  terminate된다고 설명한다.
- user memory reference는 invalid user pointer를 process 종료로 처리해야
  한다고 설명한다.

현재 코드 기준 작업:

- `handle_exec()`에서 `cmd_line` user string을 검증한다.
- 검증된 문자열을 커널 페이지에 복사한다.
- 복사한 커널 문자열을 `process_exec()`에 넘긴다.
- `process_exec()`가 실패를 반환하면 현재 process를 `-1`로 종료한다.

주의:

- `exec-missing` 테스트 계열은 실패 처리를 확인한다.
- `exec-read` 계열은 fd가 exec 이후 유지되는지 확인한다.
- `exec-boundary` 계열은 command string이 page boundary에 걸친 경우를
  확인한다.

## 6. fork에서 parent intr_frame 전달

목표:

- child가 parent의 syscall 이후 user context를 올바르게 이어받게 한다.

근거:

- reference는 child process에서 `fork()` return value가 `0`이어야 한다고
  설명한다.
- skeleton 주석은 `parent->tf`가 userland context를 들고 있지 않으므로
  `process_fork()`의 두 번째 인자를 `__do_fork()`에 전달해야 한다고 말한다.

현재 코드 기준 작업:

- `process_fork()`에서 parent의 `intr_frame`을 child 시작 함수가 볼 수 있는
  안정적인 저장 공간에 복사한다.
- `__do_fork()`에서 그 frame을 local frame으로 복사한다.
- child 쪽 frame의 return register는 `0`으로 만든다.
- parent 쪽 return value는 child pid 또는 `TID_ERROR`로 설정되게 한다.

주의:

- parent kernel stack 위의 `intr_frame` 주소를 child가 나중에 그대로
  참조하면 lifetime 문제가 생길 수 있다.
- parent는 child가 복제 성공/실패를 알릴 때까지 대기해야 한다.

## 7. fork에서 주소 공간 복제

목표:

- child가 parent의 virtual memory space를 duplicate하게 한다.

근거:

- reference는 child가 virtual memory space를 duplicate해야 한다고 설명한다.
- Project 2 template은 `pml4_for_each()`와 `duplicate_pte()`를 활용하라고
  설명한다.

현재 코드 기준 작업:

- `duplicate_pte()`에서 kernel page는 건너뛴다.
- parent의 user page를 child용 `PAL_USER` page에 복사한다.
- parent page의 writable 여부를 child mapping에도 반영한다.
- `pml4_set_page()` 실패 시 할당한 page를 정리하고 fork 실패로 이어지게
  한다.

주의:

- `pintos/include/threads/mmu.h`에 `is_writable(pte)`, `is_user_pte(pte)`
  helper가 있다.
- `pml4_for_each()`는 kernel mapping도 순회할 수 있으므로 user/kernel
  구분이 필요하다.
- VM project 이후에는 supplemental page table copy 경로도 고려해야 한다.

## 8. fork에서 file descriptor 복제

목표:

- child가 parent의 fd table을 독립적으로 duplicate하게 한다.

근거:

- reference는 child process가 file descriptor를 inherit한다고 설명한다.
- `fork` 설명은 fd를 포함한 resource가 duplicated되어야 한다고 설명한다.
- 현재 `process.c` 주석은 file object 복제에 `file_duplicate()`를 사용하라는
  힌트를 둔다.

현재 코드 기준 작업:

- parent의 `file_descriptors` 목록을 순회한다.
- 각 fd 번호를 child에서도 같은 fd 번호로 유지할지, fd allocator를 어떻게
  확장할지 정책을 정한다.
- 각 `struct file *`은 `file_duplicate()`로 복제한다.
- 중간 실패 시 이미 복제한 fd와 file object를 정리하고 fork 실패로 처리한다.

주의:

- `fork-read`, `fork-close`, `multi-child-fd`, `exec-read` 계열이 fd 복제와
  독립 close 동작을 확인한다.
- fd table을 단순히 같은 `struct file *` pointer 공유로 처리하면 file
  position과 close 독립성 요구사항을 깨뜨릴 수 있다.

## 9. fork 성공/실패 동기화

목표:

- parent가 child의 resource duplicate 성공 여부를 알고 나서 `fork()`에서
  반환하게 한다.

근거:

- reference는 parent가 child 복제 성공 여부를 알기 전에는 `fork()`에서
  반환하면 안 된다고 설명한다.
- child가 duplicate에 실패하면 parent의 `fork()` call은 `TID_ERROR`를
  반환해야 한다.

현재 코드 기준 작업:

- child metadata 또는 fork 전달용 aux에 fork 완료 동기화 상태를 둔다.
- child가 주소 공간과 fd table 복제를 마치면 성공/실패를 기록하고 parent를
  깨운다.
- parent는 그 결과에 따라 child pid 또는 `TID_ERROR`를 반환한다.

주의:

- child가 실패해서 곧바로 exit하는 경우에도 metadata가 wait/cleanup 규칙을
  깨뜨리지 않아야 한다.
- parent가 성공으로 보고 반환했는데 child는 아직 복제 중인 상태가 되면 안
  된다.

## 10. executable deny-write 유지

목표:

- 실행 중인 executable file에 write가 허용되지 않게 한다.

근거:

- reference는 실행 중인 executable에 대한 write를 deny해야 한다고 설명한다.
- `file_deny_write()`로 write를 막고, process가 실행되는 동안 해당 file을
  open 상태로 유지해야 한다고 설명한다.

현재 코드 기준 문제:

- `load()`는 executable file을 열고 `done`에서 닫는다.
- 이 구조로는 process 실행 중 deny-write 상태를 유지할 수 없다.

현재 코드 기준 작업:

- load 성공 시 executable file object를 현재 thread/process 상태에 보관한다.
- load 성공 후 `file_deny_write()`를 적용한다.
- process exit 또는 다음 exec로 실행 이미지가 바뀔 때 해당 file을 닫는다.
- load 실패 시에는 열린 file을 즉시 닫는다.

주의:

- `rox-simple`, `rox-child`, `rox-multichild` 계열이 이 동작을 확인한다.
- file을 닫으면 deny-write도 해제되므로, 실행 중에는 닫으면 안 된다.

## 11. file system 동기화 범위 정리

목표:

- file system code에 동시에 접근해 생기는 race를 막는다.

근거:

- reference는 `filesys` directory의 file system code가 여러 thread에서
  동시에 호출하기에 safe하지 않다고 설명한다.
- system call implementation은 file system code를 critical section으로
  취급해야 한다.
- `process_exec()`도 file에 접근한다.

현재 코드 기준 작업:

- file system syscall과 `process_exec()`/`load()`의 file 접근을 같은
  동기화 정책으로 보호한다.
- lock을 잡은 상태에서 user pointer 검증 실패나 process exit이 발생할 때
  resource leak 또는 lock 미해제가 생기지 않도록 경로를 정리한다.

주의:

- user memory 접근 중 page fault가 날 수 있는 설계라면 lock을 잡는 시점이
  특히 중요하다.
- file system lock은 `fork`의 fd duplicate, executable deny-write와도
  충돌 없이 동작해야 한다.

## 권장 구현 순서

1. syscall entry가 원본 `intr_frame`을 보관하도록 정리한다.
2. wait/exit용 child metadata와 lifecycle 규칙을 먼저 잡는다.
3. `process_exit()`와 exception kill 경로가 exit status를 일관되게 남기게
   한다.
4. `process_wait()`를 direct child, one-time wait, already-exited child까지
   처리하도록 구현한다.
5. exec용 address-space cleanup과 exit용 full cleanup을 분리한다.
6. `handle_exec()`를 연결하고 `cmd_line` kernel copy와 load 실패 종료를
   정리한다.
7. `process_fork()`/`__do_fork()` 사이에 parent `intr_frame` 전달 구조를
   만든다.
8. `duplicate_pte()`와 address space 복제를 완성한다.
9. fd table 복제를 추가한다.
10. fork 성공/실패 동기화를 추가한다.
11. executable deny-write와 file system lock 범위를 정리한다.

## 관련 테스트 관점

테스트는 직접 실행하지 않았다. 현재 단계에서 관련성이 큰 테스트 파일과
체크 포인트는 다음과 같다.

- `fork-once`, `fork-multiple`: fork return value, child exit status 전달
- `fork-recursive`, `multi-recurse`: 반복 생성/종료 cleanup
- `fork-read`, `fork-close`: fd table 복제와 close 독립성
- `fork-boundary`: thread name user string boundary 처리
- `exec-once`, `exec-arg`: exec 성공과 argument passing
- `exec-boundary`, `exec-bad-ptr`: exec user string 검증
- `exec-missing`: load 실패 처리
- `exec-read`, `multi-child-fd`: exec 이후 fd 유지
- `wait-simple`: child exit status 전달
- `wait-twice`: 같은 child 중복 wait 실패
- `wait-bad-pid`: direct child가 아닌 pid 처리
- `wait-killed`: kernel에 의해 종료된 child의 `-1` status
- `rox-simple`, `rox-child`, `rox-multichild`: executable deny-write
- `no-vm/multi-oom`: resource exhaustion과 실패 cleanup

실행이 필요하면 `docs/ai/006_단계별_테스트/012_userprog_03_process_fork_exec_wait.md`
의 명령을 기준으로 사용자가 직접 실행한다.

## 최종 점검 질문

구현 뒤에는 다음 질문에 모두 답할 수 있어야 한다.

- `fork()` parent는 child 복제 성공/실패를 확인한 뒤 반환하는가?
- child의 `fork()` return value는 항상 `0`인가?
- parent의 fd close가 child fd를 닫지 않고, child의 fd close가 parent fd를
  닫지 않는가?
- `exec()` 뒤에도 기존 fd가 유지되는가?
- `exec()` 실패 시 process가 `-1`로 종료되는가?
- direct child가 아닌 pid에 대한 `wait()`가 즉시 `-1`을 반환하는가?
- 같은 child에 대한 두 번째 `wait()`가 즉시 `-1`을 반환하는가?
- child가 parent보다 먼저 죽어도 parent가 나중에 status를 회수할 수 있는가?
- parent가 child를 wait하지 않고 죽어도 child와 metadata가 누수되지 않는가?
- exception으로 죽은 child를 wait하면 `-1`이 반환되는가?
- initial process 종료 전 Pintos가 먼저 종료되지 않는가?
- 실행 중인 executable에 write가 deny되는가?
- file system 접근이 공통 lock 또는 동등한 동기화 정책으로 보호되는가?
