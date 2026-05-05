# userprog extra: dup2

이 단계는 Project 2 extra인 `dup2` 테스트만 따로 확인한다. 모든 명령은 Docker 컨테이너 안에서 실행한다.

## 대상 기능

- `dup2(oldfd, newfd)` syscall 구현
- fd table entry 공유와 reference count 관리
- `newfd`가 이미 열려 있을 때 close 후 재연결
- duplicated fd의 file offset 공유
- boundary pointer 처리

## 참고한 reference 문서

- `docs/reference/pintos-kaist-kr/2_project2/6_dup.md`
- `docs/reference/pintos-kaist-kr/2_project2/3_system_call.md`
- `pintos/userprog/Make.vars`
- `pintos/tests/userprog/dup2/Make.tests`

## 공통 준비

```bash
export PINTOS_ROOT=/workspace/pintos
source "$PINTOS_ROOT/activate"
```

## extra 테스트 설정

`dup2`는 기본 `userprog/Make.vars`에서는 꺼져 있다. 테스트하려면 `pintos/userprog/Make.vars`에서 아래 줄의 주석을 해제해야 한다.

```makefile
TDEFINE := -DEXTRA2
TEST_SUBDIRS += tests/userprog/dup2
GRADING_FILE = $(SRCDIR)/tests/userprog/Grading.extra
```

설정 변경 후 build를 새로 만든다.

## 순차 실행 명령

```bash
make -C "$PINTOS_ROOT/userprog" clean
make -C "$PINTOS_ROOT/userprog" \
  build/tests/userprog/dup2/dup2-simple.result \
  build/tests/userprog/dup2/dup2-complex.result
```

## 병렬 실행 명령

```bash
make -C "$PINTOS_ROOT/userprog" clean
make -j"$(nproc)" -C "$PINTOS_ROOT/userprog" \
  build/tests/userprog/dup2/dup2-simple.result \
  build/tests/userprog/dup2/dup2-complex.result
```

## 결과 확인

```bash
cat "$PINTOS_ROOT/userprog/build/tests/userprog/dup2/dup2-simple.result"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/dup2/dup2-simple.output"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/dup2/dup2-simple.errors"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/dup2/dup2-complex.result"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/dup2/dup2-complex.output"
cat "$PINTOS_ROOT/userprog/build/tests/userprog/dup2/dup2-complex.errors"

cat "$PINTOS_ROOT/userprog/build/results"
```

## 실패 시 확인할 포인트

- `oldfd`가 유효하지 않으면 `newfd`를 건드리지 않는지 확인한다.
- `oldfd == newfd`인 경우 같은 fd를 그대로 반환하는지 확인한다.
- `newfd`가 이미 열려 있을 때 기존 파일을 정확히 close하는지 확인한다.
- duplicated fd끼리 file offset을 공유해야 하는지, 별도 `file_reopen()`으로 분리해야 하는지 테스트 기대값을 기준으로 확인한다.
- fd table 확장 한계와 boundary 검사를 빠뜨리지 않았는지 확인한다.
