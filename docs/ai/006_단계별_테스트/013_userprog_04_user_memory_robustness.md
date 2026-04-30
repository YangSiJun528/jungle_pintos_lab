# userprog 04: user memory robustness

이 단계는 잘못된 user address가 들어왔을 때 kernel이 죽지 않고 해당 user process만 종료하는지 확인한다. 모든 명령은 Docker 컨테이너 안에서 실행한다.

## 대상 기능

- user pointer 검증
- page boundary를 걸친 버퍼 처리
- null pointer, kernel address, unmapped address 방어
- 잘못된 instruction pointer나 read/write 대상 처리

## 참고한 reference 문서

- `docs/reference/pintos-kaist-kr/project2/user_memory.md`
- `docs/reference/pintos-kaist-kr/project2/system_call.md`
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
  build/tests/userprog/create-null.result \
  build/tests/userprog/create-bad-ptr.result \
  build/tests/userprog/create-bound.result \
  build/tests/userprog/open-null.result \
  build/tests/userprog/open-bad-ptr.result \
  build/tests/userprog/open-boundary.result \
  build/tests/userprog/read-bad-ptr.result \
  build/tests/userprog/read-boundary.result \
  build/tests/userprog/write-bad-ptr.result \
  build/tests/userprog/write-boundary.result \
  build/tests/userprog/exec-bad-ptr.result \
  build/tests/userprog/exec-boundary.result \
  build/tests/userprog/fork-boundary.result \
  build/tests/userprog/bad-read.result \
  build/tests/userprog/bad-write.result \
  build/tests/userprog/bad-read2.result \
  build/tests/userprog/bad-write2.result \
  build/tests/userprog/bad-jump.result \
  build/tests/userprog/bad-jump2.result
```

## 병렬 실행 명령

```bash
make -C "$PINTOS_ROOT/userprog" clean
make -j"$(nproc)" -C "$PINTOS_ROOT/userprog" \
  build/tests/userprog/create-null.result \
  build/tests/userprog/create-bad-ptr.result \
  build/tests/userprog/create-bound.result \
  build/tests/userprog/open-null.result \
  build/tests/userprog/open-bad-ptr.result \
  build/tests/userprog/open-boundary.result \
  build/tests/userprog/read-bad-ptr.result \
  build/tests/userprog/read-boundary.result \
  build/tests/userprog/write-bad-ptr.result \
  build/tests/userprog/write-boundary.result \
  build/tests/userprog/exec-bad-ptr.result \
  build/tests/userprog/exec-boundary.result \
  build/tests/userprog/fork-boundary.result \
  build/tests/userprog/bad-read.result \
  build/tests/userprog/bad-write.result \
  build/tests/userprog/bad-read2.result \
  build/tests/userprog/bad-write2.result \
  build/tests/userprog/bad-jump.result \
  build/tests/userprog/bad-jump2.result
```

## 결과 확인

```bash
for t in create-null create-bad-ptr create-bound open-null open-bad-ptr open-boundary read-bad-ptr read-boundary write-bad-ptr write-boundary exec-bad-ptr exec-boundary fork-boundary bad-read bad-write bad-read2 bad-write2 bad-jump bad-jump2; do
  base="$PINTOS_ROOT/userprog/build/tests/userprog/$t"
  printf '\n== %s.result ==\n' "$t"
  cat "$base.result"
done

cat "$PINTOS_ROOT/userprog/build/results" 2>/dev/null || true
```

실패 로그:

```bash
t=bad-read
base="$PINTOS_ROOT/userprog/build/tests/userprog/$t"
cat "$base.output"
cat "$base.errors"
```

## 실패 시 확인할 포인트

- 주소 하나만 검사하지 말고 버퍼 전체 구간을 검사하는지 확인한다.
- page boundary를 넘어가는 문자열과 버퍼에서 두 번째 page도 접근 가능한지 확인한다.
- kernel virtual address를 user pointer로 허용하지 않는지 확인한다.
- 잘못된 user memory 때문에 kernel lock을 잡은 채 process exit 경로로 들어가지 않는지 확인한다.
- `bad-jump*` 실패는 syscall 인자 검증이 아니라 exception handler의 user fault 처리 경로를 본다.
