# Threads (스레드)

## `struct thread`

thread(실행 흐름)를 위한 Pintos의 주요 data structure(자료구조)는 `threads/thread.h`에 선언된 `struct thread`입니다.

* * *
    
    
    struct thread;
    

thread 또는 user process(사용자 프로세스)를 나타냅니다. project(프로젝트)를 진행하면서 `struct thread`에 여러분만의 member(멤버)를 추가해야 합니다. 기존 member의 definition(정의)을 변경하거나 삭제할 수도 있습니다. 모든 `struct thread`는 자신만의 memory page(메모리 페이지) 시작 부분을 차지합니다. page의 나머지는 thread의 stack(스택)으로 사용되며, page 끝에서 아래쪽으로 grow(성장)합니다. 모양은 다음과 같습니다.
    
    
                          4 kB +---------------------------------+
                               |         kernel stack            |
                               |               |                 |
                               |               |                 |
                               |               V                 |
                               |        grows downward           |
                               |                                 |
                               |                                 |
                               |                                 |
                               |                                 |
                               |                                 |
                               |                                 |
                               |                                 |
                               |                                 |
        sizeof (struct thread) +---------------------------------+
                               |             magic               |
                               |          intr_frame             |
                               |               :                 |
                               |               :                 |
                               |             status              |
                               |              tid                |
                          0 kB +---------------------------------+
    

