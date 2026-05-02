# intr_frame과 userprog 흐름 정리

이 문서는 현재 과제 기준인 CS330 64-bit KAIST Pintos에서 `struct intr_frame`이 무엇이고, Project 2 `userprog`에서 어떤 의미로 쓰이는지 정리한다. 구현 코드는 포함하지 않고, `process_exec()`, argument passing, syscall, exception, fork에서 필요한 개념을 중심으로 설명한다.

## 한 줄 결론

`struct intr_frame`은 x86-64 CPU가 user mode와 kernel mode 사이를 오갈 때 필요한 register 상태를 C 코드에서 다룰 수 있게 만든 "저장된 실행 문맥"이다.

Project 2에서는 특히 다음 네 곳에서 중요하다.

- 새 user program을 시작할 때: `rip`, `rsp`, `R.rdi`, `R.rsi` 등을 채워 `do_iret()`으로 user mode에 들어간다.
- syscall을 처리할 때: user가 syscall 직전에 넣어 둔 register 값이 `intr_frame`에 저장되어 handler로 전달된다.
- syscall return value를 줄 때: `R.rax`를 바꾸면 user program이 syscall return value로 받는다.
- `fork`를 구현할 때: parent의 user context를 child가 이어받아야 하므로 syscall 당시의 `intr_frame`이 중요하다.

## 이 프로젝트는 x86-64 기준이다

문서나 강의 자료에서 "x86"이라고 뭉뚱그려 말할 수 있지만, 현재 Pintos tree는 x86-64 기준이다. 따라서 register 이름도 64-bit 이름을 쓴다.

| 32-bit에서 자주 보이는 이름 | 이 프로젝트에서 주로 보는 64-bit 이름 |
|---|---|
| `eip` | `rip` |
| `esp` | `rsp` |
| `eax` | `rax` |
| `edi` | `rdi` |
| `esi` | `rsi` |

`rip`는 다음에 실행할 instruction address이고, `rsp`는 stack pointer다.

## intr_frame의 큰 역할

CPU가 user code를 실행하다가 syscall, interrupt, exception 때문에 kernel로 들어오면 기존 user register 값을 잃어버리면 안 된다. kernel이 일을 끝낸 뒤 다시 user code로 돌아가려면 다음 정보가 필요하다.

- 어디로 돌아갈지: `rip`
- 어떤 user stack을 다시 사용할지: `rsp`
- user mode segment는 무엇인지: `cs`, `ss`, `ds`, `es`
- flags 상태는 무엇인지: `eflags`
- 일반 register 값은 무엇이었는지: `rax`, `rdi`, `rsi`, `rdx`, ...

Pintos는 이런 값을 `struct intr_frame` 형태로 kernel stack 위에 저장하거나, 새 process 시작을 위해 kernel이 직접 만들어 둔다.

## 주요 필드 의미

`struct intr_frame`은 `pintos/include/threads/interrupt.h`에 정의되어 있다. userprog에서 자주 보는 필드만 정리하면 다음과 같다.

| 필드 | 의미 | userprog에서 중요한 곳 |
|---|---|---|
| `R.rax` | general register `rax` | syscall 번호, syscall return value |
| `R.rdi` | general register `rdi` | syscall 1번째 인자, `_start(argc, argv)`의 `argc` |
| `R.rsi` | general register `rsi` | syscall 2번째 인자, `_start(argc, argv)`의 `argv` |
| `R.rdx` | general register `rdx` | syscall 3번째 인자 |
| `R.r10` | general register `r10` | syscall 4번째 인자 |
| `R.r8` | general register `r8` | syscall 5번째 인자 |
| `R.r9` | general register `r9` | syscall 6번째 인자 |
| `rip` | user로 돌아갔을 때 실행할 instruction address | executable entry point, syscall return 위치, fault instruction |
| `rsp` | user로 돌아갔을 때 사용할 stack pointer | argument passing, user stack 복구 |
| `cs` | code segment selector | user exception인지 kernel exception인지 구분 |
| `ss` | stack segment selector | user stack segment |
| `eflags` | CPU flags | interrupt enable 상태 등 |
| `vec_no` | interrupt/exception vector number | exception/debugging |
| `error_code` | exception error code | page fault 원인 판단 |

