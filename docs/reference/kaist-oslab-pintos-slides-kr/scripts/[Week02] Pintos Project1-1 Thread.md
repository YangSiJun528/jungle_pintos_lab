# [Week02] Pintos Project1-1 Thread

Source: https://youtu.be/myO2bs5LMak?si=3rbvoNu2h8X5hjnz

## Intro

이 강의는 첫 번째 project에 대한 설명입니다. project의 제목은 Threads입니다. 이 project에서는 세 가지 주요 주제를 구현합니다. 첫 번째는 Alarm Clock, 두 번째는 Priority Scheduling, 세 번째는 Advanced Scheduler입니다.

## Overview

첫 번째 주제는 Alarm Clock입니다. 우리는 timer alarm이라는 system을 수정할 것입니다. Timer alarm은 일정 시간이 지난 뒤 process를 깨우는 system입니다.

현재 Pintos는 alarm에 busy waiting을 사용합니다. 우리는 Pintos가 alarm에 sleep and wakeup paradigm을 사용하도록 수정할 것입니다. 이것이 현재 Pintos에서 `timer_sleep()`이 구현된 방식입니다. 현재 `timer_sleep()` 구현에서 process는 alarm time이 되었는지 확인하기 위해 ready state와 running state 사이를 오갑니다. 그 결과 두 상태만 사용하며, CPU cycle을 기다리면서 CPU instruction을 계속 소모합니다.

Data structure 관점에서 보면, thread는 running mode에 놓이고, alarm을 호출하면 자기 자신을 ready list의 맨 뒤로 다시 넣은 뒤 다시 실행됩니다. 기본적으로 `timer_sleep()` 구현에서는 이 loop를 계속 실행합니다.

`timer_sleep()`은 이렇게 구현되어 있습니다. 이 코드를 보면 먼저 `timer_sleep()`이 호출될 때 현재 시간을 기록합니다. 그런 다음 while loop 안에서 `thread_yield()`를 계속 실행합니다. 그래서 CPU를 release하고, 다시 running state에 들어오면 시간을 확인합니다. elapsed time이 ticks보다 작으면 다시 CPU를 release합니다. 기본적으로 `timer_sleep()`을 호출한 process는 ready state와 running state 사이를 계속 switch합니다.

`thread_yield()`의 세부 내용을 살펴봅시다. 자세히 볼 가치가 있습니다. 먼저 current thread structure의 pointer를 얻고, interrupt를 disable합니다. 그런 다음 current thread structure를 ready list의 맨 뒤에 넣습니다. 이어서 현재 running thread의 state를 `THREAD_READY`로 바꾸고 context switch를 호출합니다. context switch가 끝나면 interrupt level을 원래 상태로 되돌립니다.

## Functions in `thread_yield()`

`thread_yield()` 안에는 다섯 가지 핵심 함수가 있습니다.

첫 번째는 `thread_current()`입니다. 이 함수는 current thread pointer를 반환합니다. 그런 다음 interrupt를 disable합니다. interrupt disable의 counterpart인 다른 함수가 있는데, 그것이 `intr_set_level()`입니다. 이 함수는 interrupt level을 복구합니다.

그리고 `list_push_back()`은 주어진 object를 지정된 list의 맨 뒤에 놓는 함수입니다. 이 함수의 목적은 current thread structure를 ready list의 맨 뒤에 넣는 것입니다. 그런 다음 context switch를 수행하기 위해 `schedule()`을 호출합니다.

이 project의 목적은 `timer_sleep()`을 더 효율적으로 만드는 것입니다. 우리는 blocked state를 도입할 것입니다. blocked state를 사용해서 `timer_sleep()`을 구현할 것입니다. process가 `timer_sleep()`을 호출하면 운영체제는 자기 자신을 blocked state에 넣고, 운영체제가 blocked process를 깨울 책임을 지며 시간을 자주 확인합니다. 이 접근을 통해 운영체제는 CPU cycle을 절약할 수 있고, 더 중요하게는 전력 소모를 줄일 수 있습니다.

## Design: Sleep/Wakeup-Based Alarm Clock

