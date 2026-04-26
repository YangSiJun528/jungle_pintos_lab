# Synchronization (동기화)

thread(실행 흐름) 사이의 resource(자원) sharing(공유)을 조심스럽고 통제된 방식으로 처리하지 않으면 보통 큰 혼란이 생깁니다. operating system kernel(운영체제 핵심부)에서는 잘못된 sharing이 machine(컴퓨터) 전체를 crash(충돌)시킬 수 있으므로 특히 그렇습니다. Pintos는 이를 돕기 위해 여러 synchronization primitive(동기화 기본 도구)를 제공합니다.

## Disabling Interrupts (인터럽트 비활성화)

synchronization을 수행하는 가장 거친 방법은 interrupt(현재 실행을 멈추고 처리하는 이벤트)를 disable(비활성화)하는 것입니다. 즉 CPU가 interrupt에 respond(응답)하지 못하도록 일시적으로 막는 것입니다. interrupt가 off(꺼짐) 상태이면 다른 thread가 running thread(실행 중인 스레드)를 preempt(선점)하지 않습니다. thread preemption(스레드 선점)은 timer interrupt(타이머 인터럽트)에 의해 구동되기 때문입니다. interrupt가 보통처럼 on(켜짐) 상태이면, running thread는 두 C statement(문장) 사이든 하나의 statement 실행 중이든 언제든 다른 thread에게 preempt될 수 있습니다.

덧붙이면 이는 Pintos가 "preemptible kernel(선점 가능한 커널)"이라는 뜻입니다. 즉 kernel thread가 언제든 preempt될 수 있습니다. 전통적인 Unix system은 "nonpreemptible(비선점)"입니다. 즉 kernel thread는 scheduler(스케줄러)를 명시적으로 호출하는 지점에서만 preempt될 수 있습니다. user program(사용자 프로그램)은 두 model(모델) 모두에서 언제든 preempt될 수 있습니다. 상상할 수 있듯 preemptible kernel은 더 명시적인 synchronization을 필요로 합니다.

interrupt state(인터럽트 상태)를 직접 설정할 일은 거의 없어야 합니다. 대부분의 경우 다음 section(절)에서 설명하는 다른 synchronization primitive를 사용해야 합니다. interrupt를 disable하는 주된 이유는 kernel thread와 external interrupt handler(외부 인터럽트 처리기)를 synchronize하기 위해서입니다. external interrupt handler는 sleep(잠들기)할 수 없으므로 대부분의 다른 synchronization form(형태)을 사용할 수 없습니다.

어떤 external interrupt는 interrupt를 disable해도 postpone(연기)할 수 없습니다. 이런 interrupt를 **non-maskable interrupt**(NMI, 마스크 불가능 인터럽트)라고 하며, 예를 들어 computer에 불이 난 경우처럼 emergency(비상 상황)에만 사용되어야 합니다. Pintos는 non-maskable interrupt를 handle(처리)하지 않습니다.

interrupt disable/enable(비활성화/활성화)을 위한 type(타입)과 function(함수)은 `include/threads/interrupt.h`에 있습니다.

* * *
    
    
    enum intr_level;
    

> INTR_OFF 또는 INTR_ON 중 하나로, interrupt가 각각 disabled 또는 enabled 상태임을 나타냅니다.

* * *
    
    
    enum intr_level intr_get_level (void)
    

> 현재 interrupt state를 반환합니다.

* * *
    
    
    enum intr_level intr_set_level (enum intr_level level);
    

> level에 따라 interrupt를 켜거나 끕니다. 이전 interrupt state를 반환합니다.

* * *
    
    
    enum intr_level intr_enable (void);
    

> interrupt를 켭니다. 이전 interrupt state를 반환합니다.

* * *
    
    
    enum intr_level intr_disable (void);
    

> interrupt를 끕니다. 이전 interrupt state를 반환합니다.

## Semaphores (세마포어)

semaphore(세마포어)는 nonnegative integer(음이 아닌 정수)와 그것을 atomic(원자적)으로 조작하는 두 operator(연산)로 이루어집니다.

  - "Down" 또는 "P": 값이 positive(양수)가 될 때까지 기다린 뒤 값을 감소시킵니다.
  - "UP" 또는 "V": 값을 증가시킵니다. 기다리는 thread가 있으면 하나를 깨웁니다.

