# Pintos ASSERT 활용 발표 정리

1분 발표에서 말하고 싶은 핵심은 간단하다. Pintos에서는 `ASSERT`를 더 적극적으로 써도 된다. 일반적인 C `assert`처럼 조건을 검증하는 역할을 하지만, Pintos의 `ASSERT`는 실패하면 kernel panic과 함께 실패한 조건식, 파일/라인/함수, call stack을 같이 출력한다. 그래서 단순히 "죽었다"가 아니라 "어떤 계약이 어디서 깨졌는지"를 바로 볼 수 있다. 다만 `ASSERT`는 검증용이어야 한다. 실제로 실행되어야 하는 로직이나 side effect를 `ASSERT` 안에 넣으면 안 된다.

## 1분 발표 스크립트

제가 추천하고 싶은 디버깅 습관은 Pintos에서 `ASSERT`를 더 자주 쓰는 것입니다.

Pintos의 `ASSERT`는 조건이 깨지면 kernel panic을 내고, 실패한 조건식과 파일, 라인, 함수, 그리고 call stack까지 보여줍니다. 그래서 `printf`로 위치를 좁히는 것보다 "내가 믿고 있던 조건이 어디서 깨졌는지"를 훨씬 빨리 알 수 있습니다.

특히 함수 시작 부분에서 선조건을 확인하는 데 좋습니다. 예를 들어 lock 함수라면 `lock != NULL`, interrupt handler 안에서 호출되면 안 되는 함수라면 `!intr_context ()`, interrupt가 꺼져 있어야 하는 코드라면 `intr_get_level () == INTR_OFF` 같은 조건을 바로 적을 수 있습니다.

주의할 점도 있습니다. `ASSERT` 안에는 실행되어야 하는 로직을 넣으면 안 됩니다. `ASSERT (some_function ())`처럼 쓰면 디버그 설정에 따라 그 함수가 아예 실행되지 않을 수 있습니다. 그러니까 `ASSERT`는 상태를 바꾸는 코드가 아니라, 이미 계산된 상태를 검증하는 코드로 써야 합니다.

정리하면, Pintos에서 `ASSERT`는 "이 함수는 이런 상태에서만 호출되어야 한다"는 계약을 코드에 박아 두는 도구입니다. 테스트가 실패한 뒤에 추측하는 시간을 줄여 주기 때문에, 저는 선조건과 lock/interrupt 상태 검증에는 적극적으로 쓰는 걸 추천합니다.

## 왜 Pintos ASSERT인가

Pintos의 `ASSERT`는 `<debug.h>`에 정의되어 있다.

```c
#ifndef NDEBUG
#define ASSERT(CONDITION)                                       \
	if ((CONDITION)) { } else {                             \
		PANIC ("assertion `%s' failed.", #CONDITION);   \
	}
