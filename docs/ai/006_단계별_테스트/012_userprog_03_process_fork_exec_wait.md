# userprog 03: process fork, exec, wait

이 단계는 process lifecycle 관련 syscall을 묶어서 확인한다. 모든 명령은 Docker 컨테이너 안에서 실행한다.

## 대상 기능

- `fork`의 부모/자식 컨텍스트 복제
- file descriptor table 복제와 자식 종료 정리
- `exec` 성공/실패 동기화
- `wait`의 exit status 전달, 중복 wait 방지, 잘못된 pid 처리
- recursive process 생성과 종료 정리

## 참고한 reference 문서

- `docs/reference/pintos-kaist-kr/project2/system_call.md`
- `docs/reference/pintos-kaist-kr/project2/process_termination.md`
- `docs/reference/pintos-kaist-kr/project2/user_memory.md`
- `pintos/tests/userprog/Make.tests`

## 공통 준비

```bash
export PINTOS_ROOT=/workspace/pintos
source "$PINTOS_ROOT/activate"
```

## 순차 실행 명령

```bash
make -C "$PINTOS_ROOT/userprog" clean
make -C "$PINTOS_ROOT/userprog" \
  build/tests/userprog/fork-once.result \
  build/tests/userprog/fork-multiple.result \
  build/tests/userprog/fork-recursive.result \
  build/tests/userprog/fork-read.result \
  build/tests/userprog/fork-close.result \
  build/tests/userprog/fork-boundary.result \
  build/tests/userprog/exec-once.result \
  build/tests/userprog/exec-arg.result \
  build/tests/userprog/exec-boundary.result \
  build/tests/userprog/exec-missing.result \
  build/tests/userprog/exec-bad-ptr.result \
  build/tests/userprog/exec-read.result \
  build/tests/userprog/wait-simple.result \
  build/tests/userprog/wait-twice.result \
  build/tests/userprog/wait-killed.result \
  build/tests/userprog/wait-bad-pid.result \
  build/tests/userprog/multi-recurse.result
```

## 병렬 실행 명령

```bash
make -C "$PINTOS_ROOT/userprog" clean
make -j"$(nproc)" -C "$PINTOS_ROOT/userprog" \
  build/tests/userprog/fork-once.result \
  build/tests/userprog/fork-multiple.result \
  build/tests/userprog/fork-recursive.result \
  build/tests/userprog/fork-read.result \
  build/tests/userprog/fork-close.result \
  build/tests/userprog/fork-boundary.result \
  build/tests/userprog/exec-once.result \
  build/tests/userprog/exec-arg.result \
  build/tests/userprog/exec-boundary.result \
  build/tests/userprog/exec-missing.result \
  build/tests/userprog/exec-bad-ptr.result \
  build/tests/userprog/exec-read.result \
  build/tests/userprog/wait-simple.result \
  build/tests/userprog/wait-twice.result \
  build/tests/userprog/wait-killed.result \
  build/tests/userprog/wait-bad-pid.result \
  build/tests/userprog/multi-recurse.result
```

## 결과 확인

```bash
for t in fork-once fork-multiple fork-recursive fork-read fork-close fork-boundary exec-once exec-arg exec-boundary exec-missing exec-bad-ptr exec-read wait-simple wait-twice wait-killed wait-bad-pid multi-recurse; do
  base="$PINTOS_ROOT/userprog/build/tests/userprog/$t"
  printf '\n== %s.result ==\n' "$t"
  cat "$base.result"
done

cat "$PINTOS_ROOT/userprog/build/results" 2>/dev/null || true
```

실패한 테스트의 실행 로그를 본다.

```bash
t=exec-once
base="$PINTOS_ROOT/userprog/build/tests/userprog/$t"
cat "$base.output"
cat "$base.errors"
```

## 실패 시 확인할 포인트

- `fork`에서 trap frame, page table, fd table, current working state가 필요한 만큼 복제되는지 확인한다.
- `exec` 실패를 부모가 정확히 알 수 있도록 load 완료 semaphore 또는 equivalent 동기화가 있는지 확인한다.
- `wait`가 child가 아닌 pid, 이미 wait한 pid, kill된 child를 구분하는지 확인한다.
- parent exit 이후 child metadata가 누수되거나 premature free되지 않는지 확인한다.
- `multi-recurse` timeout은 process cleanup, wait list 정리, fd close 누락을 우선 의심한다.
