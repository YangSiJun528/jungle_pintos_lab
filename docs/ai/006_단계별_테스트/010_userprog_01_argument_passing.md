# userprog 01: argument passing

이 단계는 Project 2의 첫 진입점인 user stack 구성과 argument passing만 좁게 확인한다. 모든 명령은 Docker 컨테이너 안에서 실행한다.

## 대상 기능

- `process_exec()` 이후 user stack에 `argc`, `argv`, return address 배치
- 인자 문자열 복사와 8바이트 정렬
- 공백이 여러 개 있거나 인자가 많은 경우 처리
- child program이 인자를 읽을 때 page fault 없이 실행되는지 확인

## 참고한 reference 문서

- `docs/reference/pintos-kaist-kr/2_project2/1_argument_passing.md`
- `docs/reference/pintos-kaist-kr/2_project2/0_introduction.md`
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
  build/tests/userprog/args-none.result \
  build/tests/userprog/args-single.result \
  build/tests/userprog/args-multiple.result \
  build/tests/userprog/args-many.result \
  build/tests/userprog/args-dbl-space.result
```

## 병렬 실행 명령

`userprog` 일반 테스트는 같은 build 디렉터리에서 병렬 실행 예시로 사용할 수 있다.

```bash
make -C "$PINTOS_ROOT/userprog" clean
make -j"$(nproc)" -C "$PINTOS_ROOT/userprog" \
  build/tests/userprog/args-none.result \
  build/tests/userprog/args-single.result \
  build/tests/userprog/args-multiple.result \
  build/tests/userprog/args-many.result \
  build/tests/userprog/args-dbl-space.result
```

## 결과 확인

```bash
for t in args-none args-single args-multiple args-many args-dbl-space; do
  base="$PINTOS_ROOT/userprog/build/tests/userprog/$t"
  printf '\n== %s.result ==\n' "$t"
  cat "$base.result"
  printf '== %s.output ==\n' "$t"
  cat "$base.output"
  printf '== %s.errors ==\n' "$t"
  cat "$base.errors"
done

cat "$PINTOS_ROOT/userprog/build/results" 2>/dev/null || true
```

## 실패 시 확인할 포인트

- `rsp`가 8바이트 정렬을 만족하는지 확인한다.
- `argv[argc] == NULL` sentinel을 넣었는지 확인한다.
- 문자열을 push한 뒤 포인터 배열이 문자열 주소를 정확히 가리키는지 확인한다.
- 연속 공백을 빈 인자로 잘못 세지 않는지 확인한다.
- `args-many` 실패 시 stack overflow나 페이지 경계 근처 복사 오류를 먼저 본다.
