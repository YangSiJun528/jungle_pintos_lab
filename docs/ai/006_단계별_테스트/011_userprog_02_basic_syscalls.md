# userprog 02: basic syscalls

이 단계는 argument passing 이후 최소 syscall 경로와 기본 파일 syscall을 확인한다. 모든 명령은 Docker 컨테이너 안에서 실행한다.

## 대상 기능

- syscall 번호와 인자 읽기
- `halt`, `exit`
- `create`, `open`, `read`, `write`, `close` 기본 성공 경로
- 파일 이름 포인터와 파일 객체 관리의 기본 동작

## 참고한 reference 문서

- `docs/reference/pintos-kaist-kr/2_project2/3_system_call.md`
- `docs/reference/pintos-kaist-kr/2_project2/4_process_termination.md`
- `docs/reference/pintos-kaist-kr/2_project2/2_user_memory.md`
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
  build/tests/userprog/halt.result \
  build/tests/userprog/exit.result \
  build/tests/userprog/create-normal.result \
  build/tests/userprog/create-empty.result \
  build/tests/userprog/create-null.result \
  build/tests/userprog/create-bad-ptr.result \
  build/tests/userprog/create-long.result \
  build/tests/userprog/create-exists.result \
  build/tests/userprog/create-bound.result \
  build/tests/userprog/open-normal.result \
  build/tests/userprog/open-missing.result \
  build/tests/userprog/open-boundary.result \
  build/tests/userprog/open-empty.result \
  build/tests/userprog/open-null.result \
  build/tests/userprog/open-bad-ptr.result \
  build/tests/userprog/open-twice.result \
  build/tests/userprog/read-normal.result \
  build/tests/userprog/write-normal.result \
  build/tests/userprog/close-normal.result
```

## 병렬 실행 명령

```bash
make -C "$PINTOS_ROOT/userprog" clean
make -j"$(nproc)" -C "$PINTOS_ROOT/userprog" \
  build/tests/userprog/halt.result \
  build/tests/userprog/exit.result \
  build/tests/userprog/create-normal.result \
  build/tests/userprog/create-empty.result \
  build/tests/userprog/create-null.result \
  build/tests/userprog/create-bad-ptr.result \
  build/tests/userprog/create-long.result \
  build/tests/userprog/create-exists.result \
  build/tests/userprog/create-bound.result \
  build/tests/userprog/open-normal.result \
  build/tests/userprog/open-missing.result \
  build/tests/userprog/open-boundary.result \
  build/tests/userprog/open-empty.result \
  build/tests/userprog/open-null.result \
  build/tests/userprog/open-bad-ptr.result \
  build/tests/userprog/open-twice.result \
  build/tests/userprog/read-normal.result \
  build/tests/userprog/write-normal.result \
  build/tests/userprog/close-normal.result
```

## 결과 확인

```bash
for t in halt exit create-normal create-empty create-null create-bad-ptr create-long create-exists create-bound open-normal open-missing open-boundary open-empty open-null open-bad-ptr open-twice read-normal write-normal close-normal; do
  base="$PINTOS_ROOT/userprog/build/tests/userprog/$t"
  printf '\n== %s.result ==\n' "$t"
  cat "$base.result"
done

cat "$PINTOS_ROOT/userprog/build/results" 2>/dev/null || true
```

실패한 테스트만 자세히 본다.

```bash
t=open-normal
base="$PINTOS_ROOT/userprog/build/tests/userprog/$t"
cat "$base.output"
cat "$base.errors"
```

## 실패 시 확인할 포인트

- syscall handler가 user stack에서 인자를 읽기 전에 주소 검증을 하는지 확인한다.
- `exit`가 process name과 exit status를 reference 형식대로 출력하는지 확인한다.
- fd `0`, `1`을 stdin/stdout으로 예약하고 일반 파일 fd는 그 이후부터 배정하는지 확인한다.
- `filesys_lock` 같은 전역 파일 시스템 동기화가 누락되어 있지 않은지 확인한다.
- `create-null`, `open-null`, `*-bad-ptr` 실패는 user pointer 검증을 먼저 본다.