이것이 design입니다. 코드의 앞부분에서 보듯이 현재 Pintos에는 두 list만 있습니다. 첫 번째는 all list이고, 다른 하나는 ready list입니다. Ready list는 CPU에서 실행되기를 기다리는 process 집합을 나타내는 data structure입니다.

이제 새 data structure를 도입할 것입니다. 이것은 blocked thread들의 list입니다. 이 list는 blocked state에 있는 thread들의 집합을 나타냅니다. `timer_sleep()`이 호출될 때, process가 `timer_sleep()`을 호출하면 운영체제는 thread를 sleep list에 넣습니다. 그런 다음 운영체제는 timer를 계속 확인하고, 시간이 되면 해당 thread를 깨웁니다. Wakeup은 thread를 sleep list에서 ready list로 옮긴다는 뜻입니다.

이것이 `timer_sleep()` algorithm의 design입니다. 다음은 implementation detail입니다. 먼저 sleep queue를 정의해야 합니다. sleep list를 정의해야 합니다. 이름은 직접 정할 수 있지만, sleep list로 정의해야 합니다. 그리고 초기화해야 합니다. 모든 data structure와 마찬가지로 적절하게 initialize해야 합니다. 생각할 점은 declaration statement를 어디에 둘 것인지, 언제 initialize할 것인지입니다. 이 부분은 여러분이 구현해야 합니다.

## Global Tick vs. Local Tick

blocked-state `timer_sleep()`을 구현할 때는 두 가지를 도입해야 합니다. 첫 번째는 global tick이고, 두 번째는 local tick입니다.

기본 design idea는 timer interrupt handler가 실행될 때마다 kernel이 깨울 thread가 있는지 확인한다는 것입니다. 이를 위해 먼저 blocked thread list를 scan하고, blocked thread의 local tick이 global tick보다 큰지 찾아야 합니다. local tick에서 thread는 자신이 깨어나야 할 시간을 유지합니다.

현재 우리의 thread structure에는 이 field가 없으므로, thread structure를 수정해 wake up할 시간을 저장해야 합니다. 또한 효율성을 위해 global variable 하나를 도입해야 합니다. 이름은 global tick이든 다른 이름이든 상관없습니다. 이 변수는 thread들이 가진 local tick 중 minimum value를 저장합니다.

timer interrupt handler가 실행될 때마다 tick, 즉 global tick variable을 확인합니다. current time이 tick보다 작거나 같으면, 다시 말해 current time이 tick보다 크거나 같으면, blocked list 안의 일부 thread를 깨워야 한다는 뜻입니다. 그렇지 않으면 모든 blocked thread를 scan할 필요가 없습니다. global tick variable을 도입한 이유는 sleep list를 scan하는 시간을 아끼기 위해서입니다.

## Modify Thread Structure

data structure에는 wake up할 alarm time을 나타내는 새 field를 추가해야 합니다.

이것은 `timer_sleep()` 코드의 실제 구현입니다. busy waiting을 위해 while loop 안에 자기 자신을 넣는 대신, wake up해야 할 시간까지 아직 시간이 남아 있다면 `thread_sleep()`을 호출해 자기 자신을 sleep list에 넣습니다. wake up해야 하는 alarm time을 start plus ticks로 전달합니다.

여기에 흥미로운 property가 하나 있습니다. 중간에 context switch가 있기 때문에 line two에서 start 값이 invalid해질 가능성이 있습니다. statement를 실행할 때쯤에는 얻어 둔 시간이 더 이상 유효하지 않을 수 있습니다. 지금은 이 점을 잊고 넘어가겠지만, 정말 좋은 system programmer라면 이를 고칠 방법을 생각해 볼 수 있습니다.

## `thread_sleep()`

이것이 `thread_sleep()` 함수에서 구현하고 싶은 body입니다. current thread가 idle thread라면 caller thread의 state를 blocked로 바꾸고, thread structure를 통해 wake up할 local tick을 저장해야 합니다. 필요하다면 global tick을 update한 다음 `schedule()`을 호출해야 합니다. thread list를 통해 thread structure를 삽입할 때 interrupt를 disable하는 것을 잊으면 안 됩니다.