## process_exec에서의 intr_frame

`process_exec()`는 새 executable로 현재 실행 문맥을 바꾼다. 이때 아직 user program이 실행된 적이 없으므로, CPU가 저장해 준 user context가 있는 것이 아니다. kernel이 직접 "처음 user mode에 들어갈 상태"를 만들어야 한다.

현재 skeleton 흐름은 개념적으로 다음과 같다.

1. local `intr_frame`을 하나 준비한다.
2. user code/data segment와 flags를 설정한다.
3. 기존 process context를 정리한다.
4. `load()`로 executable을 load한다.
5. `load()`가 user stack을 만들고 `rip`, `rsp`를 채운다.
6. argument passing이 `rsp`, `R.rdi`, `R.rsi`를 추가로 조정한다.
7. `do_iret()`으로 `intr_frame` 값을 CPU register에 복원하고 user mode로 들어간다.

여기서 핵심 필드는 다음이다.

| 필드 | process 시작 시 의미 |
|---|---|
| `rip` | user program entry point. loader가 ELF entry address를 넣는다. |
| `rsp` | user stack pointer. 처음에는 `USER_STACK` 근처이고, argument passing 후 더 낮은 주소로 내려간다. |
| `R.rdi` | `_start()`의 첫 번째 인자 `argc` |
| `R.rsi` | `_start()`의 두 번째 인자 `argv` |
| `cs` | user code segment |
| `ss`, `ds`, `es` | user data/stack segment |
| `eflags` | user mode 진입 시 필요한 flags |

즉 argument passing은 단순히 stack에 문자열을 복사하는 일이 아니다. 최종적으로 `intr_frame`의 `rsp`, `R.rdi`, `R.rsi`까지 맞춰야 `_start(argc, argv)`가 정상적으로 시작된다.

## do_iret의 의미

`do_iret(struct intr_frame *tf)`는 `intr_frame`에 저장된 값을 실제 CPU register로 복원한 뒤 `iretq`로 kernel mode에서 user mode로 나간다.

중요한 감각은 다음이다.

- `do_iret()`은 `tf`를 보고 register들을 복원한다.
- 마지막에 `iretq`가 `rip`, `cs`, `eflags`, `rsp`, `ss`를 사용해 user mode로 돌아간다.
- 따라서 `tf->rip`과 `tf->rsp`가 틀리면 user program이 엉뚱한 instruction이나 stack에서 시작한다.

`process_exec()`에서 `do_iret()`을 호출하면 성공 시 돌아오지 않는다. CPU가 user program 실행으로 넘어가기 때문이다.

## syscall에서의 intr_frame

user program이 syscall을 호출할 때는 register에 syscall 번호와 인자를 넣고 `syscall` instruction을 실행한다.

Project 2 reference 기준 syscall register 규칙은 다음이다.

| 값 | 위치 |
|---|---|
| syscall number | `R.rax` |
| 1번째 인자 | `R.rdi` |
| 2번째 인자 | `R.rsi` |
| 3번째 인자 | `R.rdx` |
| 4번째 인자 | `R.r10` |
| 5번째 인자 | `R.r8` |
| 6번째 인자 | `R.r9` |
| return value | `R.rax` |

여기서 일반 함수 호출과 다른 점은 4번째 인자가 `rcx`가 아니라 `r10`이라는 점이다.

syscall 진입 시 `syscall_entry`는 user register 상태를 kernel stack에 `intr_frame` 모양으로 저장하고, 그 주소를 `syscall_handler(struct intr_frame *f)`에 넘긴다. 따라서 syscall handler는 `f->R.rax`에서 syscall 번호를 읽고, 필요한 인자를 `f->R.rdi`, `f->R.rsi` 등에서 읽는다.

syscall이 값을 반환해야 한다면 `f->R.rax`를 원하는 return value로 바꾸면 된다. handler가 끝난 뒤 assembly 경로가 이 값을 user `rax`로 복원하므로, user program은 syscall return value로 받는다.

## syscall의 rip/rsp

syscall path에서도 `intr_frame`에는 user로 돌아갈 위치와 stack이 들어 있다.

- `rip`: syscall 처리 후 user code에서 이어 실행할 위치
- `rsp`: syscall을 호출한 user stack pointer

