# FAQ

### How much code will I need to write? (얼마나 많은 코드를 작성해야 하나요?)

다음은 `git diff --stat`으로 생성한 reference solution(참고 구현)의 요약입니다. 마지막 행은 insert(추가)와 delete(삭제)된 총 line(줄) 수를 보여 줍니다. 변경된 line은 insertion과 deletion 양쪽으로 계산됩니다. reference solution은 가능한 solution 중 하나일 뿐입니다. 다른 solution도 많이 가능하며, 그중 많은 solution은 reference solution과 크게 다를 수 있습니다. 훌륭한 solution 중에는 reference solution이 수정한 file을 모두 수정하지 않는 것도 있고, reference solution이 수정하지 않은 file을 수정하는 것도 있을 수 있습니다.

    
    
    devices/timer.c               |  29 +++++++++++++++++++++++++++++
    include/threads/fixed-point.h |  10 ++++++++++
    include/threads/synch.h       |   4 ++++
    include/threads/thread.h      |  21 +++++++++++++++++++++
    threads/synch.c               | 143 +++++++++++++++++++++++++++++++++++++++++++-
    threads/thread.c              | 135 ++++++++++++++++++++++++++++++++++++++++----
    6 files changed, 330 insertions(+), 12 deletions(-)
    

`fixed-point.h`는 reference solution이 추가한 새 file입니다.

### How do I update the `Makefile`s when I add a new source file? (새 source file을 추가할 때 `Makefile`은 어떻게 갱신하나요?)

`.c` file을 추가하려면 각 subdirectory(하위 디렉터리)의 `targets.mk` file을 edit(수정)하세요. 새 file을 variable(변수) `dir_SRC`에 추가합니다. 여기서 dir은 `targets.mk`가 위치한 directory입니다. 그런 다음 make를 실행하세요. 새 file이 compile(컴파일)되지 않으면 `make clean`을 실행한 뒤 다시 시도하세요. 새 `.h` file은 `Makefile`을 수정할 필요가 없습니다.

### What does warning: no previous prototype for `func` mean? (`warning: no previous prototype for func`는 무슨 뜻인가요?)

static(정적)이 아닌 function(함수)을 prototype(원형 선언) 없이 정의했다는 뜻입니다. non-static function은 다른 `.c` file에서 사용되도록 의도된 것이므로, 안전을 위해 정의보다 먼저 include(포함)되는 header file(헤더 파일)에 prototype이 있어야 합니다. 문제를 고치려면 include하는 header file에 prototype을 추가하세요. 또는 그 function이 실제로 다른 `.c` file에서 사용되지 않는다면 static으로 만드세요.

### What is the interval between timer interrupts? (timer interrupt 간격은 얼마인가요?)

Timer interrupt(타이머 인터럽트)는 초당 TIMER_FREQ번 발생합니다. 이 값은 `devices/timer.h`를 수정하여 조정할 수 있습니다. 기본값은 100 Hz입니다. 이 값을 바꾸는 것은 권장하지 않습니다. 어떤 변경이든 많은 test(테스트)를 실패하게 만들 가능성이 높기 때문입니다.

### How long is a time slice? (time slice 길이는 얼마인가요?)

time slice(시간 조각) 하나에는 `TIME_SLICE` ticks(틱)가 있습니다. 이 macro(매크로)는 `threads/thread.c`에 선언되어 있습니다. 기본값은 4 ticks입니다. 이 값을 바꾸는 것은 권장하지 않습니다. 어떤 변경이든 많은 test를 실패하게 만들 가능성이 높기 때문입니다.

### How do I run the tests? (test는 어떻게 실행하나요?)

