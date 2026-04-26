# [Week02] Pintos Project1-2 BSD

Source: https://youtu.be/4-OjMqyygss?si=uy9Q-H-kDi0U7rD1

## Intro

이 영상에서는 scheduler 구현의 또 다른 주제를 설명합니다. 이 강의의 제목은 4.4BSD-like scheduler입니다.

우리의 주요 목표는 4.4BSD scheduler, 즉 MLFQS, multilevel feedback queue scheduler를 구현하는 것입니다. 4.4BSD scheduler는 interactive nature를 가진 process에 priority를 줍니다. 기본적으로 priority-based scheduler입니다. priority를 계산하기 위해 equation을 사용합니다.

## Nice

BSD scheduler에서 가장 중요한 concept는 nice입니다. nice는 integer value이며 thread의 niceness를 나타냅니다. thread가 더 nice하다는 것은 자기 CPU time의 일부를 기꺼이 양보하겠다는 뜻입니다.

Pintos에서 nice value의 범위는 -20부터 20까지입니다. nice value 0은 default이며 priority에 영향을 주지 않습니다. nice가 positive이면 priority를 낮춥니다. nice가 negative이면 process의 priority를 높입니다.

nice value를 조정하고 얻기 위한 함수가 두 개 있습니다. 첫 번째는 `thread_get_nice()`이고, 두 번째는 `thread_set_nice()`입니다.

## Priority

Pintos에서 process의 priority 범위는 0부터 63까지입니다. unsigned integer이며 minimum은 0, maximum은 63입니다. 숫자가 클수록 priority가 높습니다. thread가 initialize될 때 thread의 priority는 31로 설정됩니다.

process의 priority는 다음과 같이 계산됩니다.

```text
priority = PRI_MAX - (recent_cpu / 4) - (nice * 2)
```

이 equation의 세부 사항을 설명하겠습니다. 이 equation은 매우 단순하지만 깊은 의미가 있습니다.

이 equation 뒤에는 네 가지 principle이 있습니다. 첫째, thread가 더 nice하면 priority를 낮춥니다. thread가 최근에 CPU를 많이 사용했다면 priority를 낮춥니다. 여기서 중요한 단어는 recently입니다. CPU scheduler는 process가 최근 CPU를 어떻게 사용했는지를 어떻게 고려할까요?

나중에 모든 thread에 대해 priority는 네 번째 clock tick마다 한 번씩 재계산되고, 그 결과는 nearest integer로 truncate됩니다. 이 statement는 꽤 당연하게 들리지만 그렇지 않습니다. 이유는 `PRI_MAX`는 integer이지만, `recent_cpu / 4`와 `nice`는 integer number가 아니기 때문입니다. 그것들은 floating-point number입니다. 따라서 floating-point number를 integer value로 map하는 rule이 필요하고, 결과는 nearest integer로 truncate됩니다.

## `recent_cpu`

먼저 `recent_cpu` concept를 이야기하겠습니다. 이것은 process가 CPU cycle을 얼마나 사용했는지를 나타냅니다. Timer interrupt는 매 timer interrupt마다 현재 running process의 `recent_cpu`를 1씩 증가시킵니다.

앞 slide에서 최근에 CPU를 사용하면 priority가 낮아진다고 했습니다. process가 오래전에 CPU를 많이 사용했더라도, 그 사실을 discount하는 mechanism이 필요합니다. 그래서 decay라는 concept를 가져옵니다.

Decay는 매초 `recent_cpu` 값의 양을 특정 decay factor만큼 감소시킵니다. 물론 decay는 1보다 작습니다. 우리는 매초 nice로 `recent_cpu`를 조정합니다. 그래서 매초 nice value를 `recent_cpu`에 더하고, 그것을 새 `recent_cpu` 값으로 설정합니다.

모두 합치면, `recent_cpu`는 매초 decay 곱하기 previous `recent_cpu`에 nice value를 더해 계산됩니다.

```text
recent_cpu = decay * recent_cpu + nice
```