0으로 initialize(초기화)된 semaphore는 정확히 한 번 발생할 event(이벤트)를 기다리는 데 사용할 수 있습니다. 예를 들어 thread A가 다른 thread B를 시작하고, B가 어떤 activity(작업)가 완료되었음을 signal(신호)할 때까지 기다리고 싶다고 합시다. A는 0으로 initialize된 semaphore를 만들고, B를 시작할 때 그것을 전달한 뒤 semaphore를 "down"할 수 있습니다. B가 activity를 끝내면 semaphore를 "up"합니다. 이는 A가 먼저 semaphore를 "down"하든 B가 먼저 "up"하든 관계없이 동작합니다.

예를 들면 다음과 같습니다.
    
    
    struct semaphore sema;
    
    /* Thread A */
    void threadA (void) {
        sema_down (&sema);
    }
    
    /* Thread B */
    void threadB (void) {
        sema_up (&sema);
    }
    
    /* main function */
    void main (void) {
        sema_init (&sema, 0);
        thread_create ("threadA", PRI_MIN, threadA, NULL);
        thread_create ("threadB", PRI_MIN, threadB, NULL);
    }
    

이 예에서 threadA는 threadB가 `sema_up ()`을 호출할 때까지 `sema_down ()`에서 execution(실행)을 멈춥니다.

1로 initialize된 semaphore는 보통 resource access(자원 접근)를 control(제어)하는 데 사용됩니다. code block(코드 블록)이 resource를 사용하기 전에 semaphore를 "down"하고, resource 사용을 마친 뒤 resource를 "up"합니다. 이런 경우에는 아래 설명하는 lock(잠금)이 더 적절할 수 있습니다.

semaphore는 1보다 큰 값으로 initialize될 수도 있습니다. 이는 드물게 사용됩니다. semaphore는 Edsger Dijkstra가 발명했고 THE operating system에서 처음 사용되었습니다. Pintos의 semaphore type과 operation은 `include/threads/synch.h`에 선언되어 있습니다.

* * *
    
    
    struct semaphore;
    

> semaphore를 나타냅니다.

* * *
    
    
    void sema_init (struct semaphore *sema, unsigned value);
    

> 주어진 initial value(초기값)로 sema를 새 semaphore로 initialize합니다.

* * *
    
    
    void sema_down (struct semaphore *sema);
    

sema에 대해 "down" 또는 "P" operation을 실행합니다. 값이 positive가 될 때까지 기다렸다가 1 감소시킵니다.

* * *
    
    
    bool sema_try_down (struct semaphore *sema);
    

> 기다리지 않고 sema에 대해 "down" 또는 "P" operation을 실행하려고 시도합니다. sema를 성공적으로 감소시켰으면 true를 반환하고, 이미 zero(0)이어서 기다리지 않고는 감소시킬 수 없으면 false를 반환합니다. 이 function을 tight loop(촘촘한 반복문)에서 호출하면 CPU time(프로세서 시간)을 낭비하므로, 대신 `sema_down()`을 사용하거나 다른 approach(접근)를 찾으세요.

* * *
    
    
    void sema_up (struct semaphore *sema);
    

> sema에 대해 "up" 또는 "V" operation을 실행하여 값을 증가시킵니다. sema를 기다리는 thread가 있으면 그중 하나를 깨웁니다. 대부분의 synchronization primitive와 달리 `sema_up()`은 external interrupt handler 안에서 호출될 수 있습니다.

