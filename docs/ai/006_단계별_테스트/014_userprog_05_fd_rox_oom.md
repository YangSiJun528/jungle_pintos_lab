# userprog 05: fd, rox, oom

이 단계는 file descriptor edge case, 실행 파일 write deny, process/resource 고갈 상황을 확인한다. 모든 명령은 Docker 컨테이너 안에서 실행한다.

## 대상 기능

- 잘못된 fd 처리
- stdin/stdout 특수 fd 처리
- fd table close와 중복 close 방어
- child에게 fd table이 의도대로 복제/분리되는지 확인
- 실행 중인 executable에 대한 write deny
- no-VM 환경의 multi-process OOM 정리

## 참고한 reference 문서

- `docs/reference/pintos-kaist-kr/2_project2/3_system_call.md`
- `docs/reference/pintos-kaist-kr/2_project2/5_deny_write.md`
- `docs/reference/pintos-kaist-kr/2_project2/4_process_termination.md`
- `pintos/tests/userprog/Make.tests`
- `pintos/tests/userprog/no-vm/Make.tests`

## 공통 준비

```bash
export PINTOS_ROOT=/workspace/pintos
source "$PINTOS_ROOT/activate"
```

## 순차 실행 명령

```bash
make -C "$PINTOS_ROOT/userprog" clean
make -C "$PINTOS_ROOT/userprog" \
  build/tests/userprog/open-twice.result \
  build/tests/userprog/close-twice.result \
  build/tests/userprog/close-bad-fd.result \
  build/tests/userprog/read-zero.result \
  build/tests/userprog/read-stdout.result \
  build/tests/userprog/read-bad-fd.result \
  build/tests/userprog/write-zero.result \
  build/tests/userprog/write-stdin.result \
  build/tests/userprog/write-bad-fd.result \
  build/tests/userprog/multi-child-fd.result \
  build/tests/userprog/rox-simple.result \
  build/tests/userprog/rox-child.result \
  build/tests/userprog/rox-multichild.result \
  build/tests/userprog/no-vm/multi-oom.result
```

## 병렬 실행 명령

```bash
make -C "$PINTOS_ROOT/userprog" clean
make -j"$(nproc)" -C "$PINTOS_ROOT/userprog" \
  build/tests/userprog/open-twice.result \
  build/tests/userprog/close-twice.result \
  build/tests/userprog/close-bad-fd.result \
  build/tests/userprog/read-zero.result \
  build/tests/userprog/read-stdout.result \
  build/tests/userprog/read-bad-fd.result \
  build/tests/userprog/write-zero.result \
  build/tests/userprog/write-stdin.result \
  build/tests/userprog/write-bad-fd.result \
  build/tests/userprog/multi-child-fd.result \
  build/tests/userprog/rox-simple.result \
  build/tests/userprog/rox-child.result \
  build/tests/userprog/rox-multichild.result \
  build/tests/userprog/no-vm/multi-oom.result
```

## 결과 확인

```bash
cat "$PINTOS_ROOT/userprog/build/tests/userprog/open-twice.result"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/close-twice.result"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/close-bad-fd.result"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/read-zero.result"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/read-stdout.result"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/read-bad-fd.result"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/write-zero.result"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/write-stdin.result"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/write-bad-fd.result"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/multi-child-fd.result"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/rox-simple.result"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/rox-child.result"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/rox-multichild.result"

cat "$PINTOS_ROOT/userprog/build/tests/userprog/no-vm/multi-oom.result"
cat "$PINTOS_ROOT/userprog/build/results"
```

실패 로그:

```bash
cat "$PINTOS_ROOT/userprog/build/tests/userprog/multi-child-fd.output"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/multi-child-fd.errors"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/no-vm/multi-oom.output"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/no-vm/multi-oom.errors"
```

## 실패 시 확인할 포인트

- fd lookup 실패 시 kernel panic이 아니라 syscall 실패값을 반환하는지 확인한다.
- `read` fd `1`, `write` fd `0` 같은 잘못된 방향의 stdio 접근을 막는지 확인한다.
- process exit에서 열린 파일을 모두 닫고 file object refcount를 정리하는지 확인한다.
- `rox-*` 실패는 `file_deny_write()`와 `file_allow_write()` 호출 시점을 확인한다.
- `multi-oom` 실패는 fork 실패 경로, page allocation 실패 경로, child metadata free 경로를 같이 본다.