[Testing](https://casys-kaist.github.io/pintos-kaist/introduction/grading#Testing)을 참조하세요.

### Why do I get a test failure in pass()? (`pass()`에서 test failure가 나는 이유는 무엇인가요?)

아마 다음과 비슷한 backtrace(호출 스택 추적)를 보고 있을 것입니다.

    
    
    0x8004208810: debug_panic (lib/kernel/debug.c:32)
    0x800420a99f: pass (tests/threads/tests.c:93)
    0x800420bdd3: test_mlfqs_load_1 (...threads/mlfqs-load-1.c:33)
    0x800420a8cf: run_test (tests/threads/tests.c:51)
    0x8004200452: run_task (threads/init.c:283)
    0x8004200536: run_actions (threads/init.c:333)
    0x80042000bb: main (threads/init.c:137)
    

이것은 backtrace program(프로그램)의 헷갈리는 output(출력)일 뿐입니다. 실제로 `pass()`가 `debug_panic()`을 호출했다는 뜻이 아닙니다. 사실은 `fail()`이 `PANIC()` macro를 통해 `debug_panic()`을 호출한 것입니다. GCC는 `debug_panic()`이 return(반환)하지 않는다는 것을 알고 있습니다. `NO_RETURN`으로 선언되어 있기 때문입니다. 그래서 `debug_panic()`이 return할 때 control(제어)을 가져오기 위한 code를 `fail()` 안에 넣지 않습니다. 이는 stack(스택)의 return address(반환 주소)가 memory(메모리)에서 우연히 `fail()` 뒤에 오는 function의 시작점처럼 보인다는 뜻입니다. 이 경우 그 function이 우연히 `pass()`입니다. 자세한 내용은 [Backtraces](4_FAQ.md)를 참조하세요.

### How do interrupts get re-enabled in the new thread following schedule()? (`schedule()` 이후 새 thread에서 interrupt는 어떻게 다시 enable되나요?)

`schedule()`로 들어가는 모든 path(경로)는 interrupt를 disable(비활성화)합니다. interrupt는 결국 다음에 schedule되는 thread에서 다시 enable(활성화)됩니다. 가능한 경우를 생각해 봅시다. 새 thread는 `switch_thread()` 안에서 실행 중입니다(단, 아래 참고). `switch_thread()`는 `schedule()`이 호출하고, `schedule()`은 몇 가지 가능한 function 중 하나가 호출합니다.

  - `thread_exit()`이지만, 이런 thread로는 다시 switch(전환)하지 않으므로 중요하지 않습니다.
  - `thread_yield()`는 `schedule()`에서 return하자마자 interrupt level(인터럽트 레벨)을 복원합니다.
  - `thread_block()`은 여러 곳에서 호출됩니다.
    - `sema_down()`은 `schedule()`에서 return하자마자 interrupt level을 복원합니다.
    - `idle()`은 명시적인 assembly STI instruction(어셈블리 STI 명령)으로 interrupt를 enable합니다.
    - `devices/intq.c`의 `wait()`는 caller(호출자)가 interrupt를 다시 enable할 책임을 집니다.

새로 생성된 thread가 처음 실행되는 특별한 경우가 있습니다. 그런 thread는 `kernel_thread()`의 첫 동작으로 `intr_enable()`을 호출합니다. `kernel_thread()`는 첫 번째 thread를 제외한 모든 kernel thread(커널 스레드)의 call stack(호출 스택) 맨 아래에 있습니다.

### Do I need to account for timer values overflowing? (timer 값 overflow를 고려해야 하나요?)

timer value(타이머 값)가 overflow(오버플로)될 가능성은 걱정하지 않아도 됩니다. timer value는 signed 64-bit number(부호 있는 64비트 수)로 표현되며, 초당 100 ticks라면 거의 2,924,712,087년 동안 충분합니다.

### Doesn't priority scheduling lead to starvation? (priority scheduling은 starvation을 일으키지 않나요?)

맞습니다. strict priority scheduling(엄격한 우선순위 스케줄링)은 starvation(기아)을 일으킬 수 있습니다. 더 높은 priority의 runnable thread(실행 가능한 스레드)가 하나라도 있으면 어떤 thread가 실행되지 못하기 때문입니다. advanced scheduler는 thread priority를 dynamic(동적으로) 변경하는 mechanism(메커니즘)을 도입합니다. strict priority scheduling은 real-time system(실시간 시스템)에서 가치가 있습니다. programmer(프로그래머)가 어떤 job(작업)이 processing time(처리 시간)을 얻을지 더 많이 control할 수 있기 때문입니다. 높은 priority는 일반적으로 time-critical task(시간에 민감한 작업)에 예약됩니다. 이는 "fair(공정)"하지는 않지만, general-purpose operating system(범용 운영체제)에는 적용되지 않는 다른 관심사를 해결합니다.

### What thread should run after a lock has been released? (lock이 release된 뒤 어떤 thread가 실행되어야 하나요?)

lock(상호 배제 잠금)이 release(해제)되면, 그 lock을 기다리는 가장 높은 priority의 thread가 unblock(차단 해제)되어 ready thread list(준비 스레드 리스트)에 들어가야 합니다. 그런 다음 scheduler(스케줄러)는 ready list에서 가장 높은 priority의 thread를 실행해야 합니다.

### If the highest-priority thread yields, does it continue running? (가장 높은 priority thread가 yield하면 계속 실행되나요?)

네. 가장 높은 priority를 가진 thread가 하나뿐이라면, 그 thread가 `thread_yield()`를 호출하더라도 block(차단)되거나 finish(종료)될 때까지 계속 실행됩니다. 같은 highest priority를 가진 thread가 여러 개라면, `thread_yield()`는 그 thread들 사이를 "round robin(순환 방식)" 순서로 switch해야 합니다.

### What happens to the priority of a donating thread? (donating thread의 priority는 어떻게 되나요?)

Priority donation(우선순위 기부)은 donee thread(기부받는 스레드)의 priority만 변경합니다. donor thread(기부하는 스레드)의 priority는 바뀌지 않습니다. priority donation은 additive(가산적)이지 않습니다. priority 5인 thread A가 priority 3인 thread B에게 donate하면, B의 새 priority는 8이 아니라 5입니다.

### Can a thread's priority change while it is on the ready queue? (ready queue에 있는 동안 thread priority가 바뀔 수 있나요?)

네. ready 상태인 low-priority thread L이 lock을 보유하고 있다고 생각해 봅시다. high-priority thread H가 그 lock을 acquire(획득)하려고 시도하다가 block되며, 그 결과 ready thread L에게 자신의 priority를 donate합니다.

### Can a thread's priority change while it is blocked? (blocked 상태에서 thread priority가 바뀔 수 있나요?)

네. lock L을 acquire한 thread가 어떤 이유로든 blocked 상태인 동안, 더 높은 priority thread가 L을 acquire하려고 하면 priority donation으로 그 thread의 priority가 올라갈 수 있습니다. 이 경우는 priority-donate-sema test가 확인합니다.

### Can a thread added to the ready list preempt the processor? (ready list에 추가된 thread가 processor를 preempt할 수 있나요?)

네. ready list에 추가된 thread가 running thread(실행 중인 스레드)보다 높은 priority를 가진다면, 올바른 동작은 즉시 processor(프로세서)를 yield하는 것입니다. 다음 timer interrupt까지 기다리는 것은 허용되지 않습니다. 가장 높은 priority thread는 runnable(실행 가능)해지는 즉시 실행되어야 하며, 현재 실행 중인 thread가 무엇이든 preempt(선점)해야 합니다.

### How does `thread_set_priority()` affect a thread receiving donations? (`thread_set_priority()`는 donation을 받는 thread에 어떤 영향을 주나요?)

이 함수는 thread의 base priority(기본 우선순위)를 설정합니다. thread의 effective priority(유효 우선순위)는 새로 설정된 priority와 가장 높은 donated priority 중 더 높은 값이 됩니다. donation이 release되면 thread의 priority는 이 function call(함수 호출)을 통해 설정된 값이 됩니다. 이 동작은 priority-donate-lower test가 확인합니다.

### Doubled test names in output make them fail. (output에 test 이름이 중복되어 실패합니다.)

다음과 같이 어떤 test 이름이 중복된 output을 보고 있다고 합시다.

    
    
    (alarm-priority) begin
    (alarm-priority) (alarm-priority) Thread priority 30 woke up.
    Thread priority 29 woke up.
    (alarm-priority) Thread priority 28 woke up.
    

일어나고 있는 일은 두 thread의 output이 interleave(서로 섞임)되고 있다는 것입니다. 즉, 한 thread가 `(alarm-priority) Thread priority 29 woke up.\n`을 print(출력)하고 다른 thread가 `(alarm-priority) Thread priority 30 woke up.\n`을 print하는데, 첫 번째 thread가 output 중간에서 두 번째 thread에게 preempt되고 있습니다. 이 문제는 priority scheduler에 bug가 있음을 나타냅니다. priority 29인 thread는 priority 30인 thread가 할 일이 있는 동안 실행될 수 없어야 하기 때문입니다. 일반적으로 Pintos kernel의 `printf()` function 구현은 printf call이 진행되는 동안 console lock(콘솔 잠금)을 acquire하고 이후 release하여 이런 interleaved output을 방지하려고 합니다. 하지만 test name, 예를 들어 `(alarm-priority)`, 그리고 그 뒤의 message는 printf 두 번을 사용해 출력되므로, console lock이 두 번 acquire되고 release됩니다.

### How does priority donation interact with the advanced scheduler? (priority donation은 advanced scheduler와 어떻게 상호작용하나요?)

상호작용할 필요가 없습니다. priority donation과 advanced scheduler를 동시에 test하지 않을 것입니다.

### Can I use one queue instead of 64 queues? (64개 queue 대신 하나의 queue를 사용해도 되나요?)

네. 일반적으로 여러분의 구현은 설명과 달라도 됩니다. 동작이 같기만 하면 됩니다.

### Some scheduler tests fail and I don't understand why. Help! (일부 scheduler test가 실패하는데 이유를 모르겠습니다. 도와주세요!)

advanced scheduler test 중 일부가 알 수 없이 실패한다면 다음을 시도해 보세요.

  - 실패하는 test의 source file을 읽어 무엇이 일어나고 있는지 이해했는지 확인하세요. 각 file 맨 위에는 목적과 예상 결과를 설명하는 comment(주석)가 있습니다.
  - fixed-point arithmetic routine(고정소수점 연산 루틴)과 scheduler routine에서 그 routine을 사용하는 방식을 다시 확인하세요.
  - timer interrupt에서 여러분의 구현이 얼마나 많은 work(작업)를 하는지 생각해 보세요. timer interrupt handler(타이머 인터럽트 처리 함수)가 너무 오래 걸리면, 그 interrupt가 preempt한 thread에서 timer tick 대부분을 빼앗게 됩니다. control이 그 thread로 돌아왔을 때 다음 timer interrupt가 도착하기 전에 실제로 할 수 있는 work가 거의 없습니다. 그러면 그 thread가 실제로 사용할 기회가 있었던 것보다 훨씬 많은 CPU time을 사용한 것으로 blame(계산)됩니다. 이는 interrupted thread(인터럽트된 스레드)의 recent CPU count(최근 CPU 카운트)를 높이고, priority를 낮춥니다. scheduling decision(스케줄링 결정)을 바꿀 수 있으며 load average(부하 평균)도 높입니다.