`thread_sleep()`에서 중요한 것은 caller thread의 state를 blocked로 바꾸고 sleep queue에 넣는 것입니다.

## Implementation of Alarm Clock

Timer interrupt 구현은 모든 것의 핵심입니다. timer interrupt에서 timer handler를 수정해야 합니다. timer interrupt handler 안에서 운영체제는 timer가 발생할 때마다 어떤 thread를 깨워야 하는지 결정해야 합니다.

깨울 thread가 있으면 sleep queue에서 제거한 다음 ready list에 삽입해야 합니다. 여러분은 ready list와 sleep list를 수정하게 됩니다. sleep list와 ready list를 수정할 때는 list 수정 전후로 interrupt를 disable하고 enable하는 것을 잊으면 안 됩니다. 또한 thread를 ready list에 넣을 때 thread state를 sleep에서 ready로 바꾸는 것도 잊으면 안 됩니다.

기억해야 할 점은 sleep list를 어떻게 구성하느냐에 따라 깨울 thread를 식별하는 시간이 크게 달라질 수 있다는 것입니다. 예를 들어 이것이 sleep list라고 해 봅시다. sleep list를 alarm clock time 기준으로 정렬하면, 이것은 10, 이것은 101, 이것은 105입니다. 이 blocked list는 alarm time의 ascending order로 정렬되어 있습니다. 깨울 thread를 찾고 싶을 때 queue의 beginning부터 scan을 시작하고, 깨울 마지막 thread를 찾을 때까지만 scan한 뒤 멈출 수 있습니다. 하지만 이 list가 정렬되어 있지 않다면, 깨울 thread를 찾아야 할 때마다 전체 list를 scan해야 합니다.

이것이 timer interrupt를 수정한 코드입니다. 이 부분을 추가해야 합니다. sleep list를 확인하고, global tick을 확인하고, 깨울 thread가 있는지 찾습니다. 그런 다음 필요하면 그 thread들을 ready list로 옮기고 global clock을 update합니다.

## Summary

요약하면 sleep and wakeup protocol에 기반한 `timer_sleep()` 함수를 만들기 위해 몇 가지 함수를 수정해야 합니다. `thread_init()`, `timer_sleep()`, 그리고 timer interrupt입니다.

## Design Tip for Modularization

자기 코드를 작성하는 방법은 다양하지만, modularization을 위한 design suggestion을 따르면 네 가지 함수를 추가하는 것이 좋습니다.

첫 번째는 thread state를 blocked로 설정한 뒤 sleep queue에 삽입하는 함수입니다. 두 번째 함수는 sleep queue에서 깨울 thread를 찾고 깨우는 함수입니다. 깨운다는 것은 thread를 sleep queue에서 ready list로 넣는다는 의미입니다. 또한 thread들이 가진 tick의 minimum value를 저장하는 함수를 작성할 수도 있습니다. 마지막으로 tick의 minimum value를 반환하는 함수를 작성할 수도 있습니다.

모든 코드를 작성한 뒤에는 code가 alarm test를 통과하는지 확인해야 합니다. 이것이 결과이고, 테스트를 통과하길 바랍니다.

## Outline

두 번째 주제는 Priority Scheduling입니다. Pintos는 FIFO scheduling을 사용합니다. 여러분은 Priority Scheduling을 위해 Pintos scheduler를 수정해야 합니다.

여러분이 해야 할 일의 결과는 다음과 같습니다. 첫째, ready list를 thread priority 기준으로 정렬합니다. 둘째, semaphores, condition variables, locks 같은 synchronization primitive의 wait list를 priority 기준으로 정렬합니다. 또한 preemption도 구현해야 합니다. preemption point는 thread가 ready list에 들어갈 때입니다. timer interrupt가 호출될 때마다 preemptibility를 확인할 필요가 없다는 점이 중요합니다. 이 scheduler algorithm에서 운영체제는 새 thread가 ready list에 도착할 때만 preemption을 확인합니다.

