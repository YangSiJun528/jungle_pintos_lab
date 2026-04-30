# vm extra: copy-on-write

이 단계는 Project 3 extra인 copy-on-write 테스트를 따로 확인한다. 모든 명령은 Docker 컨테이너 안에서 실행한다.

## 대상 기능

- fork 시 writable page를 즉시 복사하지 않고 COW mapping으로 공유
- write fault 발생 시 private page 분리
- shared frame reference count 관리
- parent/child exit 시 shared frame 정리

## 참고한 reference 문서

- `docs/reference/pintos-kaist-kr/3_project3/6_cow.md`
- `docs/reference/pintos-kaist-kr/3_project3/1_vm_management.md`
- `pintos/vm/Make.vars`
- `pintos/tests/vm/cow/Make.tests`

## 공통 준비

```bash
export PINTOS_ROOT=/workspace/pintos
source "$PINTOS_ROOT/activate"
```

## extra 테스트 설정

현재 저장소의 `pintos/vm/Make.vars`는 `tests/vm/cow`를 `TEST_SUBDIRS`에 포함한다. 만약 로컬 변경으로 빠져 있다면 아래 항목이 있는지 확인한다.

```makefile
TEST_SUBDIRS += tests/vm/cow
```

## 순차 실행 명령

```bash
make -C "$PINTOS_ROOT/vm" clean
make -C "$PINTOS_ROOT/vm" \
  build/tests/vm/cow/cow-simple.result
```

## 병렬 실행 명령

```bash
make -C "$PINTOS_ROOT/vm" clean
make -j"$(nproc)" -C "$PINTOS_ROOT/vm" \
  build/tests/vm/cow/cow-simple.result
```

## 결과 확인

```bash
base="$PINTOS_ROOT/vm/build/tests/vm/cow/cow-simple"
cat "$base.result"
cat "$base.output"
cat "$base.errors"

cat "$PINTOS_ROOT/vm/build/results" 2>/dev/null || true
```

## 실패 시 확인할 포인트

- fork 직후 parent와 child PTE writable bit를 모두 내리는지 확인한다.
- write fault에서 COW page인지 일반 protection fault인지 구분하는지 확인한다.
- refcount가 1인 COW page를 불필요하게 복사하지 않아도 되는지 정책을 점검한다.
- frame free 시 refcount와 frame table entry가 동시에 일관되게 갱신되는지 확인한다.
- COW 구현이 기존 `page-merge-*`, `mmap-*`, `swap-*` 테스트를 깨지 않는지 다시 확인한다.