x86-64 `syscall` instruction은 return address와 flags를 일반 interrupt처럼 stack에 자동 push하지 않는다. 대신 return RIP는 `rcx`, flags는 `r11` 쪽에 담기는 방식이다. Pintos의 `syscall_entry`는 이 값을 `intr_frame` 형태로 직접 맞춰 저장한다.

따라서 syscall handler 입장에서는 일반 interrupt handler처럼 `struct intr_frame *f`를 받지만, 그 frame은 `syscall_entry`가 만들어 준 frame이다.

## exception과 page fault에서의 intr_frame

user program이 잘못된 instruction을 실행하거나 잘못된 memory에 접근하면 exception이 발생한다. Pintos는 exception handler에도 `struct intr_frame *f`를 넘긴다.

exception 처리에서 중요한 필드는 다음이다.

| 필드 | 의미 |
|---|---|
| `f->cs` | fault가 user code에서 왔는지 kernel code에서 왔는지 판단한다. |
| `f->rip` | fault를 일으킨 instruction address다. |
| `f->rsp` | fault 당시 user stack pointer다. |
| `f->vec_no` | 어떤 exception vector인지 나타낸다. |
| `f->error_code` | page fault 같은 exception의 상세 원인 bit를 담는다. |

page fault에서는 fault가 발생한 주소 자체가 `f->rip`에 들어있는 것이 아니다. fault가 발생한 virtual address는 CR2 register에서 읽는다. `f->rip`은 그 접근을 시도한 instruction의 주소다.

page fault `error_code`는 `pintos/include/userprog/exception.h`의 bit로 해석한다.

| bit | 의미 |
|---|---|
| `PF_P` | 0이면 not-present page, 1이면 권한 위반 |
| `PF_W` | 0이면 read, 1이면 write 접근 |
| `PF_U` | 0이면 kernel mode 접근, 1이면 user mode 접근 |

Project 2의 user pointer 검증에서는 잘못된 user pointer가 kernel panic으로 이어지지 않게 하는 것이 중요하다. 구현 방식에 따라 page fault handler를 일부 수정할 수 있다는 reference 설명도 이 흐름과 연결된다.

## fork에서의 intr_frame

`fork`는 parent process의 실행 문맥을 child가 이어받게 해야 한다. 문서상 child는 parent의 resource와 virtual memory를 복제하고, child에서는 `fork()` return value가 0이어야 한다.

여기서 중요한 점은 `thread_current()->tf`가 parent의 userland context를 들고 있다고 생각하면 안 된다는 것이다. skeleton 주석도 `parent->tf`가 userland context가 아니므로 `process_fork()`의 두 번째 인자인 `intr_frame`을 전달해야 한다고 설명한다.

개념적으로 필요한 상태는 다음이다.

| 대상 | fork 이후 보여야 하는 값 |
|---|---|
| parent | `fork()`가 child pid를 return한 것처럼 이어 실행 |
| child | 같은 user instruction 이후에서 이어 실행하되, `fork()` return value는 0 |

이 차이는 결국 parent와 child가 사용하는 `intr_frame`의 `R.rax` 값을 다르게 만들어야 한다는 뜻이다. child의 address space와 fd 등을 복제하는 것도 중요하지만, user code가 보는 실행 결과는 `intr_frame`의 register 상태에서 결정된다.

## thread->tf와 syscall intr_frame은 다르다

`struct thread` 안에도 `struct intr_frame tf`가 있다. 하지만 이것을 항상 "현재 user process의 saved user register"라고 생각하면 안 된다.

`thread->tf`는 thread switching을 위해 thread.c가 쓰는 실행 문맥이다. kernel thread를 처음 시작할 때도 `tf.rip`, `tf.R.rdi`, `tf.R.rsi`를 채워 `kernel_thread(function, aux)`로 들어가게 한다.

반면 syscall handler에 들어온 `struct intr_frame *f`는 user program이 syscall을 실행한 바로 그 순간의 user register 상태다. Project 2의 syscall, fork 구현에서 더 직접적으로 필요한 것은 이 `f`다.

정리하면 다음처럼 구분한다.

