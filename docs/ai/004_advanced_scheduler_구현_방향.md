# Advanced Scheduler 구현 방향

이 문서는 `docs/reference` 하위 자료만 기준으로, Pintos Project 1의 Advanced Scheduler를 어떻게 이해하고 구현해야 하는지 정리한 것이다. 핵심은 기존 priority scheduler를 버리고 새 구조를 통째로 만드는 것이 아니라, 기존 priority 기반 scheduling 흐름 위에 `-mlfqs` 정책을 분기해 붙이는 것이다.

## 결론

Advanced Scheduler는 priority scheduler와 완전히 다른 scheduler가 아니다. 실행할 thread를 고를 때 여전히 priority를 기준으로 한다. 차이는 priority를 누가 정하느냐에 있다.

기본 priority scheduler에서는 thread가 생성될 때 받은 priority와 `thread_set_priority()`로 설정한 priority가 중요하다. 반면 `-mlfqs`가 켜진 4.4BSD scheduler에서는 thread가 priority를 직접 제어하지 않는다. scheduler가 `nice`, `recent_cpu`, `load_avg` 값을 이용해 priority를 계속 다시 계산한다.

따라서 구현 방향은 다음처럼 잡는 것이 맞다.

1. 기존 priority scheduler의 ready list 선택 흐름은 유지한다.
2. `thread_mlfqs`가 true일 때만 priority 계산 방식을 4.4BSD 방식으로 바꾼다.
3. priority donation은 `thread_mlfqs`가 true일 때 동작하지 않게 한다.
4. timer tick마다 필요한 scheduler 값을 갱신한다.
5. fixed-point arithmetic을 사용해 `recent_cpu`와 `load_avg`를 계산한다.

## 기존 구현을 어디까지 살리는가

Advanced Scheduler도 결국 priority가 가장 높은 thread를 실행해야 한다. 같은 priority에 여러 thread가 있으면 round-robin 순서로 실행한다. 이 요구는 기존 priority scheduler에서 만든 ready list 정렬, highest-priority 선택, preemption 흐름과 잘 맞는다.

그래서 ready list를 priority 순서로 관리하는 구현은 그대로 활용할 수 있다. 문서에서는 64개의 ready queue를 설명하지만, FAQ에서는 동작이 같다면 하나의 queue를 써도 된다고 한다. 즉 구현의 핵심은 queue 개수가 아니라 결과 동작이다.

다만 기존 priority scheduler에서 priority donation을 구현했다면, 그 부분은 Advanced Scheduler 경로에서는 꺼야 한다. 4.4BSD scheduler는 donation을 포함하지 않고, 테스트도 priority donation과 Advanced Scheduler를 동시에 보지 않는다.

## `thread_mlfqs`가 켜졌을 때 달라지는 동작

`-mlfqs` 옵션이 들어오면 `thread_mlfqs`가 true가 된다. 이때는 다음 규칙을 따라야 한다.

- `thread_create()`에 전달된 priority 인자는 scheduler priority 결정에 쓰지 않는다.
- `thread_set_priority()` 호출은 priority를 직접 바꾸지 않는다.
- `thread_get_priority()`는 scheduler가 계산해 둔 현재 priority를 반환한다.
- priority donation은 동작하지 않는다.

이 말은 `thread_mlfqs`가 false인 기본 경로와 true인 Advanced Scheduler 경로를 명확히 나누라는 뜻이다. 기본 priority scheduler의 동작까지 바꾸면 안 된다.

## 추가해야 하는 값

Advanced Scheduler를 위해 thread마다 다음 값이 필요하다.

- `nice`: thread가 CPU를 얼마나 양보할지 나타내는 정수 값
- `recent_cpu`: thread가 최근에 CPU를 얼마나 사용했는지 나타내는 fixed-point 값

시스템 전체에는 다음 값이 필요하다.

- `load_avg`: 최근 1분 동안 실행 준비가 된 thread 수의 이동 평균

초기값은 다음처럼 잡는다.

