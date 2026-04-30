# Docker 공통 테스트 환경

이 문서는 Project 2 `userprog`, Project 3 `vm`, Project 4 `filesys` 단계별 테스트 문서에서 공통으로 쓰는 실행 환경을 정리한다. 모든 명령은 Docker 컨테이너 안에서 실행한다.

## 공통 준비

컨테이너에 들어온 뒤 현재 위치와 상관없이 아래를 먼저 실행한다.

```bash
export PINTOS_ROOT=/workspace/pintos
source "$PINTOS_ROOT/activate"
```

## 프로젝트별 기본 위치

```bash
make -C "$PINTOS_ROOT/userprog" clean
make -C "$PINTOS_ROOT/vm" clean
make -C "$PINTOS_ROOT/filesys" clean
```

## 특정 테스트 실행 형식

`make check` 전체 대신 필요한 `.result` target만 지정하면 단계별로 빠르게 확인할 수 있다.

```bash
make -C "$PINTOS_ROOT/userprog" build/tests/userprog/args-none.result
make -C "$PINTOS_ROOT/vm" build/tests/vm/pt-grow-stack.result
make -C "$PINTOS_ROOT/filesys" build/tests/filesys/base/sm-create.result
```

## 결과 파일 확인 형식

테스트별 결과는 각 프로젝트의 `build/tests/...` 아래에 생긴다.

- `.result`: checker 판정. 통과하면 `PASS`
- `.output`: Pintos 실행 출력
- `.errors`: Pintos 실행 중 표준 에러
- `build/results`: `make check` 또는 grading 흐름에서 생성되는 전체 요약

예시:

```bash
cat "$PINTOS_ROOT/userprog/build/tests/userprog/args-none.result"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/args-none.output"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/args-none.errors"
cat "$PINTOS_ROOT/userprog/build/results" 2>/dev/null || true
```

## 단계별 문서 사용 순서

1. 해당 단계 문서의 `공통 준비`를 실행한다.
2. `순차 실행 명령`으로 먼저 통과 여부를 확인한다.
3. 같은 단계가 안정적으로 통과하면 `병렬 실행 명령`으로 반복 확인한다.
4. 실패하면 `.result`만 보지 말고 `.output`, `.errors`를 같이 확인한다.
5. 현재 단계가 실패한 상태에서는 다음 단계 테스트로 넘어가지 않는다.

## 참고

- `make clean`은 해당 프로젝트의 `build/`를 지운다.
- stale `.result`를 피하려면 단계 실행 전에 `make clean`을 실행한다.
- 컴파일 실패, kernel panic, timeout이 나면 `.result`가 없을 수 있다.
- 특정 `.result` target만 실행한 경우 `build/results`가 없을 수 있다.
- `filesys/extended`, `filesys/buffer-cache`는 내부 실행 중 `tmp.dsk`를 공유하므로 같은 build 디렉터리에서 무조건 안전한 병렬 대상으로 보지 않는다.