두 파일을 수정해야 합니다. 이것이 design입니다. ready list를 검사하고 다음에 실행할 thread를 선택할 때 highest priority를 가진 thread를 얻습니다. 이것이 한 가지입니다. lock을 기다리는 thread가 있을 때 lock이 사용 가능해지면, 운영체제는 highest priority를 가진 thread를 선택합니다. 이 두 가지를 구현해야 합니다.

## Three Things to Consider

고려해야 할 것이 세 가지 있습니다.

첫째, ready list에서 실행할 thread를 선택할 때 highest priority를 가진 thread를 선택해야 합니다.

둘째, 새 thread를 ready list에 삽입할 때 운영체제는 running thread의 priority와 기존 priority를 비교해야 하며, 새로 삽입된 thread가 현재 running thread보다 higher priority라면 schedule해야 합니다.

셋째, 같은 rule은 locks, semaphores, condition variables 같은 synchronization primitive를 기다리는 thread 집합에도 적용됩니다. lock, semaphore, 또는 condition variable이 사용 가능해지면 운영체제는 highest priority를 가진 thread를 선택합니다.

## Priority in Pintos

Pintos의 priority를 설명하겠습니다. Pintos에서 priority range는 0부터 63까지입니다. 총 64개의 priority level이 있고, 숫자가 클수록 priority가 높습니다. default priority는 thread가 처음 생성될 때 설정되며, default priority value는 31입니다.

Pintos operating system은 두 함수를 제공합니다. 첫 번째는 thread의 priority를 지정한 값으로 설정하는 `thread_set_priority()`이고, 두 번째는 주어진 thread의 priority를 얻는 `thread_get_priority()`입니다.

Priority-based scheduling을 구현할 때는 몇 가지를 수정해야 합니다. 첫 번째는 `thread_create()` 안에서 ready list를 ready list 안 thread들의 priority 기준으로 정렬된 상태로 유지하고 싶다는 것입니다. thread를 생성한 뒤 삽입할 때 priority order에 맞춰 thread를 넣습니다. 이것은 매우 expensive할 것입니다.

두 번째는 thread가 ready list에 추가될 때 운영체제가 새로 들어온 thread의 priority와 현재 실행 중인 thread의 priority를 비교해야 한다는 것입니다. 새로 들어온 thread의 priority가 더 높다면, 현재 running thread를 switch out하고 새 thread를 CPU에 올리기 위해 `schedule()`을 호출해야 합니다.

## `thread_create()`

이것은 `thread_create()`의 code입니다. thread를 unblock한 뒤, 현재 running thread와 새로 삽입된 thread의 priority를 비교해야 합니다. 새로 도착한 thread가 higher priority를 가진다면 기존 thread는 CPU를 양보해야 합니다.

## Others to Modify

수정해야 할 다른 부분도 몇 가지 있습니다.

thread가 ready state에서 unblock될 때, ready state에 놓일 때도 priority order에 맞춰 삽입해야 합니다. 또한 thread가 `thread_yield()`를 호출할 때도 자기 자신을 priority order에 맞춰 ready list에 넣어야 합니다.

수정해야 할 또 다른 함수가 있습니다. 바로 `thread_set_priority()`입니다. `thread_set_priority()`는 주어진 priority value로 thread의 priority를 변경합니다. 현재 Pintos에서 `thread_set_priority()`는 단순히 priority를 새 priority value로 설정합니다. 하지만 새로 수정된 algorithm에서는 `thread_set_priority()`가 priority value만 설정하는 것이 아니라 ready list 안에서 thread의 location 또는 position도 조정해야 합니다. ready list가 list 안 thread들의 priority value 기준으로 정렬되어 있어야 하기 때문입니다.

## Hint: `thread_unblock()`

`thread_unblock()` 코드를 어떻게 수정할 수 있는지 세부 내용을 보여드리겠습니다. thread를 unblock할 때 thread priority 기준으로 ready list에 배치할 것입니다. 기존 코드에서는 운영체제가 unblocked thread를 `list_push_back()`처럼 ready list의 맨 뒤에 넣습니다. 따라서 이 코드를 지우고 새로 unblocked된 thread를 priority 기준으로 ready list에 넣습니다.