#else
#define ASSERT(CONDITION) ((void) 0)
#endif
```

조건이 false면 `PANIC()`으로 이어지고, kernel panic은 대략 다음 정보를 출력한다.

- 실패한 파일과 라인
- 실패한 함수 이름
- 실패한 assertion 조건식
- call stack

즉, `ASSERT`는 단순한 방어 코드라기보다 디버깅용 "계약 검사"에 가깝다. Pintos 공식 문서도 함수 시작 부분에서 인자 유효성을 검사하고, 의심되는 지점이나 loop invariant를 확인하는 데 assertion을 쓰라고 설명한다.

## 내 코드에서 추천할 만한 사용 패턴

### 1. 함수 선조건 검사

```c
void
lock_acquire (struct lock *lock) {
	ASSERT (lock != NULL);
	ASSERT (!intr_context ());
	ASSERT (!lock_held_by_current_thread (lock));

	...
}
```

이 함수는 아무 때나 호출해도 되는 함수가 아니다. lock 포인터가 유효해야 하고, interrupt context가 아니어야 하며, 같은 lock을 이미 들고 있으면 안 된다. 이런 조건은 함수 본문을 읽기 전에 바로 드러나야 한다.

### 2. interrupt 상태 검사

```c
void
thread_block (void) {
	ASSERT (!intr_context ());
	ASSERT (intr_get_level () == INTR_OFF);

	thread_current ()->status = THREAD_BLOCKED;
	schedule ();
}
```

스케줄러 코드는 interrupt 상태가 틀리면 재현하기 어려운 버그로 이어진다. 이럴 때 `ASSERT`로 "이 함수는 interrupt가 꺼진 상태에서만 호출된다"는 전제를 박아 두면 문제 위치가 빨리 좁혀진다.

### 3. interrupt handler 전용 함수 검사

```c
void
threads_wakeup (int64_t ticks) {
	ASSERT (intr_context ());
	ASSERT (intr_get_level () == INTR_OFF);

	...
}
```

반대로 이 함수는 timer interrupt handler에서 호출되는 쪽이다. 그래서 `!intr_context ()`가 아니라 `intr_context ()`가 선조건이다. 이런 식으로 `ASSERT`는 "이 함수가 어느 문맥에서 호출되어야 하는지"를 코드로 설명한다.

### 4. lock 소유권 검사

```c
void
lock_release (struct lock *lock) {
	ASSERT (lock != NULL);
	ASSERT (lock_held_by_current_thread (lock));

	...
}
```

`lock_release()`는 현재 thread가 가진 lock만 release해야 한다. 이 조건이 깨지면 이후 list나 semaphore 쪽에서 이상한 증상으로 터질 수 있는데, `ASSERT`를 앞에 두면 원인 위치에서 바로 멈춘다.

## 잘 쓰는 기준

좋은 `ASSERT`는 "이 지점에서 반드시 참이어야 하는 조건"을 표현한다.

- 함수 인자: `ASSERT (lock != NULL);`
- 호출 문맥: `ASSERT (!intr_context ());`
- interrupt 상태: `ASSERT (intr_get_level () == INTR_OFF);`
- lock 소유권: `ASSERT (lock_held_by_current_thread (lock));`
- priority 범위: `ASSERT (PRI_MIN <= priority && priority <= PRI_MAX);`
- list 순회 불변식: "이 포인터는 NULL이면 안 된다", "이 element는 interior여야 한다" 같은 조건

나쁜 `ASSERT`는 실행되어야 하는 일을 `ASSERT` 안에 숨긴다.

```c
/* 나쁜 예: 함수 호출 자체가 필요한 로직이면 ASSERT 안에 넣으면 안 된다. */
ASSERT (try_acquire_resource ());
```

대신 이렇게 분리한다.

```c
bool ok = try_acquire_resource ();
ASSERT (ok);
```

단, 이 경우에도 `try_acquire_resource()`가 실제 로직이라면 `ASSERT`와 별개로 실패 처리를 해야 한다.

```c
bool ok = try_acquire_resource ();
ASSERT (ok);
if (!ok)
	return false;
```

요점은 `ASSERT`가 프로그램의 필수 동작을 대신하면 안 된다는 것이다. `ASSERT`는 검증이고, 실제 처리 로직은 별도로 있어야 한다.

## 시연 대신 보여줄 만한 결과

실제 발표에서 시연 시간이 없다면, 아래처럼 "실패했을 때 이런 식으로 나온다"는 예시를 보여주면 충분하다.

```text
Kernel PANIC at ../../threads/synch.c:237 in lock_acquire():
assertion `!lock_held_by_current_thread (lock)' failed.
Call stack: 0x8004202fb 0x80042dc22 0x80042cf67 ...
The `backtrace' program can make call stacks useful.
```

이 출력에서 볼 포인트는 세 가지다.

- `synch.c:237`: 실패 위치
- `lock_acquire()`: 깨진 함수
- `!lock_held_by_current_thread (lock)`: 깨진 조건

여기에 call stack까지 있으므로, 어떤 경로로 잘못된 `lock_acquire()`가 호출됐는지 추적할 수 있다. 발표에서는 "printf를 여러 개 심어서 범위를 좁히는 대신, ASSERT가 깨진 계약과 호출 경로를 바로 보여준다"고 설명하면 된다.

## 발표용 한 줄 결론

Pintos에서 `ASSERT`는 단순히 프로그램을 죽이는 코드가 아니라, 함수의 선조건과 불변식을 문서화하고 실패 시 backtrace까지 남겨 주는 디버깅 도구다. 많이 쓰되, side effect 없는 순수 검증식으로만 쓰자.

## 참고 자료

- [Pintos KAIST Debugging Tools](../reference/pintos-kaist-kr/5_appendix/5_debugging_tools.md)
- [Pintos `ASSERT` 구현](../../pintos/include/lib/debug.h)
- [Pintos `debug_panic()` 구현](../../pintos/lib/kernel/debug.c)
- [Assert 사용 시 주의해야 할 점](https://blog.popekim.com/ko/2025/02/11/assert-mistakes.html)