- initial thread의 `nice`는 0
- 새 thread의 `nice`는 parent thread에서 상속
- 첫 thread의 `recent_cpu`는 0
- 다른 새 thread의 `recent_cpu`는 parent thread에서 상속
- `load_avg`는 boot 시 0

## priority 계산

Advanced Scheduler의 priority는 다음 공식으로 계산한다.

```text
priority = PRI_MAX - (recent_cpu / 4) - (nice * 2)
```

이 값은 thread 초기화 시 계산하고, 이후 매 4번째 timer tick마다 모든 thread에 대해 다시 계산한다. 계산 결과는 내림 처리하고, 항상 `PRI_MIN`부터 `PRI_MAX` 사이에 들어오도록 보정해야 한다.

이 공식의 의미는 단순하다. 최근에 CPU를 많이 쓴 thread는 priority가 내려가고, nice 값이 큰 thread도 priority가 내려간다. 반대로 최근에 CPU를 덜 쓴 thread는 상대적으로 다시 CPU를 받을 가능성이 커진다. 이 점이 strict priority scheduling에서 생길 수 있는 starvation을 완화한다.

## `recent_cpu` 계산

`recent_cpu`는 두 단계로 관리한다.

첫째, timer interrupt가 발생할 때마다 현재 실행 중인 thread의 `recent_cpu`를 1 증가시킨다. 단, idle thread가 실행 중인 경우에는 증가시키지 않는다.

둘째, 1초마다 모든 thread의 `recent_cpu`를 다음 공식으로 다시 계산한다.

```text
recent_cpu = (2 * load_avg) / (2 * load_avg + 1) * recent_cpu + nice
```

이 재계산은 정확히 `timer_ticks () % TIMER_FREQ == 0`인 시점에만 해야 한다. 다른 tick에서 미리 하거나 늦게 하면 테스트가 기대하는 값과 어긋날 수 있다.

중요한 주의점이 하나 있다. `nice`가 음수인 thread의 `recent_cpu`는 음수가 될 수 있다. 이 값을 0으로 고정하면 안 된다.

## `load_avg` 계산

`load_avg`는 시스템 전체 값이며, 1초마다 다음 공식으로 갱신한다.

```text
load_avg = (59 / 60) * load_avg + (1 / 60) * ready_threads
```

`ready_threads`는 update 시점에 실행 준비가 되었거나 실행 중인 thread 수다. idle thread는 포함하지 않는다.

`load_avg` 역시 정확히 `timer_ticks () % TIMER_FREQ == 0`인 시점에만 갱신해야 한다. 이 값은 `recent_cpu` 계산에 들어가므로, 갱신 시점이 흔들리면 priority 계산도 함께 흔들린다.

## fixed-point arithmetic

Pintos kernel에서는 floating-point arithmetic을 쓰지 않는다. 따라서 `recent_cpu`와 `load_avg`처럼 실수 성격의 값은 fixed-point arithmetic으로 표현해야 한다.

문서 기준은 17.14 fixed-point format이다. 하위 14 bit를 소수부로 쓰고, 나머지를 정수부와 부호로 사용한다. 구현에서는 정수를 fixed-point로 바꾸고, fixed-point 값을 다시 정수로 바꾸는 helper를 두는 것이 자연스럽다.

곱셈과 나눗셈은 overflow를 피하기 위해 64-bit 연산을 사용해야 한다. 특히 `load_avg`와 `recent_cpu`를 직접 곱하는 순서는 overflow 위험이 있으므로, `recent_cpu` 공식에서는 계수를 먼저 계산한 뒤 그 계수를 `recent_cpu`에 곱하는 쪽이 문서의 권장 방향이다.

## timer interrupt에서 해야 할 일

Advanced Scheduler는 timer interrupt와 강하게 연결되어 있다. tick이 증가한 뒤 일반 kernel thread가 실행되기 전에 scheduler data가 먼저 갱신되어야 한다. 그래야 thread가 새 tick 값은 보지만 scheduler 값은 예전 값을 보는 불일치가 생기지 않는다.

timer interrupt 경로에서 해야 할 일은 다음 순서로 이해하면 된다.

