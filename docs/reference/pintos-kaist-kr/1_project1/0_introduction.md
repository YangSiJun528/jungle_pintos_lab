# Project1: Threads (스레드)

이 assignment(과제)에서는 최소한으로 동작하는 thread system(스레드 시스템)을 제공합니다. 여러분의 작업은 이 system의 기능을 확장하면서 synchronization problem(동기화 문제)을 더 잘 이해하는 것입니다. 이번 assignment에서는 주로 `threads` directory(디렉터리)에서 작업하며, 일부 작업은 `devices` directory에서 진행합니다. compilation(컴파일)은 `threads` directory에서 수행해야 합니다. 이 project 설명을 읽기 전에 [Synchronization](../5_appendix/1_synchronization.md) 자료를 최소한 훑어보세요.

## Background (배경)

### Understanding Threads (Thread 이해하기)

첫 단계는 초기 thread system의 code(코드)를 읽고 이해하는 것입니다. Pintos는 이미 thread creation(스레드 생성)과 thread completion(스레드 종료), thread 사이를 전환하는 단순한 scheduler(스케줄러), 그리고 synchronization primitive(동기화 기본 도구: semaphore, lock, condition variable, optimization barrier)를 구현하고 있습니다.

이 code 중 일부는 약간 이해하기 어려워 보일 수 있습니다. [introduction](https://casys-kaist.github.io/pintos-kaist/introduction)에 설명된 대로 base system(기본 시스템)을 아직 compile하고 실행해 보지 않았다면 지금 해 보세요. source code(소스 코드)의 일부를 읽으며 어떤 일이 일어나는지 확인할 수 있습니다. 원한다면 거의 어디에나 `printf()` 호출을 추가한 뒤 다시 compile하고 실행하여 어떤 일이 어떤 순서로 발생하는지 볼 수 있습니다. 또한 debugger(디버거)에서 kernel(운영체제 핵심부)을 실행하고, 흥미로운 지점에 breakpoint(중단점)를 설정하고, code를 single-step(한 줄씩 실행)하며 data(데이터)를 검사할 수도 있습니다.

thread가 생성되면 schedule(실행 선택)될 새 context(실행 문맥)를 만드는 것입니다. `thread_create()`에 argument(인자)로 이 context에서 실행할 function(함수)을 제공합니다. thread가 처음 schedule되어 실행되면, 그 function의 시작부터 그 context 안에서 실행됩니다. function이 return(반환)하면 thread는 종료됩니다. 따라서 각 thread는 Pintos 안에서 실행되는 작은 program처럼 동작하며, `thread_create()`에 전달된 function은 `main()`처럼 동작합니다.

어떤 시점에도 정확히 하나의 thread만 실행되며, 나머지가 있다면 비활성 상태가 됩니다. scheduler는 다음에 실행할 thread를 결정합니다. 어떤 시점에도 실행할 준비가 된 thread가 없으면, `idle()`에 구현된 특별한 *idle* thread(유휴 스레드)가 실행됩니다. 한 thread가 다른 thread가 어떤 일을 하기를 기다려야 할 때 synchronization primitive가 context switch(문맥 전환)를 강제할 수 있습니다.

context switch의 mechanism(동작 방식)은 `threads/thread.c`의 `thread_launch()`에 있습니다. 이해하지 않아도 됩니다. 이 함수는 현재 실행 중인 thread의 state(상태)를 저장하고, 전환할 thread의 state를 복원합니다.

GDB debugger를 사용하여 context switch를 천천히 trace(추적)해 보세요([GDB](0_introduction.md) 참조). `schedule()`에 breakpoint를 걸고 시작한 뒤, 거기서부터 single-step할 수 있습니다. 각 thread의 address(주소)와 state, 그리고 각 thread의 call stack(호출 스택)에 어떤 procedure(프로시저)가 있는지 계속 추적하세요. 한 thread가 `do_iret()`에서 `iret`을 실행하면 다른 thread가 실행되기 시작한다는 것을 볼 수 있습니다.

**Warning (주의)**: Pintos에서 각 thread는 크기가 4 kB보다 약간 작은, 작고 고정된 execution stack(실행 스택)을 할당받습니다. kernel은 stack overflow(스택 넘침)를 감지하려고 하지만 완벽하게 감지할 수는 없습니다. 예를 들어 `int buf[1000];`처럼 큰 data structure(자료구조)를 non-static local variable(비정적 지역 변수)로 선언하면, 이해하기 어려운 kernel panic(커널 패닉) 같은 이상한 문제를 일으킬 수 있습니다. stack allocation(스택 할당)의 대안으로는 page allocator와 block allocator가 있습니다([Memory Allocation](../5_appendix/2_memory_allocation.md) 참조).

### Source Files (소스 파일)

다음은 `threads`와 `include/threads` directory에 있는 file의 간략한 overview(개요)입니다. 대부분의 code를 수정할 필요는 없지만, 이 overview가 어떤 code를 살펴볼지 시작점을 제공하기를 바랍니다.

#### `threads` codes

  - `loader.S`, `loader.h`

kernel loader(커널 로더)입니다. PC BIOS가 memory(메모리)에 load(적재)하는 512 bytes의 code와 data로 assemble(어셈블)되며, 이어서 disk(디스크)에서 kernel을 찾아 memory에 load하고 `start.S`의 `bootstrap()`으로 jump(점프)합니다. 이 code를 보거나 수정할 필요는 없습니다. `start.S`는 memory protection(메모리 보호)에 필요한 기본 설정을 수행하고 64bit long mode(64비트 롱 모드)로 jump합니다. loader와 달리 이 code는 실제로 kernel의 일부입니다.

  - `kernel.lds.S`

kernel을 link(링크)하는 데 사용하는 linker script(링커 스크립트)입니다. kernel의 load address(적재 주소)를 설정하고 `start.S`가 kernel image(커널 이미지)의 앞부분 근처에 오도록 배치합니다. 역시 이 code를 보거나 수정할 필요는 없지만, 궁금할 때를 위해 여기에 있습니다.

  - `init.c`, `init.h`

`main()`을 포함한 kernel initialization(커널 초기화)입니다. `main()`은 kernel의 `main program`입니다. 최소한 무엇이 initialize(초기화)되는지 보기 위해 `main()`을 살펴보세요. 여기에 여러분의 initialization code를 추가하고 싶을 수도 있습니다.

  - `thread.c`, `thread.h`

기본 thread support(스레드 지원)입니다. 작업의 많은 부분이 이 file들에서 이루어집니다. `thread.h`는 `struct thread`를 정의하며, 네 project 모두에서 수정할 가능성이 큽니다. 자세한 내용은 [Threads](../5_appendix/0_threads.md)를 참조하세요.

  - `palloc.c`, `palloc.h`

system memory(시스템 메모리)를 4 kB page(고정 크기 메모리 단위)의 배수로 나누어 주는 page allocator(페이지 할당자)입니다. 자세한 내용은 [Page Allocator](../5_appendix/2_memory_allocation.md#Page%20Allocator)를 참조하세요.

  - `malloc.c`, `malloc.h`

kernel용 `malloc()`과 `free()`의 단순한 구현입니다. 자세한 내용은 [Block Allocator](../5_appendix/2_memory_allocation.md#Block%20Allocator)를 참조하세요.

  - `interrupt.c`, `interrupt.h`

기본 interrupt(현재 실행을 멈추고 처리하는 이벤트) handling(처리)과 interrupt를 켜고 끄는 함수입니다.

  - `intr-stubs.S`, `intr-stubs.h`

low-level interrupt handling(저수준 인터럽트 처리)을 위한 assembly code(어셈블리 코드)입니다.

  - `synch.c`, `synch.h`

기본 synchronization primitive입니다. semaphore(세마포어), lock(잠금), condition variable(조건 변수), optimization barrier(최적화 장벽)를 포함합니다. 네 project 모두에서 synchronization을 위해 이것들을 사용해야 합니다. 자세한 내용은 [Synchronization](../5_appendix/1_synchronization.md)을 참조하세요.

  - `mmu.c`, `mmu.h`

x86-64 page table(페이지 테이블) operation(연산)을 위한 함수입니다. lab1 이후에 이 file을 더 자세히 보게 됩니다.

  - `io.h`

I/O port access(입출력 포트 접근)를 위한 함수입니다. 주로 여러분이 건드릴 필요가 없는 `devices` directory의 source code가 사용합니다.

  - `vaddr.h`, `pte.h`

virtual address(가상 주소)와 page table entry(페이지 테이블 항목)를 다루는 함수와 macro(매크로)입니다. project 3에서 더 중요해집니다. 지금은 무시해도 됩니다.

  - `flags.h`

x86-64 `flags` register(레지스터)의 몇 bit(비트)를 정의하는 macro입니다. 아마 관심 가질 필요가 없습니다.

#### `devices` codes

기본 threaded kernel(스레드 기반 커널)은 `devices` directory에도 다음 file들을 포함합니다.

  - `timer.c`, `timer.h`

기본적으로 초당 100번 tick(틱)하는 system timer(시스템 타이머)입니다. 이번 project에서 이 code를 수정합니다.

  - `vga.c`, `vga.h`

VGA display driver(디스플레이 드라이버)입니다. 화면에 text를 쓰는 일을 담당합니다. 이 code를 볼 필요는 없습니다. `printf()`가 대신 VGA display driver를 호출하므로, 이 code를 직접 호출할 이유는 거의 없습니다.

  - `serial.c`, `serial.h`

serial port driver(직렬 포트 드라이버)입니다. 역시 `printf()`가 대신 이 code를 호출하므로 직접 호출할 필요가 없습니다. serial input(직렬 입력)은 input layer(입력 계층)로 전달하여 처리합니다(아래 참조).

  - `block.c`, `block.h`

block device(블록 장치), 즉 고정 크기 block 배열로 구성된 random-access(임의 접근) disk-like device(디스크 같은 장치)에 대한 abstraction layer(추상화 계층)입니다. 기본 Pintos는 IDE disk와 partition이라는 두 종류의 block device를 지원합니다. 종류와 관계없이 block device는 project 2가 되기 전까지는 실제로 사용되지 않습니다.

  - `ide.c`, `ide.h`

최대 4개의 IDE disk에서 sector(섹터)를 읽고 쓰는 기능을 지원합니다.

  - `partition.c`, `partition.h`

disk의 partition 구조를 이해하여, 하나의 disk를 독립적으로 사용할 수 있는 여러 region(영역, partition)으로 나눌 수 있게 합니다.

  - `kbd.c`, `kbd.h`

keyboard driver(키보드 드라이버)입니다. keystroke(키 입력)를 처리하여 input layer로 전달합니다(아래 참조).

  - `input.c`, `input.h`

input layer입니다. keyboard나 serial driver가 전달한 input character(입력 문자)를 queue(큐)에 넣습니다.

  - `intq.c`, `intq.h`

kernel thread와 interrupt handler(인터럽트 처리 함수)가 모두 접근하려는 circular queue(원형 큐)를 관리하기 위한 interrupt queue(인터럽트 큐)입니다. keyboard와 serial driver가 사용합니다.

  - `rtc.c`, `rtc.h`

real-time clock driver(실시간 시계 드라이버)입니다. kernel이 현재 날짜와 시간을 알 수 있게 합니다. 기본적으로는 random number generator(난수 생성기)의 초기 seed(시드)를 선택하기 위해 `thread/init.c`에서만 사용합니다.

  - `speaker.c`, `speaker.h`

PC speaker에서 tone(소리)을 낼 수 있는 driver입니다.

  - `pit.c`, `pit.h`

8254 Programmable Interrupt Timer를 configure(설정)하는 code입니다. 각 device가 PIT의 output channel(출력 채널) 중 하나를 사용하기 때문에, 이 code는 `devices/timer.c`와 `devices/speaker.c` 양쪽에서 사용됩니다.

#### `lib` codes

마지막으로 `lib`와 `lib/kernel`에는 유용한 library routine(라이브러리 루틴)이 들어 있습니다. `lib/user`는 project 2부터 user program에서 사용되지만 kernel의 일부는 아닙니다. 조금 더 자세히 보면 다음과 같습니다.

  - `ctype.h`, `inttypes.h`, `limits.h`, `stdarg.h`, `stdbool.h`, `stddef.h`, `stdint.h`, `stdio.c`, `stdio.h`, `stdlib.c`, `stdlib.h`, `string.c`, `string.h`

standard C library의 subset(부분집합)입니다.

  - `debug.c`, `debug.h`

debugging을 돕는 함수와 macro입니다. 자세한 내용은 [Debugging Tools](0_introduction.md)를 참조하세요.

  - `random.c`, `random.h`

pseudo-random number generator(의사 난수 생성기)입니다. 실제 random value(난수 값)의 sequence(순서)는 Pintos 실행마다 달라지지 않습니다.

  - `round.h`

rounding(반올림/정렬)을 위한 macro입니다.

  - `syscall-nr.h`

system call number(시스템 콜 번호)입니다. project 2까지는 사용되지 않습니다.

  - `kernel/list.c`, `kernel/list.h`

doubly linked list(이중 연결 리스트) 구현입니다. Pintos code 전반에서 사용되며, project 1에서도 여러분이 몇 군데에서 직접 사용하고 싶을 가능성이 큽니다. **시작하기 전에 이 code를 훑어보는 것을 권장합니다. 특히 header file의 comment(주석)를 보세요.**

  - `kernel/bitmap.c`, `kernel/bitmap.h`

bitmap(비트맵) 구현입니다. 원한다면 여러분 code에서 사용할 수 있지만, project 1에서는 필요할 일이 별로 없을 것입니다.

  - `kernel/hash.c`, `kernel/hash.h`

hash table 구현입니다. project 3에서 유용할 가능성이 큽니다.

  - `kernel/console.c`, `kernel/console.h`, `kernel/stdio.h`

`printf()`와 몇 가지 다른 함수를 구현합니다.

### Synchronization (동기화)

올바른 synchronization은 이 문제들의 solution에서 중요한 부분입니다. 어떤 synchronization problem이든 interrupt를 끄면 쉽게 해결할 수 있습니다. interrupt가 꺼져 있는 동안에는 concurrency(동시성)가 없으므로 race condition(경쟁 상태)이 발생할 가능성도 없습니다. 그래서 모든 synchronization problem을 이런 방식으로 해결하고 싶어질 수 있지만, 그렇게 하지 마세요. 대신 synchronization problem 대부분은 semaphore, lock, condition variable을 사용해 해결하세요. 어떤 synchronization primitive를 어떤 상황에서 사용할 수 있는지 확실하지 않다면 synchronization tour section([Synchronization](../5_appendix/1_synchronization.md) 참조)이나 `threads/synch.c`의 comment를 읽으세요.

Pintos project에서 interrupt disable(인터럽트 비활성화)로 해결하는 것이 가장 적절한 유일한 문제 유형은 kernel thread와 interrupt handler 사이에 공유되는 data를 조정하는 것입니다. interrupt handler는 sleep(잠들기)할 수 없기 때문에 lock을 acquire할 수 없습니다. 즉, kernel thread와 interrupt handler 사이에 공유되는 data는 kernel thread 안에서 interrupt를 끄는 방식으로 보호해야 합니다.

이번 project에서는 interrupt handler에서 thread state의 작은 부분에 접근하기만 하면 됩니다. alarm clock에서는 timer interrupt가 sleeping thread(잠든 스레드)를 깨워야 합니다. advanced scheduler에서는 timer interrupt가 몇 가지 global variable(전역 변수)과 per-thread variable(스레드별 변수)에 접근해야 합니다. kernel thread에서 이 변수들에 접근할 때에는 timer interrupt가 끼어들지 못하도록 interrupt를 disable해야 합니다.

interrupt를 끌 때에는 가능한 한 작은 code 구간에서만 끄도록 주의하세요. 그렇지 않으면 timer tick이나 input event(입력 이벤트) 같은 중요한 것을 잃을 수 있습니다. interrupt를 끄면 interrupt handling latency(인터럽트 처리 지연)도 증가하므로, 지나치면 machine이 둔하게 느껴질 수 있습니다.

`synch.c`의 synchronization primitive 자체도 interrupt를 disable하여 구현되어 있습니다. 여기에서 interrupt가 disabled된 상태로 실행되는 code의 양을 늘려야 할 수도 있지만, 그래도 최소한으로 유지하려고 해야 합니다.

interrupt disable은 debugging에 유용할 수 있습니다. 어떤 code 구간이 interrupt되지 않는지 확인하고 싶을 때 사용할 수 있습니다. project를 제출하기 전에는 debugging code를 제거해야 합니다. 단순히 comment out(주석 처리)하지 마세요. code를 읽기 어렵게 만들 수 있습니다.

제출물에는 busy waiting이 없어야 합니다. `thread_yield()`를 호출하는 tight loop(촘촘한 반복문)도 busy waiting의 한 형태입니다.

### Development Suggestions (개발 제안)

과거에는 많은 group(그룹)이 assignment를 조각으로 나눈 뒤, 각 group member(구성원)가 자신의 부분을 deadline(마감) 직전까지 작업하고, 마지막에 모여 code를 합쳐 제출했습니다. 이는 나쁜 생각입니다. 이 접근을 권장하지 않습니다. 이렇게 하는 group은 두 변경사항이 서로 conflict(충돌)하여 마지막 순간에 많은 debugging이 필요해지는 경우가 많습니다. 어떤 group은 compile이나 boot(부팅)조차 되지 않는 code를 제출하기도 했고, test 통과는 말할 것도 없었습니다.

대신 [git](https://git-scm.com/docs/gittutorial) 같은 source code control system(소스 코드 관리 시스템)을 사용하여 team의 변경사항을 일찍, 자주 integrate(통합)하는 것을 권장합니다. 이렇게 하면 모든 사람이 code가 끝난 뒤에야 보는 것이 아니라 작성되는 즉시 서로의 code를 볼 수 있으므로 surprise(예상치 못한 문제)가 줄어듭니다. 이런 system은 변경사항을 review(검토)하고, 어떤 변경이 bug를 도입했을 때 동작하던 code version(버전)으로 돌아가는 것도 가능하게 합니다.

이 project와 이후 project를 진행하다 보면 이해할 수 없는 bug를 만나게 될 것입니다. 그럴 때는 유용한 debugging tip(디버깅 팁)이 가득한 debugging tools appendix를 다시 읽으세요([Debugging Tools](0_introduction.md) 참조). 특히 kernel panic이나 assertion failure(단언 실패)를 최대한 활용하는 데 도움이 되는 backtrace section([Backtraces](0_introduction.md) 참조)을 꼭 읽으세요.