## Change the Synchronization Primitives

synchronization primitive도 변경해야 합니다. locks, semaphores, condition variables가 있습니다. lock, semaphore, 또는 condition variable이 사용 가능해지면 thread priority 기준으로 waiting thread를 깨워야 합니다.

## FIFO Lock/Unlock in Priority-Less Pintos

이 view graph는 Pintos에서 lock이 어떻게 유지되는지 보여 줍니다. Pintos operating system은 lock holder를 결정할 때 first-in, first-served를 사용합니다.

thread A, B, C, D 네 개가 있다고 가정합시다. A는 현재 lock을 hold하고 있고 이 시점까지 계속 실행됩니다. A가 lock을 hold하는 동안 B가 먼저 request했고, D가 다음으로 request했으며, C가 세 번째로 request했습니다.

A가 이 시점에 lock을 release한 뒤, 운영체제는 lock을 acquire할 thread를 결정합니다. 이때 lock을 기다리는 thread는 B, D, C 세 개입니다. 이 세 thread 중 D가 highest priority를 가집니다. 하지만 D가 highest priority임에도 운영체제는 단순히 list의 첫 thread를 제거하고 lock을 assign합니다. 그래서 lock을 acquire하는 순서는 B, D, C입니다.

B와 D 사이에는 priority inversion이 있습니다. 더 높은 priority를 가진 process가 더 낮은 priority의 process를 기다리고 있습니다. 이는 Pintos가 FIFO lock/unlock mechanism을 사용하기 때문에 발생합니다.

## Priority-Based Lock/Unlock

Priority-based lock/unlock mechanism에서는 waiter들이 priority를 기준으로 lock을 acquire합니다. 같은 예시입니다. thread A, B, C, D 네 개가 있습니다. B가 request했고, D가 request했고, C가 lock을 request했습니다.

A가 lock을 release한 뒤, 이 시점에는 세 thread가 있습니다. 이전 slide와 달리 여기의 waiting list는 priority 기준으로 ordered되어 있습니다. thread A가 lock을 release하면 highest priority를 가진 thread D가 lock을 얻습니다. D가 lock을 release한 뒤에는 다음으로 높은 priority를 가진 thread C가 lock을 얻고, B가 마지막으로 lock을 얻습니다.

## Semaphore in Pintos

semaphore와 condition variable의 basic function을 간단히 소개하고, semaphore와 condition variable에서 어떤 함수를 수정해야 하는지 짚어 보겠습니다.

semaphore에는 세 함수가 있습니다. 첫 번째는 semaphore를 주어진 값으로 initialize하는 `sema_init()`입니다. 두 번째는 `sema_down()`입니다. 이것은 semaphore를 요청하고, acquire되면 process가 계속 진행합니다. 하지만 process가 semaphore acquire에 실패하면 block해야 합니다. `sema_up()`은 semaphore를 release합니다. 여기서는 `sema_down()`과 `sema_up()`을 수정해야 합니다.

lock에는 세 함수가 있습니다. `lock_init()`, `lock_acquire()`, `lock_release()`입니다. 보시다시피 lock은 semaphore로 구현됩니다. 따라서 priority 기반으로 lock primitive를 수정하려면 semaphore를 수정하는 것만으로 충분합니다.

## Condition Variable in Pintos

세 번째 함수는 condition variable입니다. condition variable에는 네 가지 중요한 함수가 있습니다. 첫 번째는 condition variable data structure를 initialize하는 `cond_init()`입니다. 두 번째는 `cond_wait()`입니다. process가 `cond_wait()`를 호출하면 process는 blocked state에 들어가고 condition variable의 signal을 기다립니다. 세 번째는 `cond_signal()`입니다. 이것은 condition variable에서 기다리는 highest priority thread에 signal을 보냅니다. 또 다른 함수인 `cond_broadcast()`는 condition variable에서 기다리는 모든 thread에 signal을 보냅니다.

