### Advanced Scheduler (고급 스케줄러)

**system에서 실행되는 job(작업)의 평균 response time(응답 시간)을 줄이기 위해 [4.4BSD scheduler](#4.4BSD%20Scheduler)와 비슷한 multilevel feedback queue scheduler(다단계 피드백 큐 스케줄러)를 구현하세요.**

priority scheduler(우선순위 스케줄러)와 마찬가지로 advanced scheduler는 priority(우선순위)를 기준으로 실행할 thread(실행 흐름)를 선택합니다. 하지만 advanced scheduler는 priority donation(우선순위 기부)을 하지 않습니다. 따라서 advanced scheduler 작업을 시작하기 전에, priority donation은 제외하더라도 priority scheduler가 동작하도록 만들어 두는 것을 권장합니다.

Pintos startup time(시작 시점)에 scheduling algorithm policy(스케줄링 알고리즘 정책)를 선택할 수 있도록 code(코드)를 작성해야 합니다. 기본적으로는 priority scheduler가 활성화되어야 하지만, `-mlfqs` kernel option(커널 옵션)을 통해 4.4BSD scheduler를 선택할 수 있어야 합니다. 이 option을 전달하면 option이 `parse_options()`에서 parse(해석)될 때 `threads/thread.h`에 선언된 `thread_mlfqs`가 true로 설정됩니다. 이 parsing은 `main()` 초기에 발생합니다.

4.4BSD scheduler가 enabled(활성화)되면 thread는 더 이상 자신의 priority를 직접 control(제어)하지 않습니다. `thread_create()`의 priority argument(인자)는 무시되어야 하고, `thread_set_priority()` 호출도 무시되어야 하며, `thread_get_priority()`는 scheduler가 설정한 thread의 current priority(현재 우선순위)를 반환해야 합니다. advanced scheduler는 이후 project에서는 사용되지 않습니다.

### 4.4BSD Scheduler

general-purpose scheduler(범용 스케줄러)의 목표는 thread들의 서로 다른 scheduling need(실행 배정 요구)를 균형 있게 맞추는 것입니다. I/O(입출력)를 많이 수행하는 thread는 input/output device(입출력 장치)를 바쁘게 유지하기 위해 빠른 response time이 필요하지만 CPU time(프로세서 시간)은 거의 필요하지 않습니다. 반대로 compute-bound thread(계산 중심 스레드)는 작업을 끝내기 위해 많은 CPU time을 받아야 하지만 빠른 response time은 필요하지 않습니다. 다른 thread들은 그 사이 어딘가에 있으며, I/O 기간과 computation(계산) 기간이 번갈아 나타나므로 요구사항도 시간에 따라 달라집니다. 잘 설계된 scheduler는 이런 요구사항을 가진 thread들을 동시에 수용할 수 있는 경우가 많습니다.

project 1에서는 이 appendix(부록)에 설명된 scheduler를 구현해야 합니다. 강사진의 scheduler는 [McKusick]에 설명된 scheduler와 비슷하며, multilevel feedback queue scheduler의 한 예입니다. 이런 scheduler는 실행 준비가 된 thread들의 queue(큐)를 여러 개 유지합니다. 각 queue는 서로 다른 priority를 가진 thread들을 담습니다. 어떤 시점이든 scheduler는 비어 있지 않은 queue 중 가장 높은 priority의 queue에서 thread를 선택합니다. 가장 높은 priority queue에 여러 thread가 있으면 "round robin(순환 방식)" 순서로 실행됩니다.

scheduler의 여러 부분은 일정한 timer tick(타이머 틱) 수가 지난 뒤 data(데이터)를 update(갱신)해야 합니다. 모든 경우에 이러한 update는 ordinary kernel thread(일반 커널 스레드)가 실행될 기회를 얻기 전에 발생해야 합니다. 그래야 kernel thread가 새로 증가한 `timer_ticks()` 값은 보지만 scheduler data value(스케줄러 데이터 값)는 예전 값을 보는 일이 없습니다.

4.4BSD scheduler는 priority donation을 포함하지 않습니다.

#### Niceness (나이스 값)

thread priority는 아래에 제시된 formula(공식)를 사용하여 scheduler가 dynamic(동적으로) 결정합니다. 하지만 각 thread에는 그 thread가 다른 thread에게 얼마나 "nice(양보적)"해야 하는지를 결정하는 integer nice value(정수 나이스 값)도 있습니다. nice가 0이면 thread priority에 영향을 주지 않습니다. 양수 nice는 최대 20까지 thread의 priority를 낮추고, 원래 받을 수 있었던 CPU time 일부를 포기하게 합니다. 반대로 음수 nice는 최소 -20까지 다른 thread에게서 CPU time을 가져오는 경향이 있습니다.

initial thread(초기 스레드)는 nice value 0으로 시작합니다. 다른 thread들은 parent thread(부모 스레드)에서 nice value를 inherit(상속)받아 시작합니다. test program(테스트 프로그램)이 사용할 아래 함수들을 구현해야 합니다. `threads/thread.c`에 skeleton definition(골격 정의)을 제공했습니다.
    
    
    int thread_get_nice (void);
    

> 현재 thread의 nice value를 반환합니다.
    
    
    int thread_set_nice (int nice);
    

> 현재 thread의 nice value를 새 nice로 설정하고, 새 값을 바탕으로 thread priority를 다시 계산합니다([Calculating Priority](#Calculating%20Priority) 참조). 실행 중인 thread가 더 이상 가장 높은 priority가 아니라면 yield(양보)합니다.

#### Calculating Priority (Priority 계산)

강사진의 scheduler는 64개의 priority와 따라서 64개의 ready queue(준비 큐)를 가지며, 번호는 0 `(PRI_MIN)`부터 `63 (PRI_MAX)`까지입니다. 낮은 숫자가 낮은 priority에 대응하므로 priority 0이 가장 낮고 priority 63이 가장 높습니다. thread priority는 thread initialization(스레드 초기화) 시 처음 계산됩니다. 또한 매 네 번째 clock tick(클록 틱)마다 모든 thread에 대해 다시 계산됩니다. 두 경우 모두 다음 formula로 결정됩니다.

> priority = PRI_MAX - (recent_cpu / 4) - (nice * 2),

여기서 recent cpu는 thread가 최근에 사용한 CPU time의 estimate(추정치)이고(아래 참조), nice는 thread의 nice value입니다. 결과는 가장 가까운 integer로 내림(round down, truncate)해야 합니다. recent cpu와 nice에 붙은 coefficient(계수) 1/4와 2는 실제로 잘 동작하는 것으로 확인되었지만 더 깊은 의미는 없습니다. 계산된 priority는 항상 valid range(유효 범위)인 `PRI_MIN`부터 `PRI_MAX` 안에 있도록 조정됩니다.

이 formula는 최근에 CPU time을 받은 thread에게 scheduler가 다음에 실행될 때 더 낮은 priority를 부여합니다. 이것이 starvation(기아)을 막는 핵심입니다. 최근에 CPU time을 전혀 받지 못한 thread는 recent cpu가 0이므로, 높은 nice value가 없다면 곧 CPU time을 받을 수 있어야 합니다.

#### Calculating `recent_cpu` (`recent_cpu` 계산)

recent cpu는 각 process(프로세스)가 "recently(최근에)" 얼마나 많은 CPU time을 받았는지를 측정하기 위한 값입니다. 더 나아가 더 최근의 CPU time이 덜 최근의 CPU time보다 더 큰 weight(가중치)를 가져야 합니다. 한 가지 접근은 n개 element(원소)를 가진 array(배열)를 사용하여 마지막 n초 각각에서 받은 CPU time을 추적하는 것입니다. 하지만 이 접근은 thread마다 O(n) space(공간)를 요구하고, 새 weighted average(가중 평균)를 계산할 때마다 O(n) time(시간)을 요구합니다.

대신 exponentially weighted moving average(지수 가중 이동 평균)를 사용합니다. 일반적인 형태는 다음과 같습니다.

> x(0) = f(0),
> 
> x(t) = ax(t - 1) + (1 - a)f(t),
> 
> a = k/(k + 1),

여기서 `x(t)`는 integer time(정수 시각) `t >= 0`에서의 moving average(이동 평균), `f(t)`는 평균을 내는 function, `k > 0`은 decay rate(감쇠율)를 조절하는 값입니다. 이 formula를 몇 단계 반복하면 다음과 같습니다.

> x(1) = f(1),
> 
> x(2) = af(1) + f(2),
> 
> ...
> 
> x(5) = a^4 * f(1) + a^3 * f(2) + a^2 * f(3) + a * f(4) + f(5).

`f(t)`의 값은 시각 `t`에서 weight 1을 가지고, 시각 `t + 1`에서는 weight `a`, 시각 `t + 2`에서는 `a^2`를 가지며, 이런 식으로 이어집니다. `x(t)`를 `k`와 관련지어 볼 수도 있습니다. `f(t)`는 시각 `t + k`에서 대략 `1/e`의 weight를, 시각 `t + 2k`에서 대략 `1/(e^2)`의 weight를 가집니다. 반대 방향으로 보면, `f(t)`는 시각 `t + log_a(w)`에서 weight `w`로 decay합니다.

`recent_cpu`의 initial value(초기값)는 처음 생성된 thread에서는 0이고, 다른 새 thread에서는 parent의 값입니다. timer interrupt(타이머 인터럽트)가 발생할 때마다, idle thread가 실행 중인 경우를 제외하고 실행 중인 thread에 대해서만 `recent_cpu`가 1 증가합니다. 또한 초당 한 번씩, 실행 중이든 ready 상태든 blocked(차단) 상태든 모든 thread에 대해 recent cpu 값이 다음 formula로 다시 계산됩니다.

> recent_cpu = (2 * load_avg)/(2 * load_avg + 1) * recent_cpu + nice

여기서 `load_avg`는 실행 준비가 된 thread 수의 moving average입니다(아래 참조). `load_avg`가 1이면 평균적으로 단일 thread가 CPU를 두고 경쟁한다는 뜻입니다. 이 경우 recent cpu의 현재 값은 `log_(2/3) .1 ~= 6`초 뒤 weight `.1`로 decay합니다. load avg가 2이면 `.1`까지 decay하는 데 `log_(3/4) .1 ~= 8`초가 걸립니다. 그 효과는 recent cpu가 thread가 "recently" 받은 CPU time의 양을 estimate하고, decay rate가 CPU를 두고 경쟁하는 thread 수에 반비례하도록 만드는 것입니다.

일부 test의 assumption(가정) 때문에, recent cpu의 이러한 재계산은 system tick counter(시스템 틱 카운터)가 1초의 배수에 도달할 때, 즉 `timer_ticks () % TIMER_FREQ == 0`일 때 정확히 수행되어야 하며, 다른 시각에는 수행되면 안 됩니다.

negative nice value(음수 나이스 값)를 가진 thread에서는 `recent_cpu` 값이 negative(음수)가 될 수 있습니다. negative `recent_cpu`를 0으로 clamp(고정)하지 마세요.

이 formula의 계산 순서에 대해 생각해야 할 수 있습니다. recent cpu의 coefficient를 먼저 계산한 뒤 곱하는 것을 권장합니다. 일부 학생들은 `load_avg`에 `recent_cpu`를 직접 곱하면 overflow(오버플로)가 발생할 수 있다고 보고했습니다.

`threads/thread.c`에 skeleton이 있는 `thread_get_recent_cpu()`를 구현해야 합니다.
    
    
    int thread_get_recent_cpu (void);
    

> 현재 thread의 recent cpu 값에 100을 곱한 값을 가장 가까운 integer로 round(반올림)하여 반환합니다.

#### Calculating `load_avg` (`load_avg` 계산)

마지막으로 load avg, 흔히 system load average(시스템 부하 평균)라고 부르는 값은 지난 1분 동안 실행 준비가 된 thread의 평균 수를 estimate합니다. `recent_cpu`와 마찬가지로 exponentially weighted moving average입니다. priority와 `recent_cpu`와 달리 `load_avg`는 system-wide(시스템 전체) 값이며 thread-specific(스레드별) 값이 아닙니다. system boot(시스템 부팅) 시 0으로 초기화됩니다. 이후 초당 한 번씩 다음 formula에 따라 update됩니다.

> load_avg = (59/60) * load_avg + (1/60) * ready_threads,

여기서 ready threads는 update 시점에 실행 중이거나 실행 준비가 된 thread 수입니다(idle thread는 포함하지 않음).

일부 test의 assumption 때문에, `load_avg`는 system tick counter가 1초의 배수에 도달할 때, 즉 `timer_ticks () % TIMER_FREQ == 0`일 때 정확히 update되어야 하며, 다른 시각에는 update되면 안 됩니다. `threads/thread.c`에 skeleton이 있는 `thread_get_load_avg()`를 구현해야 합니다.
    
    
    int thread_get_load_avg (void)
    

> current system load average(현재 시스템 부하 평균)에 100을 곱한 값을 가장 가까운 integer로 round하여 반환합니다.

#### Summary (요약)

다음 formula들은 scheduler를 구현하는 데 필요한 계산을 요약합니다. scheduler requirement(요구사항)의 완전한 설명은 아닙니다.

모든 thread는 자신이 직접 control하는 -20에서 20 사이의 nice value를 가집니다. 또한 각 thread는 0(`PRI_MIN`)부터 63(`PRI_MAX`) 사이의 priority를 가지며, 이 priority는 매 네 번째 tick마다 다음 formula로 다시 계산됩니다.

> priority = PRI_MAX - (recent_cpu / 4) - (nice * 2)

`recent_cpu`는 thread가 "recently" 받은 CPU time의 양을 측정합니다. 각 timer tick마다 실행 중인 thread의 recent cpu가 1 증가합니다. 초당 한 번, 모든 thread의 recent cpu는 다음과 같이 update됩니다.

> recent_cpu = (2 * load_avg) / (2 * load_avg + 1) * recent_cpu + nice

`load_avg`는 지난 1분 동안 실행 준비가 된 thread의 평균 수를 estimate합니다. boot 시 0으로 초기화되며 초당 한 번 다음과 같이 다시 계산됩니다.

> load_avg = (59/60) * load_avg + (1/60) * ready_threads

여기서 `ready_threads`는 update 시점에 실행 중이거나 실행 준비가 된 thread 수입니다(idle thread는 포함하지 않음).

#### Fixed-Point Real Arithmetic (고정소수점 실수 연산)

위 formula에서 `priority`, `nice`, `ready_threads`는 integer이지만, `recent_cpu`와 `load_avg`는 real number(실수)입니다. 안타깝게도 Pintos는 kernel에서 floating-point arithmetic(부동소수점 연산)을 지원하지 않습니다. 이는 kernel을 복잡하고 느리게 만들기 때문입니다. 실제 kernel도 같은 이유로 이런 제한을 갖는 경우가 많습니다. 즉, real quantity(실수 값)에 대한 계산을 integer를 사용해 simulate(모사)해야 합니다. 어렵지는 않지만, 많은 학생이 방법을 모릅니다. 이 section은 기본을 설명합니다.

기본 아이디어는 integer의 오른쪽 끝 bit들을 fraction(소수 부분)을 나타내는 데 사용하는 것입니다. 예를 들어 signed 32-bit integer(부호 있는 32비트 정수)의 가장 낮은 14 bit를 fractional bit(소수 비트)로 지정하면, integer x는 real number `x/(2^14)`를 나타냅니다. 이를 17.14 fixed-point number representation(고정소수점 수 표현)이라고 합니다. decimal point(소수점) 앞에 17 bit, 뒤에 14 bit, 그리고 sign bit(부호 비트) 하나가 있기 때문입니다. 17.14 format의 수가 표현할 수 있는 최대값은 `(2^31 - 1)/(2^14) ~= 131,071.999`입니다.

`p.q` fixed-point format을 사용하고 `f = 2^q`라고 합시다. 위 정의에 따르면 integer나 real number에 `f`를 곱하여 `p.q` format으로 변환할 수 있습니다. 예를 들어 17.14 format에서 위의 `load_avg` 계산에 사용되는 fraction `59/60`은 `(59/60)2^14 = 16,110`입니다. fixed-point value를 다시 integer로 변환하려면 `f`로 나눕니다. C의 일반 `/` operator(연산자)는 0을 향해 round합니다. 즉, 양수는 내리고 음수는 올립니다. 가장 가까운 값으로 round하려면 나누기 전에 양수에는 `f / 2`를 더하고, 음수에는 빼세요.

fixed-point number에 대한 많은 operation은 단순합니다. `x`와 `y`가 fixed-point number이고 `n`이 integer라고 합시다. 그러면 `x`와 `y`의 합은 `x + y`, 차는 `x - y`입니다. `x`와 `n`의 합은 `x + n * f`, 차는 `x - n * f`, 곱은 `x * n`, 몫은 `x / n`입니다.

두 fixed-point value를 곱하는 데에는 두 가지 복잡한 점이 있습니다. 첫째, 결과의 decimal point가 q bit만큼 너무 왼쪽에 있게 됩니다. `(59/60)(59/60)`은 `1`보다 약간 작아야 하지만, `16,111 x 16,111 = 259,564,321`은 `2^14 = 16,384`보다 훨씬 큽니다. q bit만큼 오른쪽으로 shift(이동)하면 `259,564,321/2^14 = 15,842`, 즉 약 `0.97`이 되어 올바른 답이 됩니다. 둘째, 답을 표현할 수 있더라도 multiplication(곱셈) 중 overflow가 발생할 수 있습니다. 예를 들어 17.14 format에서 `64`는 `64 x 2^14 = 1,048,576`이고 그 제곱 `64^2 = 4,096`은 17.14 range 안에 충분히 들어갑니다. 하지만 `1,048,576^2 = 2^40`은 signed 32-bit integer 최대값 `2^31 - 1`보다 큽니다. 쉬운 해결책은 multiplication을 64-bit operation으로 수행하는 것입니다. 그러면 x와 y의 product(곱)는 `((int64_t) x) * y / f`입니다.

두 fixed-point value의 division(나눗셈)에는 반대 문제가 있습니다. decimal point가 너무 오른쪽에 있게 되므로, division 전에 dividend(피제수)를 q bit만큼 왼쪽으로 shift하여 고칩니다. left shift(왼쪽 이동)는 dividend의 위쪽 q bit를 버리므로, 다시 64 bit로 division을 수행하여 해결할 수 있습니다. 따라서 x를 y로 나눌 때 quotient(몫)는 `((int64_t) x) * f / y`입니다.

이 section에서는 q-bit shift 대신 `f`로 multiplication이나 division을 일관되게 사용했습니다. 이유는 두 가지입니다. 첫째, multiplication과 division은 C shift operator의 의외의 operator precedence(연산자 우선순위)를 갖지 않습니다. 둘째, multiplication과 division은 negative operand(음수 피연산자)에 대해 잘 정의되어 있지만, C shift operator는 그렇지 않습니다. 구현할 때 이 문제들에 주의하세요.

다음 table(표)은 fixed-point arithmetic operation을 C에서 구현하는 방법을 요약합니다. 표에서 `x`와 `y`는 fixed-point number, `n`은 integer, fixed-point number는 `p + q = 31`인 signed `p.q` format이고, `f`는 `1 << q`입니다.

| Arithmetic | C |
| --- | --- |
| Convert `n` to fixed point | `n * f` |
| Convert `x` to integer (rounding toward zero) | `x / f` |
| Convert `x` to integer (rounding to nearest) | `(x + f / 2) / f` if `x >= 0`<br>`(x - f / 2) / f` if `x <= 0` |
| Add `x` and `y` | `x + y` |
| Subtract `y` from `x` | `x - y` |
| Add `x` and `n` | `x + n * f` |
| Subtract `n` from `x` | `x - n * f` |
| Multiply `x` by `y` | `((int64_t) x) * y / f` |
| Multiply `x` by `n` | `x * n` |
| Divide `x` by `y` | `((int64_t) x) * f / y` |
| Divide `x` by `n` | `x / n` |