이에는 두 가지 consequence(결과)가 있습니다. 첫째, `struct thread`가 너무 커지면 안 됩니다. 너무 커지면 kernel stack(커널 스택)을 위한 room(공간)이 충분하지 않습니다. base `struct thread`는 크기가 몇 byte(바이트)에 불과합니다. 아마 1 kB보다 훨씬 작게 유지해야 합니다. 둘째, kernel stack이 너무 크게 grow하면 안 됩니다. stack overflow(스택 넘침)가 발생하면 thread state(스레드 상태)를 corrupt(손상)시킵니다. 따라서 kernel function(커널 함수)은 큰 structure나 array(배열)를 non-static local variable(비정적 지역 변수)로 allocate(할당)해서는 안 됩니다. 대신 `malloc()`이나 `palloc_get_page()`를 사용하는 dynamic allocation(동적 할당)을 사용하세요([Memory Allocation](https://casys-kaist.github.io/pintos-kaist/appendix/memoryAllocation.md) 참조).

* * *
    
    
    tid_t tid;
    

> thread의 thread identifier(스레드 식별자), 즉 tid입니다. 모든 thread는 kernel(운영체제 핵심부)의 전체 lifetime(수명) 동안 unique(고유)한 tid를 가져야 합니다. 기본적으로 `tid_t`는 `int`의 `typedef`이며, 각 새 thread는 initial process(초기 프로세스)의 1부터 시작하여 숫자상 다음으로 큰 tid를 받습니다. 원한다면 type(타입)과 numbering scheme(번호 매김 방식)을 바꿀 수 있습니다.

* * *
    
    
    enum thread_status status;
    

> thread의 state(상태)이며 다음 중 하나입니다.
> 
>   - `THREAD_RUNNING`
> 
> thread가 running(실행 중)입니다. 어떤 시점에도 정확히 하나의 thread만 running입니다. `thread_current()`는 running thread를 반환합니다.
> 
>   - `THREAD_READY`
> 
> thread가 run할 준비가 되었지만 지금은 running이 아닙니다. scheduler(스케줄러)가 다음에 invoke(호출)될 때 이 thread가 선택될 수 있습니다. ready thread는 `ready_list`라는 doubly linked list(이중 연결 리스트)에 보관됩니다.
> 
>   - `THREAD_BLOCKED`
> 
> thread가 무언가를 기다리고 있습니다. 예를 들어 lock(잠금)이 available(사용 가능)해지거나 interrupt(인터럽트)가 invoke되기를 기다립니다. `thread_unblock()` 호출로 `THREAD_READY` state로 transition(전이)하기 전까지 thread는 다시 schedule(실행 선택)되지 않습니다. 이는 thread를 자동으로 block/unblock하는 Pintos synchronization primitive(동기화 기본 도구) 중 하나를 사용하여 간접적으로 하는 것이 가장 편리합니다([Synchronization](1_synchronization.md) 참조). blocked thread가 무엇을 기다리는지 *a priori*(사전에) 알 방법은 없지만, backtrace(호출 스택 추적)가 도움이 될 수 있습니다([Backtraces](0_threads.md) 참조).
> 
>   - `THREAD_DYING`
> 
> 다음 thread로 switch(전환)한 뒤 scheduler가 이 thread를 destroy(파괴)합니다.

* * *
    
    
    char name[16];
    

> string(문자열)으로 된 thread name(스레드 이름), 또는 최소한 그 앞 몇 character(문자)입니다.

* * *
    
    
    struct intr_frame tf;
    

> context switching(문맥 전환)을 위한 information(정보)을 저장합니다. register(레지스터)와 stack pointer(스택 포인터)를 포함합니다.

* * *
    
    
    int priority;
    

> `PRI_MIN`(0)부터 `PRI_MAX`(63)까지의 thread priority(우선순위)입니다. 낮은 숫자가 낮은 priority에 대응하므로 priority 0이 가장 낮고 priority 63이 가장 높습니다. 제공된 Pintos는 thread priority를 무시하지만, project 1에서 priority scheduling(우선순위 스케줄링)을 구현하게 됩니다([Priority Scheduling](../1_project1/2_priority_scheduling.md) 참조).

* * *
    
    
    struct list_elem elem;
    

> thread를 doubly linked list에 넣기 위한 "list element(리스트 요소)"입니다. 이는 `ready_list`(실행 준비 thread list) 또는 `sema_down()`에서 semaphore(세마포어)를 기다리는 thread list일 수 있습니다. semaphore를 기다리는 thread는 ready가 아니고 그 반대도 마찬가지이므로 두 용도로 사용할 수 있습니다.

* * *
    
    
    uint64_t *pml4;
    

> project 2 이후에만 존재합니다. [Page Tables](3_virtual_address.md)를 참조하세요.

* * *
    
    
    unsigned magic
    

> 항상 `THREAD_MAGIC`으로 설정됩니다. 이는 `threads/thread.c`에 정의된 임의의 number(숫자)이며 stack overflow를 detect(감지)하는 데 사용됩니다. `thread_current()`는 running thread의 `struct thread`에서 `magic` member가 `THREAD_MAGIC`으로 설정되어 있는지 check(확인)합니다. stack overflow는 이 값을 바꾸는 경향이 있어 assertion(단언)을 trigger(유발)합니다. 최대 효과를 위해 `struct thread`에 member를 추가할 때 `magic`을 끝에 두세요.

## Thread Functions (스레드 함수)

`threads/thread.c`는 thread support(스레드 지원)를 위한 여러 public function(공개 함수)을 구현합니다. 가장 유용한 것들을 살펴봅시다.

* * *
    
    
    void thread_init (void);
    

> `main()`이 thread system(스레드 시스템)을 initialize(초기화)하기 위해 호출합니다. 주요 목적은 Pintos의 initial thread를 위한 `struct thread`를 만드는 것입니다. Pintos loader(로더)가 initial thread의 stack을 다른 Pintos thread와 같은 위치인 page top(페이지 맨 위)에 두기 때문에 가능합니다.
> 
> `thread_init()`이 실행되기 전에는 running thread의 `magic` 값이 올바르지 않으므로 `thread_current()`가 fail(실패)합니다. `lock_acquire()`를 포함한 많은 function이 직접 또는 간접적으로 `thread_current()`를 호출하므로, Pintos initialization 초기에 `thread_init()`이 호출됩니다.

* * *
    
    
    void thread_start (void);
    

> `main()`이 scheduler를 start(시작)하기 위해 호출합니다. 다른 thread가 ready가 아닐 때 schedule되는 thread인 idle thread(유휴 스레드)를 create(생성)합니다. 그런 다음 interrupt를 enable(활성화)합니다. 이는 부수 효과로 scheduler를 enable합니다. scheduler는 timer interrupt(타이머 인터럽트)에서 return할 때 `intr_yield_on_return()`을 사용하여 실행되기 때문입니다.

* * *
    
    
    void thread_tick (void);
    

> 각 timer tick(타이머 틱)마다 timer interrupt가 호출합니다. thread statistic(통계)을 추적하고 time slice(시간 조각)가 expire(만료)되면 scheduler를 trigger합니다.

* * *
    
    
    void thread_print_stats (void);
    

> Pintos shutdown(종료) 중 thread statistic을 print(출력)하기 위해 호출됩니다.

* * *
    
    
    tid_t thread_create (const char *name, int priority, thread func *func, void *aux);
    

> 주어진 priority로 name이라는 새 thread를 create하고 start하며, 새 thread의 tid를 반환합니다. thread는 func를 실행하고, aux를 function의 단일 argument(인자)로 전달합니다.
> 
> `thread_create()`는 thread의 `struct thread`와 stack을 위한 page를 allocate하고 member를 initialize한 뒤, fake stack frame(가짜 스택 프레임) 집합을 setup합니다. thread는 blocked state로 initialize된 뒤 return 직전에 unblock되며, 이로 인해 새 thread가 schedule될 수 있습니다.

* * *
    
    
    void thread_func (void *aux);
    

> `thread_create()`에 전달되는 function의 type입니다. aux argument는 function의 argument로 그대로 전달됩니다.

* * *
    
    
    void thread_block (void);
    

> running thread를 running state에서 blocked state로 transition합니다. 이 thread는 `thread_unblock()`이 호출되기 전까지 다시 run하지 않으므로, 그렇게 될 방법을 마련해 두어야 합니다. `thread_block()`은 매우 low-level(저수준)이므로, 대신 synchronization primitive 중 하나를 사용하는 편이 좋습니다([Synchronization](1_synchronization.md) 참조).

* * *
    
    
    void thread_unblock (struct thread *thread);
    

> blocked state에 있어야 하는 thread를 ready state로 transition하여 running을 resume(재개)할 수 있게 합니다. thread가 기다리는 event(이벤트)가 발생할 때 호출됩니다. 예를 들어 thread가 기다리던 lock이 available해질 때입니다.

* * *
    
    
    struct thread *thread_current (void);
    

> running thread를 반환합니다.

* * *
    
    
    tid_t thread_tid (void);
    

> running thread의 thread id를 반환합니다. `thread_current ()->tid`와 equivalent(동등)합니다.

* * *
    
    
    const char *thread_name (void);
    

> running thread의 name을 반환합니다. `thread_current ()->name`과 equivalent합니다.

* * *
    
    
    void thread_exit (void) NO_RETURN;
    

> current thread(현재 스레드)를 exit하게 합니다. 절대 return하지 않습니다.

* * *
    
    
    void thread_yield (void);
    

> CPU를 scheduler에게 yield(양보)합니다. scheduler는 실행할 새 thread를 고릅니다. 새 thread가 current thread일 수도 있으므로, 이 function이 이 thread를 특정 시간 동안 실행되지 않게 유지한다고 의존할 수는 없습니다.

* * *
    
    
    int thread_get_priority (void);
    void thread_set_priority (int new_priority);
    

> thread priority를 set/get(설정/조회)하기 위한 stub(스텁)입니다. [Priority Scheduling](https://casys-kaist.github.io/pintos-kaist/project1/priority_scheduling)을 참조하세요.

* * *
    
    
    int thread_get_nice (void);
    void thread_set_nice (int new_nice);
    int thread_get_recent_cpu (void);
    int thread_get_load_avg (void);
    

> advanced scheduler(고급 스케줄러)를 위한 stub입니다.