이것들이 수정해야 할 함수들입니다. 수정해야 할 함수는 두 개입니다. 첫 번째는 `sema_down()`이고, 다른 하나는 `cond_wait()`입니다. 이 code 안에서 process가 wait list에 들어갈 때 priority 기준으로 정렬되도록 수정해야 합니다. 또한 모든 list가 priority 기준으로 ordered되도록 `thread_set_priority()`뿐만 아니라 `sema_up()`과 `cond_signal()`도 수정해야 합니다.

## Priority Inversion

고려해야 할 중요한 문제가 하나 있습니다. 첫 번째는 priority inversion입니다. Priority inversion은 higher priority process가 lower priority process를 기다리는 상황입니다.

이 상황을 생각해 봅시다. thread A, thread B, thread C 세 개가 있습니다. A는 실행 중이고 이 시점에 lock을 acquire했습니다. 그런 다음 계속 실행됩니다. C는 lock을 요청하지만 lock이 A에게 hold되어 있으므로 A는 실행 중이고 C는 이 시점부터 blocked됩니다. 이때 A가 이 지점까지 실행되었고, 그 다음 B가 도착했습니다. B는 A보다 higher priority이므로, A가 ready list로 들어가는 동안 B가 실행됩니다.

그러면 흥미로운 일이 발생합니다. 현재 C는 A를 기다리고 있기 때문에 blocked되어 있습니다. 그런데 A는 B가 A보다 higher priority이기 때문에 CPU를 B에게 넘깁니다. 문제는 C와 B의 관계입니다. C는 B보다 higher priority이지만, 결과적으로 C는 B가 끝나기를 기다리는 셈이 됩니다. 이것을 priority inversion이라고 합니다. 우리의 priority scheduling에서는 이 문제를 고쳐야 합니다.

1997년에 Mars Pathfinder가 priority inversion 때문에 operating system crash로 멈췄습니다. NASA engineer들은 Pathfinder의 source code를 download했고, crash가 priority inversion 때문임을 확인했습니다. 그런 다음 patch를 찾아 system을 고치고, code를 Pathfinder에 upload하여 동작하게 만들었습니다. 이것은 priority inversion의 흥미로운 사례이자 중요성을 보여 주는 이야기입니다.

priority inversion 문제를 해결하기 위해 사용할 한 가지가 있습니다. 이것은 priority donation이라고 합니다. Priority donation은 process의 priority를 lock holder에게 inherit시키는 동작입니다.

다시 thread A, thread B, thread C 세 개가 있다고 해 봅시다. A는 이 시점에 lock을 hold한 채 실행 중입니다. A가 실행 중이고, 이 시점에서 C가 lock을 요청하지만 lock은 thread A가 hold하고 있습니다. 이때 C와 A의 priority를 비교하면 A의 priority가 낮습니다. C가 lock을 요청하고 A가 lock을 hold하고 있음을 발견하면, C는 자기 priority를 A에게 donate해서 A의 priority가 C의 level까지 boost되도록 합니다.

A의 priority가 낮은 level에서 C의 level로 boost된 직후, B가 도착한 것을 볼 수 있습니다. 원래 scheduling algorithm에서는 B가 process A를 preempt해야 합니다. 하지만 이 경우 process A의 priority가 C의 level까지 boost되었으므로 B가 process A를 preempt할 방법이 없습니다. Process A는 C의 priority level로 계속 실행되고, 끝나면 lock이 process C에게 넘어갑니다. Process C가 실행되고, process C가 execution을 마친 뒤 lock을 release합니다. 그러면 process B가 마침내 execution opportunity를 얻습니다. lock holder에게 priority를 donate함으로써 priority inversion을 피합니다. 이 technique이 priority donation입니다.

priority donation이 없는 system에는 lock L이 있습니다. Lock은 현재 priority 10인 thread one에게 allocated되어 있습니다. thread one이 lock을 acquire한 뒤 thread two, thread three, thread four 세 thread가 도착했습니다. 각 thread의 priority는 9, 12, 8입니다. priority donation이 없으면 thread one의 priority는 10으로 유지됩니다. 하지만 priority donation을 사용하면 lock holder의 priority는 highest priority thread의 priority가 됩니다. 여기에는 T2, T3, T4 세 thread가 있습니다. T2의 priority는 9, T3의 priority는 12, T4의 priority는 8입니다. 이 thread들 중 thread three가 자기 priority를 thread one에게 donate합니다. 이것이 priority donation입니다.

