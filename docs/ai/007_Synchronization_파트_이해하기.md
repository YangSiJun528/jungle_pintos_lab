# Optimization Barrier, Busy Wait, Lock 비용 핵심 정리

이 문서는 Pintos reference의 Synchronization 설명을 기준으로, `barrier()`, busy wait, lock/semaphore 비용이 헷갈리는 지점을 짧게 정리한 것이다.

## 한 줄 결론

`barrier()`는 CPU를 기다리게 하는 동기화 도구가 아니라, 컴파일러 최적화를 막는 지시문이다.

반면 lock/semaphore/interrupt disable의 비용은 단순히 "어셈블리 한 줄이 느리다"가 아니라, 기다리는 동안 CPU를 낭비하거나 interrupt/preemption을 지연시키는 데서
커진다.

## 구분 기준

동기화 도구를 고를 때는 "무엇으로부터 보호하려는가"를 먼저 나누면 된다.

| 도구                   | 막는 대상                                                 | 보호 의미                                                                                                            | 주의점                                                                            |
|----------------------|-------------------------------------------------------|------------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------------|
| lock/semaphore       | 다른 kernel thread                                      | 같은 자원을 동시에 만지지 못하게 한다. 기다려야 하면 thread가 block될 수 있다.                                                              | interrupt handler는 sleep할 수 없으므로 보통 lock을 잡을 수 없다. lock은 interrupt 자체를 막지 않는다. |
| interrupt disable    | timer interrupt, external interrupt, 그로 인한 preemption | Pintos처럼 한 CPU에서 도는 문맥에서는 현재 코드 구간 중간에 interrupt handler나 다른 thread가 끼어들지 못하게 한다. 그래서 lock처럼 임계영역 보호 효과를 낼 수 있다. | 이 효과는 단일 CPU 전제에 기대는 것이다. 범위가 너무 넓으면 timer tick, 입력 처리, scheduling이 늦어진다.      |
| optimization barrier | compiler optimization                                 | compiler가 memory read/write를 barrier 앞뒤로 재정렬하거나 값을 안 바뀐다고 가정하지 못하게 한다.                                           | runtime에서 다른 thread나 interrupt를 막지 않는다. lock도 아니고 atomic 연산도 아니다.              |

따라서 감각적으로는 다음처럼 이해하면 된다.

- lock은 "thread 사이의 임계영역 보호"에 가깝다.
- interrupt disable은 "현재 CPU에서 interrupt/preemption까지 막는 더 거친 임계영역 보호"에 가깝다.
    - (단, Pintos에서 이것이 lock처럼 보이는 이유는 실행 코어가 하나라는 전제 때문이다.)
- barrier는 "실행 순서"라기보다 "compiler가 보는 memory access 순서와 재읽기 보장"에 가깝다.

다만 lock도 compiler 재정렬을 일반적으로 설명하는 도구는 아니다. Pintos reference도 lock은 interrupt를 막지 않고, lock 구간 내부의 compiler reordering을 막는
장치로 볼 수 없다고 설명한다.

## 단일 CPU 전제

Pintos에서 interrupt를 끄면 임계영역 보호처럼 쓸 수 있는 이유는, 한 시점에 kernel code를 실행하는 CPU가 하나라고 보기 때문이다. 그 CPU에서 timer interrupt가 들어오지
않으면 preemption이 일어나지 않고, 같은 CPU에서 interrupt handler도 중간에 끼어들 수 없다.

하지만 이 말은 "interrupt disable이 일반적으로 lock을 대체한다"는 뜻이 아니다. Linux 같은 멀티코어 커널에서는 한 CPU의 interrupt를 꺼도 다른 CPU는 계속 실행된다. 따라서 다른
CPU가 같은 자료구조에 접근할 수 있다면, 한 CPU에서 interrupt만 끄는 것으로는 임계영역 보호가 되지 않는다.

정리하면 다음과 같다.

- Pintos 단일 CPU 문맥: interrupt disable로 interrupt handler와 다른 thread의 끼어들기를 막을 수 있다.
- 멀티코어 문맥: local interrupt disable은 현재 CPU의 interrupt만 막는다. 다른 CPU와 공유하는 자료는 별도의 lock/atomic/memory ordering이 필요하다.