System V Release 3에서는 decay factor가 0.5였습니다. 4.4BSD에서는 더 정교한 mechanism을 포함했습니다. heavy load에서는 CPU scheduler가 decay factor를 거의 1에 가깝게 만듭니다. light load에서는 decay factor가 0으로 converge합니다.

이 objective를 달성하기 위해 다음과 같은 매우 흥미로운 formula가 등장합니다.

```text
decay = (2 * load_avg) / (2 * load_avg + 1)
```

`load_avg`가 크면 이 값은 1로 converge합니다. `load_avg`가 거의 0이면 이 값은 0으로 converge합니다.

## `load_avg`

그렇다면 `load_avg`는 무엇일까요? `load_avg`는 system이 얼마나 busy한지를 나타냅니다. booting time에는 `load_avg`가 initially 0으로 설정됩니다.

`load_avg`는 `load_avg`와 ready threads의 weighted average입니다. Ready threads는 ready list 안의 thread 수에 update 시점에 실행 중인 thread를 더한 값입니다. 이것이 system 안 thread 수를 나타냅니다.

`load_avg`는 다음과 같이 계산됩니다.

```text
load_avg = (59 / 60) * load_avg + (1 / 60) * ready_threads
```

이 모든 값은 BSD CPU scheduler algorithm을 통해 fairness, efficiency, performance를 결정하는 데 매우 critical한 역할을 합니다. 하지만 이 값을 어떻게 설정하는지의 세부 사항까지 들어가지는 않겠습니다.

## Summary of Rules

요약하면 다음 rule들을 얻을 수 있습니다.

첫째, 네 번째 tick마다 모든 thread의 priority를 다음과 같이 recompute해야 합니다.

```text
priority = PRI_MAX - (recent_cpu / 4) - (nice * 2)
```

매 clock tick마다 running thread의 `recent_cpu`를 1 증가시킵니다.

매초 모든 thread의 `recent_cpu`를 다음과 같이 update합니다.

```text
recent_cpu = decay * recent_cpu + nice
```

decay factor와 `load_avg`는 다음과 같이 계산됩니다.

```text
decay = (2 * load_avg) / (2 * load_avg + 1)
load_avg = (59 / 60) * load_avg + (1 / 60) * ready_threads
```

## Example

예시를 들어 보겠습니다. P1, P2, P3 세 process가 있습니다. nice의 initial value는 0이고, `load_avg`의 initial value도 0입니다.

첫 번째 clock tick에서 모든 thread의 priority는 63이므로 scheduler가 process one을 선택합니다. clock tick 1, 2, 3에서 `recent_cpu` value는 1, 2, 3으로 증가합니다.

네 번째 clock tick에서 P1, P2, P3의 priority가 recalculated됩니다. `recent_cpu`가 4가 되므로 process 1의 priority는 63에서 62가 됩니다. 그러면 두 다른 process, P2와 P3의 priority와 비교합니다. 나머지 두 process는 priority 63을 얻고, 더 높은 priority를 가집니다. 그 이유로 process 2가 선택되어 실행됩니다. process 2의 `recent_cpu` value도 네 번 증가하여 0에서 1, 2, 3, 4가 됩니다.

이 시점에서 P2의 priority는 62로 update됩니다. 따라서 P3의 priority가 63이고 highest priority를 가지므로 CPU를 얻습니다.

동시에 `recent_cpu`가 어떻게 reset되는지 살펴봅시다. `recent_cpu`는 이렇게 계산됩니다.

```text
recent_cpu = (2 * load_avg) / (2 * load_avg + 1) * recent_cpu + nice
```

기본적으로 이 모든 것을 고려하면, 이 time period에서 process들의 `recent_cpu` value는 0부터 4가 됩니다. 그 결과 이 priority mechanism에 따르면 P1, P2, P3가 모두 CPU를 요구할 때 round-robin manner로 실행됩니다.

## Fixed-Point Arithmetic

