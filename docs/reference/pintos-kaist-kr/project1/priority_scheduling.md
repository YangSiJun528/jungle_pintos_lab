### Priority Scheduling (우선순위 스케줄링)

**Pintos에서 priority scheduling(우선순위 기반 실행 선택)과 priority donation(우선순위 기부)을 구현하세요.**

현재 실행 중인 thread(실행 흐름)보다 높은 priority(우선순위)를 가진 thread가 ready list(준비 리스트)에 추가되면, 현재 thread는 즉시 processor(프로세서)를 새 thread에게 양보해야 합니다. 마찬가지로 thread들이 lock(상호 배제 잠금), semaphore(신호 기반 동기화 객체), condition variable(조건 변수)을 기다릴 때에는 가장 높은 priority를 가진 대기 thread가 먼저 깨어나야 합니다. thread는 언제든 자신의 priority를 올리거나 낮출 수 있지만, priority를 낮춘 결과 더 이상 가장 높은 priority가 아니라면 즉시 CPU를 양보해야 합니다.

Thread priority의 범위는 `PRI_MIN (0)`부터 `PRI_MAX (63)`까지입니다. 낮은 숫자가 낮은 priority에 대응하므로 priority 0이 가장 낮고 priority 63이 가장 높습니다. 초기 thread priority는 `thread_create()`의 argument(인자)로 전달됩니다. 다른 priority를 선택할 이유가 없다면 `PRI_DEFAULT (31)`을 사용하세요. `PRI_` macro(매크로)는 `threads/thread.h`에 정의되어 있으며, 그 값을 변경해서는 안 됩니다.

Priority scheduling의 한 가지 문제는 "priority inversion(우선순위 역전)"입니다. 각각 high, medium, low priority thread인 *H*, *M*, *L*을 생각해 봅시다. *H*가 *L*을 기다려야 하고(예를 들어 *L*이 보유한 lock을 기다리는 경우), *M*이 ready list에 있다면, low priority thread가 CPU time(프로세서 시간)을 전혀 얻지 못하므로 *H*는 CPU를 얻을 수 없습니다. 이 문제에 대한 부분적인 해결책은 *L*이 lock을 보유하는 동안 *H*가 자신의 priority를 *L*에게 "donate(기부)"하고, *L*이 lock을 release(해제)하여 *H*가 lock을 acquire(획득)하면 그 donation(기부)을 회수하는 것입니다.

Priority donation을 구현하세요. priority donation이 필요한 여러 상황을 모두 고려해야 합니다. 여러 priority가 하나의 thread에 donate되는 multiple donation(다중 기부)을 반드시 처리하세요. 또한 nested donation(중첩 기부)도 처리해야 합니다. *H*가 *M*이 보유한 lock을 기다리고, *M*이 *L*이 보유한 lock을 기다리고 있다면, *M*과 *L* 모두 *H*의 priority로 높아져야 합니다. 필요하다면 nested priority donation의 depth(깊이)에 8 level(단계) 같은 합리적인 제한을 둘 수 있습니다.

lock에 대해서는 priority donation을 반드시 구현해야 합니다. 다른 Pintos synchronization construct(동기화 구성요소)에 대해 priority donation을 구현할 필요는 없습니다. 하지만 모든 경우에 priority scheduling은 구현해야 합니다.

마지막으로 thread가 자신의 priority를 확인하고 수정할 수 있도록 다음 함수들을 구현하세요. 이 함수들의 skeleton(골격 코드)은 `threads/thread.c`에 제공되어 있습니다.

* * *
    
    
    void thread_set_priority (int new_priority);
    

> 현재 thread의 priority를 새 priority로 설정합니다. 현재 thread가 더 이상 가장 높은 priority가 아니라면 yield(양보)합니다.

* * *
    
    
    int thread_get_priority (void);
    

> 현재 thread의 priority를 반환합니다. priority donation이 있는 경우, 더 높은 donated priority(기부받은 우선순위)를 반환합니다.

thread가 다른 thread의 priority를 직접 수정할 수 있도록 하는 interface(인터페이스)를 제공할 필요는 없습니다.

priority scheduler(우선순위 스케줄러)는 이후 project에서는 사용되지 않습니다.