## Nested Donation

고려해야 할 이슈가 몇 가지 있습니다. 첫 번째는 nested donation입니다.

이 scenario를 생각해 봅시다. thread one, thread two, thread three 세 개가 있습니다. thread one의 priority는 10, thread two의 priority는 9, thread three의 priority는 7입니다. 어떤 이유로 먼저 thread one이 lock A를 hold하고 있고, thread two가 lock A를 request했다가 blocked되었습니다. 그런데 thread two는 lock B를 hold하고 있고, thread three는 lock C를 hold한 채 lock B가 release되기를 기다리고 있습니다. chained lock-holding relationship이 있는 것입니다.

여기에 thread four가 오면 어떻게 될까요? thread four는 priority 14를 가지고 있고 lock C를 요청합니다. thread four가 lock C를 request하면, priority donation 때문에 이 priority를 lock holder T3에게 donate하여 T3의 priority를 7에서 14로 만듭니다. 그런데 여기서 T3는 다시 자기 priority를 자신의 lock holder에게 donate하므로 T2의 priority가 9에서 14로 update됩니다. 다시 T2는 자기 priority를 자신의 lock holder에게 donate하고, priority 14를 lock holder에게 donate하므로 T1의 priority level도 10에서 14로 update됩니다. 이것을 nested donation이라고 합니다.

priority donation 구현에서는 priority scheduling algorithm 안에 nested donation feature를 구현해야 합니다.

## Multiple Donation

다음 주제는 multiple donation입니다. thread one은 lock A, lock B, lock C 세 lock을 hold하고 있습니다. thread one의 original priority는 10입니다. T2가 lock A를 request합니다. T2의 priority는 12이므로 thread T1은 priority 12를 donate받습니다. 그런 다음 T3가 lock B를 request하고, T3의 priority는 11입니다. 현재 T2가 donate한 priority가 12이므로 T3는 lock holder에게 자기 priority를 donate하지 않습니다.

이제 T4가 C를 request하고, T4의 priority는 13입니다. T4가 가진 priority 13은 priority 12보다 크므로, 결국 T1의 priority는 13이 됩니다.

T1이 lock C를 unlock했다고 가정합시다. lock C를 unlock한 결과 lock은 T4에게 allocated됩니다. lock C를 release한 뒤 T1의 priority가 original priority 10이 되어서는 안 됩니다. T1의 priority는 T1에게 priority를 donate한 thread들 중 largest priority로 update되어야 합니다. 여기서 T1의 priority는 이제 12가 됩니다. 이것이 multiple donation입니다.

priority donation mechanism에서는 nested donation과 multiple donation을 구현해야 합니다.

## Data Structure for Multiple Donation

idea는 간단합니다. multiple donation을 지원하려면 thread가 donor들의 list를 유지해야 합니다. lock을 release할 때마다 donor들을 search하고 남은 donor들 중 highest priority를 얻습니다.

## Data Structure for Nested Donation

nested donation을 위해서는 thread가 기다리는 lock을 유지해야 합니다. priority를 inherit하면, current priority를 child에게 inherit시킬 필요가 있는지 확인해야 합니다. 이것이 nested donation을 구현하는 방식입니다.

## Implementation of Priority Donation

이것들이 priority donation에서 수정할 함수들입니다. data structure initialization을 수정해야 하고, `lock_acquire()`, `lock_release()`, `thread_set_priority()`도 수정해야 합니다.

`lock_acquire()`에서 lock을 사용할 수 없다면 lock의 address를 저장해야 합니다. current priority를 저장하고 donating thread들을 list로 유지한 다음, priority를 donate해야 합니다.

lock이 release되면 donation list에서 해당 lock을 hold한 thread들을 제거하고 priority를 적절히 update해야 합니다. 또한 priority를 set할 때도 donation을 고려해서 priority를 set해야 합니다. 이것들이 수정해야 할 issue입니다. 이것이 test 결과입니다.