해야 할 중요한 일이 하나 있습니다. fixed-point arithmetic을 구현해야 합니다. 이유는 kernel 안에서는 integer arithmetic만 할 수 있기 때문입니다. kernel은 thread context를 switch할 때 floating-point register를 갖고 있지 않습니다. 따라서 integer arithmetic을 사용해 fixed-point arithmetic을 구현해야 합니다.

Priority, nice, ready thread value는 integer입니다. 하지만 `recent_cpu`와 `load_avg` value는 real number입니다.

우리는 17.14 fixed-point number representation을 사용할 것입니다. 이 representation에서는 decimal point가 오른쪽 14 bit를 사용하고, integer는 그 왼쪽의 다음 17 bit를 사용합니다. 마지막 left bit 하나는 sign bit입니다.

모양은 이렇습니다. 총 32 bit입니다. bit 0부터 13까지는 fractional part입니다. leftmost bit는 sign을 나타냅니다. 이것은 fixed number를 나타냅니다. 이것은 integer part이고, 이것은 fractional part이며, 이것은 sign입니다.

rule은 다음과 같습니다. 이 함수들이 필요합니다.

- `n`을 fixed point로 변환합니다.
- fixed point를 integer로 변환합니다.
- `x`를 integer로 변환합니다.
- `x`와 `y`가 fixed-point number이고 `n`이 integer일 때 두 값을 더합니다.
- Subtraction.
- Addition.
- fixed-point number와 integer number 사이의 addition.
- Subtraction, multiplication, division.

이 모든 함수를 직접 구현한 다음, arithmetic을 수행할 때 적절한 함수를 사용해야 합니다.

## Basic Implementation

이것이 basic implementation입니다. 가장 먼저 해야 할 일은 `struct thread`에 nice와 `recent_cpu` field를 추가하는 것입니다.

그다음 이 모든 함수가 필요합니다. 먼저 `recent_cpu`와 nice를 사용해 priority를 계산하는 함수가 필요합니다. 또한 `recent_cpu`와 `load_avg`를 계산하는 함수도 필요합니다. `recent_cpu`를 1 증가시키는 함수가 필요합니다. 또한 모든 thread의 priority와 `recent_cpu`를 recalculate해야 합니다.

이 단순한 equation을 사용하면 multilevel feedback을 구현하기 위해 multiple queue가 필요하지 않을 수 있습니다. 이 단순한 equation-based CPU scheduler는 multilevel feedback queue와 같은 objective를 달성합니다. interactive job에 priority를 주고, I/O-intensive job에 priority를 줍니다.

## Functions to Modify

수정해야 할 함수들은 다음과 같습니다.

`init_thread()`에서는 nice value와 `recent_cpu`를 initialize해야 합니다. `thread_set_priority()`에서는 advanced scheduler를 사용할 때 priority setting을 disable해야 합니다.

timer interrupt function을 조정해야 합니다. timer interrupt function에서는 매 1초마다 `load_avg`, 모든 thread의 `recent_cpu`, 그리고 priority를 recalculate해야 합니다. 네 번째 tick마다 모든 thread의 priority를 recalculate해야 합니다.

advanced scheduler를 사용할 때는 `lock_acquire()`와 `lock_release()` 양쪽에서 priority donation을 disable하세요.

이것들이 BSD-like scheduler를 구현하기 위해 수정해야 하는 함수들입니다.

`thread_set_nice()`라는 함수가 있습니다. 이 함수는 current thread의 nice value를 설정합니다. `thread_get_nice()`라는 함수도 있습니다. 이 함수는 current thread의 nice value를 반환합니다.

`thread_get_load_avg()`를 구현합니다. 이 함수는 `load_avg`에 100을 곱한 값을 반환합니다. 또한 `thread_get_recent_cpu()`를 작성합니다. 이 함수는 `recent_cpu`에 100을 곱한 값을 반환합니다.

이 feature들을 완전히 구현하면 tests를 통과할 수 있을 것입니다.