## `barrier()`의 역할

Pintos의 `barrier()`는 다음 목적이다.

- 컴파일러가 barrier 앞뒤의 memory read/write 순서를 바꾸지 못하게 한다.
- interrupt handler나 다른 thread가 바꿀 수 있는 값을 컴파일러가 "안 바뀐다"고 가정하지 못하게 한다.
- 런타임에서 lock을 잡거나 thread를 재우는 기능은 없다.

예를 들어 `while (ticks == start) barrier();`는 `ticks`가 timer interrupt에서 바뀔 수 있음을 컴파일러에게 알려, 루프가 잘못 최적화되지 않게 하는 용도다.

## Busy Wait의 문제

Busy wait는 조건이 만족될 때까지 CPU를 계속 쓰면서 확인하는 방식이다.

문제는 다음 두 가지다.

- 기다리는 동안 유용한 일을 하지 않는데도 CPU time을 계속 소비한다.
- `thread_yield()`를 반복 호출하는 식이어도 tight loop라면 Pintos 과제 기준으로 busy waiting에 해당한다.

따라서 실제로 기다려야 하는 상황이면, 계속 확인하는 루프보다 thread를 block하고 나중에 깨우는 방식이 맞다.

## `sema_try_down()`을 while에서 반복하면 왜 안 좋은가

`sema_try_down()`은 semaphore를 지금 얻을 수 있으면 얻고, 아니면 false를 반환한다.

이것을 `while`에서 반복하면 보통 다음 문제가 생긴다.

- 얻을 때까지 계속 CPU를 사용한다.
- 매 반복마다 semaphore 확인을 위해 interrupt disable/restore 같은 동기화 비용이 반복된다.
- 결과적으로 busy wait가 된다.

즉 핵심은 "interrupt를 끄는 연산이 여러 번 발생한다"도 맞지만, 더 큰 문제는 "기다리는 동안 현재 thread가 CPU를 계속 소비한다"는 점이다.

## Interrupt를 끄는 비용

Interrupt disable 자체는 짧은 명령일 수 있다. 하지만 interrupt가 꺼진 동안에는 timer interrupt에 의한 preemption이 지연된다.

Pintos는 timer interrupt를 통해 실행 중인 thread를 선점할 수 있다. 따라서 interrupt off 구간이 길어지면 다음 문제가 생긴다.

- timer tick 처리가 늦어진다.
- ready 상태의 다른 thread가 CPU를 받을 기회가 늦어진다.
- interrupt handler가 처리해야 할 일이 밀릴 수 있다.
- 시스템 반응성이 나빠질 수 있다.

주의할 점은 interrupt를 끈다고 다른 thread 객체가 곧바로 `THREAD_BLOCKED` 상태가 되는 것은 아니라는 것이다. 더 정확히는, 다른 ready thread가 실행될 기회가 늦어진다.

## Lock/Semaphore 비용 감각

비용은 상황에 따라 다르다.

- `barrier()`: 거의 컴파일러 지시문에 가깝다.
- 경쟁 없는 lock/semaphore: 함수 호출, interrupt disable/restore, 값 변경, list 확인 정도의 비용이 든다.
- 경쟁 있는 lock/semaphore: 현재 thread가 block되고, scheduler가 다른 thread를 고르며, 나중에 unblock/context switch가 발생할 수 있다.
- busy wait: 기다리는 시간 전체를 CPU로 태울 수 있다.

따라서 "락도 결국 어셈블리 명령 몇 개 아닌가?"라는 감각은 경쟁이 없을 때는 어느 정도 맞다. 하지만 OS에서 중요한 비용은 그 명령 자체보다, 기다림과 스케줄링, interrupt 지연이 만들어내는 부작용이다.

## 출처

- `docs/reference/pintos-kaist-original/5_appendix/1_synchronization.md`
- `docs/reference/pintos-kaist-original/1_project1/0_introduction.md`
- `pintos/include/threads/synch.h`
- `pintos/threads/synch.c`
