# vm 01: SPT, lazy loading, page fault

이 단계는 Project 3의 supplemental page table과 lazy loading 기반 page fault 처리를 먼저 확인한다. 모든 명령은 Docker 컨테이너 안에서 실행한다.

## 대상 기능

- supplemental page table 초기화, lookup, insert, destroy
- executable/file-backed page의 lazy load
- anonymous page lazy allocation
- user page fault와 kernel kill 경로 구분
- code page write 방어

## 참고한 reference 문서

- `docs/reference/pintos-kaist-kr/3_project3/1_vm_management.md`
- `docs/reference/pintos-kaist-kr/3_project3/2_anon.md`
- `docs/reference/pintos-kaist-kr/3_project3/0_introduction.md`
- `pintos/tests/vm/Make.tests`

## 공통 준비

```bash
export PINTOS_ROOT=/workspace/pintos
source "$PINTOS_ROOT/activate"
```

## 순차 실행 명령

```bash
make -C "$PINTOS_ROOT/vm" clean
make -C "$PINTOS_ROOT/vm" \
  build/tests/vm/lazy-file.result \
  build/tests/vm/lazy-anon.result \
  build/tests/vm/pt-bad-addr.result \
  build/tests/vm/pt-bad-read.result \
  build/tests/vm/pt-write-code.result \
  build/tests/vm/pt-write-code2.result
```

## 병렬 실행 명령

`vm` 일반 테스트는 같은 build 디렉터리에서 병렬 실행 예시로 사용할 수 있다.

```bash
make -C "$PINTOS_ROOT/vm" clean
make -j"$(nproc)" -C "$PINTOS_ROOT/vm" \
  build/tests/vm/lazy-file.result \
  build/tests/vm/lazy-anon.result \
  build/tests/vm/pt-bad-addr.result \
  build/tests/vm/pt-bad-read.result \
  build/tests/vm/pt-write-code.result \
  build/tests/vm/pt-write-code2.result
```

## 결과 확인

```bash
cat "$PINTOS_ROOT/vm/build/tests/vm/lazy-file.result"
cat "$PINTOS_ROOT/vm/build/tests/vm/lazy-anon.result"
cat "$PINTOS_ROOT/vm/build/tests/vm/pt-bad-addr.result"
cat "$PINTOS_ROOT/vm/build/tests/vm/pt-bad-read.result"
cat "$PINTOS_ROOT/vm/build/tests/vm/pt-write-code.result"
cat "$PINTOS_ROOT/vm/build/tests/vm/pt-write-code2.result"

cat "$PINTOS_ROOT/vm/build/results"
```

실패 로그:

```bash
cat "$PINTOS_ROOT/vm/build/tests/vm/lazy-file.output"
cat "$PINTOS_ROOT/vm/build/tests/vm/lazy-file.errors"
```

## 실패 시 확인할 포인트

- SPT key가 page-aligned user virtual address인지 확인한다.
- page fault handler가 `not_present`, `write`, `user` bit를 구분하는지 확인한다.
- lazy initializer가 file offset, read bytes, zero bytes, writable bit를 보존하는지 확인한다.
- `pt-write-code*` 실패는 writable bit와 write fault 처리 경로를 먼저 본다.
- process exit에서 SPT entry와 frame/file resources를 정리하는지 확인한다.