semaphore는 내부적으로 interrupt disable([Disabling Interrupts](#Disabling%20Interrupts) 참조)과 thread blocking/unblocking(`thread_block()`과 `thread_unblock()`)으로 만들어집니다. 각 semaphore는 `lib/kernel/list.c`의 linked list(연결 리스트) implementation을 사용하여 waiting thread list(대기 스레드 리스트)를 유지합니다.

## Locks (잠금)

lock은 initial value(초기값)가 1인 semaphore와 비슷합니다([Semaphores](#Semaphores) 참조). lock에서 "up"에 해당하는 operation은 "release(해제)"라고 하고, "down" operation은 "acquire(획득)"라고 합니다.

semaphore와 비교하면 lock에는 추가 restriction(제한)이 하나 있습니다. lock을 acquire한 thread, 즉 lock의 "owner(소유자)"만 그 lock을 release할 수 있습니다. 이 restriction이 문제가 된다면 lock 대신 semaphore를 사용해야 한다는 좋은 sign(징후)입니다.

Pintos의 lock은 "recursive(재귀적)"이지 않습니다. 즉 현재 lock을 보유한 thread가 그 lock을 다시 acquire하려고 하면 error(오류)입니다. lock type과 function은 `include/threads/synch.h`에 선언되어 있습니다.

* * *
    
    
    struct lock;
    

> lock을 나타냅니다.

* * *
    
    
    void lock_init (struct lock *lock);
    

> lock을 새 lock으로 initialize합니다. 초기에는 어떤 thread도 lock을 own(소유)하지 않습니다.

* * *
    
    
    void lock_acquire (struct lock *lock);
    

> current thread(현재 스레드)를 위해 lock을 acquire합니다. 필요하면 현재 owner가 release할 때까지 먼저 기다립니다.

* * *
    
    
    bool lock_try_acquire (struct lock *lock);
    

> 기다리지 않고 current thread가 사용할 lock을 acquire하려고 시도합니다. 성공하면 true, lock이 이미 owned 상태이면 false를 반환합니다. 이 function을 tight loop에서 호출하는 것은 CPU time을 낭비하므로 좋지 않습니다. 대신 `lock_acquire()`를 사용하세요.

* * *
    
    
    void lock_release (struct lock *lock);
    

> current thread가 own해야 하는 lock을 release합니다.

* * *
    
    
    bool lock_held_by_current_thread (const struct lock *lock):
    

> running thread가 lock을 own하면 true, 그렇지 않으면 false를 반환합니다. 임의의 thread가 lock을 own하는지 test하는 function은 없습니다. caller(호출자)가 그 답을 바탕으로 행동하기 전에 답이 바뀔 수 있기 때문입니다.

## Monitors (모니터)

monitor(모니터)는 semaphore나 lock보다 higher-level(상위 수준)의 synchronization form입니다. monitor는 synchronize되는 data(데이터), monitor lock이라고 부르는 lock, 그리고 하나 이상의 condition variable(조건 변수)로 구성됩니다. thread는 protected data(보호된 데이터)에 접근하기 전에 먼저 monitor lock을 acquire합니다. 그러면 그 thread는 "in the monitor(모니터 안에 있다)"고 말합니다. monitor 안에 있는 동안 thread는 모든 protected data를 control하며 자유롭게 examine(검사)하거나 modify(수정)할 수 있습니다. protected data 접근이 끝나면 monitor lock을 release합니다.

condition variable은 monitor 안의 code가 어떤 condition(조건)이 true가 되기를 wait(대기)할 수 있게 합니다. 각 condition variable은 abstract condition(추상 조건), 예를 들어 "처리할 data가 도착했다" 또는 "user의 마지막 keystroke(키 입력) 이후 10초가 지났다"와 associated(연관)됩니다. monitor 안의 code가 condition이 true가 되기를 기다려야 하면, associated condition variable에서 "wait"합니다. 이는 lock을 release하고 condition이 signaled(신호됨)될 때까지 기다립니다. 반대로 code가 이런 condition 중 하나를 true로 만들었다면, condition에 "signal"하여 하나의 waiter(대기자)를 깨우거나 "broadcast"하여 모두를 깨웁니다.

monitor의 theoretical framework(이론적 틀)는 C.A.R. Hoare가 제시했습니다. practical usage(실제 사용법)는 이후 Mesa operating system에 대한 논문에서 더 자세히 설명되었습니다. condition variable type과 function은 `include/threads/synch.h`에 선언되어 있습니다.

* * *
    
    
    struct condition;
    

> condition variable을 나타냅니다.

* * *
    
    
    void cond_init (struct condition *cond);
    

> cond를 새 condition variable로 initialize합니다.

* * *
    
    
    void cond_wait (struct condition *cond, struct lock *lock);
    

> lock(monitor lock)을 atomically(원자적으로) release하고, 다른 code 조각이 cond를 signal할 때까지 wait합니다. cond가 signal된 뒤 return하기 전에 lock을 다시 acquire합니다. 이 function을 호출하기 전에 lock이 held(보유)되어 있어야 합니다. signal을 보내는 것과 wait에서 깨어나는 것은 atomic operation이 아닙니다. 따라서 일반적으로 `cond_wait()`의 caller는 wait가 완료된 뒤 condition을 다시 check하고, 필요하다면 다시 wait해야 합니다. 예시는 다음 section을 참조하세요.

* * *
    
    
    void cond_signal (struct condition *cond, struct lock *lock);
    

> cond를 wait하는 thread가 있다면(monitor lock lock으로 보호됨) 그중 하나를 깨웁니다. waiting thread가 없으면 아무 action(동작) 없이 return합니다. 이 function을 호출하기 전에 lock이 held되어 있어야 합니다.

* * *
    
    
    void cond_broadcast (struct condition *cond, struct lock *lock);
    

> cond를 wait하는 모든 thread가 있다면 모두 깨웁니다(monitor lock lock으로 보호됨). 이 function을 호출하기 전에 lock이 held되어 있어야 합니다.

### Monitor Example (모니터 예시)

monitor의 classical example(고전적 예)는 하나 이상의 "producer(생산자)" thread가 character(문자)를 write하고, 하나 이상의 "consumer(소비자)" thread가 character를 read하는 buffer(버퍼)를 처리하는 것입니다. 이를 구현하려면 monitor lock 외에 `not_full`과 `not_empty`라고 부를 두 condition variable이 필요합니다.
    
    
        char buf[BUF_SIZE];     /* Buffer. */
        size_t n = 0;         /* 0 <= n <= BUF SIZE: # of characters in buffer. */
        size_t head = 0;        /* buf index of next char to write (mod BUF SIZE). */
        size_t tail = 0;         /* buf index of next char to read (mod BUF SIZE). */
        struct lock lock;         /* Monitor lock. */
        struct condition not_empty; /* Signaled when the buffer is not empty. */
        struct condition not_full;     /* Signaled when the buffer is not full. */
    
        ...initialize the locks and condition variables...
    
        void put (char ch) {
          lock_acquire (&lock);
          while (n == BUF_SIZE)    /* Can't add to buf as long as it's full. */
            cond_wait (&not_full, &lock);
          buf[head++ % BUF_SIZE] = ch;    /* Add ch to buf. */
          n++;
          cond_signal (&not_empty, &lock);    /* buf can't be empty anymore. */
          lock_release (&lock);
        }
    
        char get (void) {
          char ch;
          lock_acquire (&lock);
          while (n == 0)        /* Can't read buf as long as it's empty. */
            cond_wait (&not_empty, &lock);
          ch = buf[tail++ % BUF_SIZE];    /* Get ch from buf. */
          n--;
          cond_signal (&not_full, &lock);    /* buf can't be full anymore. */
          lock_release (&lock);
        }
    

위 code가 완전히 correct(정확)하려면 `BUF_SIZE`가 `SIZE_MAX + 1`을 evenly divide(나누어떨어지게)해야 합니다. 그렇지 않으면 `head`가 처음 0으로 wrap around(되감김)될 때 fail합니다. 실제로 `BUF_SIZE`는 보통 power of 2(2의 거듭제곱)입니다.

## Optimization Barriers (최적화 장벽)

optimization barrier(최적화 장벽)는 compiler(컴파일러)가 barrier를 가로지르는 memory state(메모리 상태)에 대해 assumption(가정)을 하지 못하게 하는 special statement(특수 문장)입니다. compiler는 barrier를 사이에 두고 variable(변수)의 read/write(읽기/쓰기)를 reorder(재정렬)하지 않으며, address가 절대 taken(사용)되지 않는 local variable(지역 변수)을 제외하고 variable value(값)가 barrier를 지나도 unchanged(변경되지 않음)라고 assume하지 않습니다. Pintos에서는 `include/threads/synch.h`가 `barrier()` macro를 optimization barrier로 정의합니다.

optimization barrier를 사용하는 한 가지 이유는 compiler가 모르는 사이 data가 asynchronously(비동기적으로) 변경될 수 있을 때입니다. 예를 들어 다른 thread나 interrupt handler가 변경할 수 있습니다. `devices/timer.c`의 `too_many_loops()` function이 예입니다. 이 function은 timer tick이 발생할 때까지 loop에서 busy-waiting(바쁜 대기)하며 시작합니다.
    
    
        /* Wait for a timer tick. */
        int64_t start = ticks;
        while (ticks == start)
            barrier();
    

loop 안에 optimization barrier가 없으면 compiler는 loop가 절대 terminate(종료)하지 않는다고 결론 내릴 수 있습니다. `start`와 `ticks`가 처음에 같고 loop 자체가 이들을 변경하지 않기 때문입니다. 그러면 compiler가 function을 infinite loop(무한 루프)로 "optimize(최적화)"할 수 있는데, 이는 분명히 바람직하지 않습니다.

optimization barrier는 다른 compiler optimization을 피하는 데도 사용할 수 있습니다. 역시 `devices/timer.c`에 있는 `busy_wait()` function이 예입니다. 이 function에는 다음 loop가 있습니다.
    
    
        while (loops-- > 0)
          barrier ();
    

이 loop의 goal(목표)은 loops를 원래 값에서 0까지 count down(감소)하며 busy-wait하는 것입니다. barrier가 없으면 compiler는 loop를 완전히 delete(삭제)할 수 있습니다. loop가 유용한 output(출력)을 만들지 않고 side effect(부작용)도 없기 때문입니다. barrier는 loop body(본문)가 important effect(중요한 효과)를 갖는 척하도록 compiler에게 강제합니다.

마지막으로 optimization barrier는 memory read/write의 ordering(순서)을 강제하는 데 사용할 수 있습니다. 예를 들어 timer interrupt가 발생할 때마다 global variable(전역 변수) `timer_put_char`의 character를 console(콘솔)에 print하되, global Boolean variable `timer_do_put`이 true일 때만 print하는 "feature(기능)"를 추가한다고 합시다. 그러면 `x`가 print되도록 setup하는 가장 좋은 방법은 다음처럼 optimization barrier를 사용하는 것입니다.
    
    
        timer_put_char = 'x';
        barrier ();
        timer_do_put = true;
    

barrier가 없으면 code는 buggy(버그가 있음)입니다. compiler가 같은 order를 유지할 이유를 보지 못하면 operation을 자유롭게 reorder할 수 있기 때문입니다. 이 경우 compiler는 assignment(대입)의 order가 중요하다는 것을 모르므로 optimizer(최적화기)가 둘의 order를 바꾸는 것이 허용됩니다. 실제로 그렇게 할지는 알 수 없고, compiler에 다른 optimization flag(최적화 플래그)를 넘기거나 다른 compiler version(버전)을 사용하면 다른 behavior가 나올 수도 있습니다.

다른 solution은 assignment 주변에서 interrupt를 disable하는 것입니다. 이는 reordering을 막지는 않지만, interrupt handler가 assignment 사이에 intervene(끼어들기)하는 것을 막습니다. 또한 interrupt를 disable하고 다시 enable하는 추가 runtime cost(실행 시간 비용)가 있습니다.
    
    
        enum intr_level old_level = intr_disable ();
        timer_put_char = 'x';
        timer_do_put = true;
        intr_set_level (old_level);
    

두 번째 solution은 `timer_put_char`와 `timer_do_put`의 declaration(선언)을 `volatile`로 mark(표시)하는 것입니다. 이 keyword(키워드)는 compiler에게 variable이 externally observable(외부에서 관찰 가능)하다고 알려 주고 optimization 자유도를 제한합니다. 하지만 `volatile`의 semantics(의미)는 잘 정의되어 있지 않으므로 좋은 general solution(범용 해결책)은 아닙니다. base Pintos code는 `volatile`을 전혀 사용하지 않습니다.

다음은 solution이 아닙니다. lock은 interrupt를 막지도 않고, lock이 held된 region(영역) 안에서 compiler가 code를 reorder하는 것도 막지 않기 때문입니다.
    
    
        lock_acquire (&timer_lock);        /* INCORRECT CODE */
        timer_put_char = 'x';
        timer_do_put = true;
        lock_release (&timer_lock);
    

compiler는 externally(다른 source file에) defined된 function invocation(호출)을 제한적인 optimization barrier로 취급합니다. 구체적으로 compiler는 외부 정의 function이 statically/dynamically allocated data(정적/동적 할당 데이터)와 address가 taken된 local variable에 접근할 수 있다고 assume합니다. 이는 명시적인 barrier를 생략할 수 있는 경우가 많다는 뜻입니다. Pintos에 명시적 barrier가 적은 이유 중 하나입니다.

같은 source file에 정의된 function이나 source file이 include(포함)한 header(헤더)에 정의된 function은 optimization barrier로 rely(의존)할 수 없습니다. function 정의 전에 invocation하는 경우에도 마찬가지입니다. compiler는 optimization을 수행하기 전에 전체 source file을 read(읽기)하고 parse(파싱)할 수 있기 때문입니다.