| frame | 어디서 생김 | 주 용도 |
|---|---|---|
| `process_exec()`의 local frame | kernel이 새 user program 시작을 위해 직접 만듦 | 최초 user mode 진입 |
| syscall handler의 `f` | `syscall_entry`가 kernel stack에 구성 | syscall 번호/인자/반환값 처리 |
| exception handler의 `f` | interrupt/exception entry가 구성 | fault 원인 판단, process kill 또는 page fault 처리 |
| `thread->tf` | thread scheduler/context switch용 | kernel thread switching, 최초 kernel thread 진입 |

## userprog에서 특히 봐야 하는 필드

Project 2를 진행하면서 우선순위를 둬서 봐야 하는 필드는 다음이다.

| 우선 | 필드 | 이유 |
|---:|---|---|
| 1 | `R.rax` | syscall 번호와 return value가 모두 여기에 걸린다. |
| 2 | `R.rdi`, `R.rsi`, `R.rdx`, `R.r10`, `R.r8`, `R.r9` | syscall 인자를 읽는 위치다. argument passing에서는 `R.rdi`, `R.rsi`도 중요하다. |
| 3 | `rsp` | user stack pointer다. argument passing의 최종 stack 위치가 여기에 반영된다. |
| 4 | `rip` | user program 시작 주소, syscall 복귀 주소, fault instruction 주소다. |
| 5 | `cs` | user fault인지 kernel fault인지 구분한다. |
| 6 | `error_code` | page fault 원인 분석에 필요하다. |

## 헷갈리기 쉬운 포인트

### intr_frame은 user stack에 있는가

대부분의 handler에서 받는 `intr_frame`은 user stack이 아니라 kernel stack 위에 있다. user mode로 돌아갈 때 `rsp` 필드가 user stack pointer로 복원될 뿐이다.

### argument passing에서 문자열을 intr_frame에 넣는가

아니다. 문자열과 `argv` 배열은 user stack에 놓는다. `intr_frame`에는 그 결과로 결정된 `argc`, `argv`, `rsp`만 반영한다.

### syscall 인자와 일반 함수 인자는 같은가

앞부분은 비슷하지만 완전히 같지는 않다. 일반 함수 호출의 4번째 인자는 `rcx`지만, syscall의 4번째 인자는 `r10`이다. syscall 번호는 `rax`에 들어간다.

### page fault의 주소는 f->rip인가

아니다. `f->rip`은 fault를 일으킨 instruction 주소다. 접근하려던 virtual address는 CR2에서 읽는다.

### fork에서 parent->tf를 복사하면 되는가

안 된다. skeleton 주석상 `parent->tf`는 userland context를 담고 있지 않다. syscall 당시 handler로 전달된 `intr_frame`을 기준으로 child의 시작 context를 만들어야 한다.

## userprog 작업별 intr_frame 체크리스트

### argument passing

- `R.rdi`가 `argc`가 되는가
- `R.rsi`가 `argv[0]` 주소가 되는가
- `rsp`가 fake return address 위치를 가리키는가
- `rip`는 executable entry point인가

### syscall

- `R.rax`에서 syscall number를 읽는가
- 인자 register 순서를 syscall convention대로 읽는가
- return value를 `R.rax`에 써 주는가
- user pointer 인자는 register 값 자체가 아니라 user virtual address임을 인식하는가

### exception/page fault

- `cs`로 user/kernel fault를 구분하는가
- page fault 주소는 CR2에서 읽는다는 점을 혼동하지 않는가
- `error_code` bit로 read/write, user/kernel, present 여부를 해석하는가

### fork

- parent의 syscall-time `intr_frame`을 child에 전달해야 한다는 점을 반영하는가
- child의 `fork()` return value가 0이 되도록 register 상태를 구분하는가
- `rip`/`rsp`가 parent와 같은 user 흐름을 이어가게 되는가

## 참고 자료

- `pintos/include/threads/interrupt.h`
- `pintos/userprog/syscall-entry.S`
- `pintos/threads/intr-stubs.S`
- `pintos/threads/thread.c`
- `pintos/userprog/process.c`
- `pintos/userprog/syscall.c`
- `pintos/userprog/exception.c`
- `docs/reference/pintos-kaist-original/2_project2/1_argument_passing.md`
- `docs/reference/pintos-kaist-original/2_project2/3_system_call.md`
- `docs/reference/pintos-kaist-original/2_project2/0_introduction.md`