1. `thread_mlfqs`가 true인지 확인한다.
2. idle thread가 아니라면 running thread의 `recent_cpu`를 1 증가시킨다.
3. 현재 tick이 1초 경계라면 `load_avg`를 갱신한다.
4. 같은 1초 경계에서 모든 thread의 `recent_cpu`를 갱신한다.
5. 4 tick마다 모든 thread의 priority를 다시 계산한다.
6. 필요하면 현재 thread가 계속 최고 priority인지 확인하고 yield되도록 한다.

FAQ는 timer interrupt handler가 너무 오래 걸리면 Advanced Scheduler 테스트가 흔들릴 수 있다고 설명한다. 따라서 timer interrupt에서 모든 일을 하더라도, 불필요하게 무거운 작업을 넣지 않는 것이 중요하다.

## 수정 대상 함수

문서에서 직접 언급한 수정 지점은 다음과 같다.

- `init_thread()`: `nice`, `recent_cpu` 초기화
- `thread_set_priority()`: Advanced Scheduler에서는 priority 직접 설정 비활성화
- timer interrupt function: tick 단위, 1초 단위, 4 tick 단위 갱신 처리
- `lock_acquire()`: Advanced Scheduler에서는 priority donation 비활성화
- `lock_release()`: Advanced Scheduler에서는 priority donation 비활성화
- `thread_set_nice()`: 현재 thread의 nice 변경, priority 재계산, 필요 시 yield
- `thread_get_nice()`: 현재 thread의 nice 반환
- `thread_get_load_avg()`: `load_avg * 100`을 반올림해 반환
- `thread_get_recent_cpu()`: 현재 thread의 `recent_cpu * 100`을 반올림해 반환

## 구현 순서 제안

문서 기준으로 구현 순서를 잡으면 다음 흐름이 가장 자연스럽다.

1. fixed-point helper를 준비한다.
2. `struct thread`에 `nice`, `recent_cpu`를 추가하고 `load_avg`를 둔다.
3. thread 초기화와 생성 시 상속 규칙을 반영한다.
4. priority 계산 helper를 만든다.
5. `thread_set_nice()`, `thread_get_nice()`, `thread_get_recent_cpu()`, `thread_get_load_avg()`를 구현한다.
6. timer interrupt 경로에 `recent_cpu`, `load_avg`, priority 갱신을 연결한다.
7. `thread_mlfqs`가 true일 때 priority 직접 설정과 donation이 동작하지 않게 한다.
8. MLFQS 테스트만 별도로 돌려 확인한다.

이 순서가 적절한 이유는 계산 도구와 상태값이 먼저 있어야 timer interrupt 갱신을 안전하게 붙일 수 있기 때문이다. 또한 Advanced Scheduler는 priority donation과 동시에 테스트하지 않으므로, donation 구현을 확장하기보다 `thread_mlfqs` 경로에서 끄는 쪽이 문서 요구와 맞다.

## 출처 기준 정리

이 문서는 긴 직접 인용을 피하고, 아래 기준 내용을 요약해 구성했다.

| 문서 | 반영한 내용 |
| --- | --- |
| [pintos-kaist-kr/1_project1/3_advanced_scheduler.md](../reference/pintos-kaist-kr/1_project1/3_advanced_scheduler.md) | Advanced Scheduler의 목표, `-mlfqs`와 `thread_mlfqs`, priority 직접 제어 비활성화, donation 제외, `nice`, `recent_cpu`, `load_avg`, fixed-point arithmetic, 갱신 시점 |
| [pintos-kaist-kr/1_project1/4_FAQ.md](../reference/pintos-kaist-kr/1_project1/4_FAQ.md) | 하나의 queue 사용 가능성, donation과 Advanced Scheduler를 동시에 테스트하지 않는다는 점, timer interrupt 작업량 관련 주의 |
| [kaist-oslab-pintos-slides-kr/scripts/[Week02] Pintos Project1-2 BSD.md](<../reference/kaist-oslab-pintos-slides-kr/scripts/[Week02] Pintos Project1-2 BSD.md>) | Basic implementation 흐름, 수정 대상 함수, fixed-point 필요성, 4 tick/1초 단위 갱신 요약 |

