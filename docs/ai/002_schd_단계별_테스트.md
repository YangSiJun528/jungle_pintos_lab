# schd 단계별 테스트 스크립트

이 문서는 `TESTING.md`의 실행 방식을 기준으로, Project 1 scheduler 관련 테스트만 단계별로 따로 실행하기 위한 복붙용 명령을 모은 것이다. 아래 명령은 Docker 컨테이너 안에서 실행한다.

## 공통 준비

```bash
export PINTOS_ROOT=/workspace/pintos
source "$PINTOS_ROOT/activate"
```

## schd-1: ready list 정렬과 preemption

대상 기능:

- `ready_list`를 priority 순서로 유지
- 같은 priority는 round-robin 순서 유지
- 새 thread가 ready list에 들어간 뒤 더 높으면 yield
- `thread_set_priority()` 이후 필요하면 yield
- alarm wakeup 이후 higher-priority thread가 먼저 실행

실행:

```bash
make -C "$PINTOS_ROOT/threads" clean
make -C "$PINTOS_ROOT/threads" \
  build/tests/threads/alarm-priority.result \
  build/tests/threads/priority-preempt.result \
  build/tests/threads/priority-change.result \
  build/tests/threads/priority-fifo.result
```

결과 확인:

```bash
cat "$PINTOS_ROOT/threads/build/tests/threads/alarm-priority.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-preempt.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-change.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-fifo.result"
```

## schd-2: semaphore와 condition waiters 정렬

대상 기능:

- `sema_down()`에서 waiters를 priority 순서로 유지
- `sema_up()`에서 highest-priority waiter를 깨움
- `cond_wait()`에서 condition waiters를 priority 순서로 유지
- `cond_signal()`에서 highest-priority waiter를 깨움
- `cond_signal()`은 내부 `sema_up()` 경로로 preemption 처리

실행:

```bash
make -C "$PINTOS_ROOT/threads" clean
make -C "$PINTOS_ROOT/threads" \
  build/tests/threads/priority-sema.result \
  build/tests/threads/priority-condvar.result
```

결과 확인:

```bash
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-sema.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-condvar.result"
```

## schd-1 + schd-2 누적 확인

schd-2 구현 뒤에는 schd-1 테스트가 깨지지 않았는지도 같이 확인한다.

실행:

```bash
make -C "$PINTOS_ROOT/threads" clean
make -C "$PINTOS_ROOT/threads" \
  build/tests/threads/alarm-priority.result \
  build/tests/threads/priority-preempt.result \
  build/tests/threads/priority-change.result \
  build/tests/threads/priority-fifo.result \
  build/tests/threads/priority-sema.result \
  build/tests/threads/priority-condvar.result
```

결과 확인:

```bash
cat "$PINTOS_ROOT/threads/build/tests/threads/alarm-priority.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-preempt.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-change.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-fifo.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-sema.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-condvar.result"
```

## schd-3: priority donation

대상 기능:

- lock holder에게 priority donation
- multiple donation
- nested donation
- donation 해제 후 priority 복구
- semaphore에 block된 lock holder donation

실행:

```bash
make -C "$PINTOS_ROOT/threads" clean
make -C "$PINTOS_ROOT/threads" \
  build/tests/threads/priority-donate-one.result \
  build/tests/threads/priority-donate-multiple.result \
  build/tests/threads/priority-donate-multiple2.result \
  build/tests/threads/priority-donate-nest.result \
  build/tests/threads/priority-donate-sema.result \
  build/tests/threads/priority-donate-lower.result \
  build/tests/threads/priority-donate-chain.result
```

결과 확인:

```bash
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-donate-one.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-donate-multiple.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-donate-multiple2.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-donate-nest.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-donate-sema.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-donate-lower.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-donate-chain.result"
```

## MLFQS: advanced scheduler

MLFQS는 priority donation과 같이 테스트하지 않는다. `thread_mlfqs`가 켜진 별도 scheduler 경로로 확인한다.

실행:

```bash
make -C "$PINTOS_ROOT/threads" clean
make -C "$PINTOS_ROOT/threads" \
  build/tests/threads/mlfqs/mlfqs-load-1.result \
  build/tests/threads/mlfqs/mlfqs-load-60.result \
  build/tests/threads/mlfqs/mlfqs-load-avg.result \
  build/tests/threads/mlfqs/mlfqs-recent-1.result \
  build/tests/threads/mlfqs/mlfqs-fair-2.result \
  build/tests/threads/mlfqs/mlfqs-fair-20.result \
  build/tests/threads/mlfqs/mlfqs-nice-2.result \
  build/tests/threads/mlfqs/mlfqs-nice-10.result \
  build/tests/threads/mlfqs/mlfqs-block.result
```

결과 확인:

```bash
cat "$PINTOS_ROOT/threads/build/tests/threads/mlfqs/mlfqs-load-1.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/mlfqs/mlfqs-load-60.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/mlfqs/mlfqs-load-avg.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/mlfqs/mlfqs-recent-1.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/mlfqs/mlfqs-fair-2.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/mlfqs/mlfqs-fair-20.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/mlfqs/mlfqs-nice-2.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/mlfqs/mlfqs-nice-10.result"
cat "$PINTOS_ROOT/threads/build/tests/threads/mlfqs/mlfqs-block.result"
```

## 실패했을 때 output 확인

`.result`가 `FAIL`이면 같은 이름의 `.output`을 확인한다.

```bash
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-preempt.output"
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-change.output"
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-sema.output"
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-condvar.output"
cat "$PINTOS_ROOT/threads/build/tests/threads/priority-donate-sema.output"
cat "$PINTOS_ROOT/threads/build/tests/threads/mlfqs/mlfqs-recent-1.output"
```

## 참고

- 각 단계 블록은 stale result를 피하려고 먼저 `make clean`을 실행한다.
- `.result` 파일 내용이 `PASS`면 해당 테스트가 통과한 것이다.
- 실행 도중 컴파일 실패, kernel panic, timeout이 나면 `.result`가 없을 수 있다.
- `make check` 전체 대신 필요한 단계 target만 명시해서 빠르게 확인한다.
