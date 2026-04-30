# filesys 01: base filesystem regression

이 단계는 Project 4 구현 중 기존 base filesystem 동작이 깨지지 않았는지 먼저 확인한다. 모든 명령은 Docker 컨테이너 안에서 실행한다.

## 대상 기능

- 작은 파일과 큰 파일 생성
- 순차 block 접근과 random 접근
- 파일 시스템 full 상황 처리
- 동시 read/write/remove 기본 동작
- Project 2 userprog와 연동되는 기본 파일 syscall 회귀 확인

## 참고한 reference 문서

- `docs/reference/pintos-kaist-kr/4_project4/0_introduction.md`
- `docs/reference/pintos-kaist-kr/4_project4/1_indexed_and_extensible_files.md`
- `pintos/filesys/Make.vars`
- `pintos/tests/filesys/base/Make.tests`

## 공통 준비

```bash
export PINTOS_ROOT=/workspace/pintos
source "$PINTOS_ROOT/activate"
```

## 순차 실행 명령

```bash
make -C "$PINTOS_ROOT/filesys" clean
make -C "$PINTOS_ROOT/filesys" \
  build/tests/filesys/base/lg-create.result \
  build/tests/filesys/base/lg-full.result \
  build/tests/filesys/base/lg-random.result \
  build/tests/filesys/base/lg-seq-block.result \
  build/tests/filesys/base/lg-seq-random.result \
  build/tests/filesys/base/sm-create.result \
  build/tests/filesys/base/sm-full.result \
  build/tests/filesys/base/sm-random.result \
  build/tests/filesys/base/sm-seq-block.result \
  build/tests/filesys/base/sm-seq-random.result \
  build/tests/filesys/base/syn-read.result \
  build/tests/filesys/base/syn-remove.result \
  build/tests/filesys/base/syn-write.result
```

## 병렬 실행 명령

`filesys/base`는 extended 테스트처럼 `tmp.dsk`를 공유하는 별도 persistence 흐름이 아니므로 병렬 예시로 사용할 수 있다.

```bash
make -C "$PINTOS_ROOT/filesys" clean
make -j"$(nproc)" -C "$PINTOS_ROOT/filesys" \
  build/tests/filesys/base/lg-create.result \
  build/tests/filesys/base/lg-full.result \
  build/tests/filesys/base/lg-random.result \
  build/tests/filesys/base/lg-seq-block.result \
  build/tests/filesys/base/lg-seq-random.result \
  build/tests/filesys/base/sm-create.result \
  build/tests/filesys/base/sm-full.result \
  build/tests/filesys/base/sm-random.result \
  build/tests/filesys/base/sm-seq-block.result \
  build/tests/filesys/base/sm-seq-random.result \
  build/tests/filesys/base/syn-read.result \
  build/tests/filesys/base/syn-remove.result \
  build/tests/filesys/base/syn-write.result
```

## 결과 확인

```bash
for t in lg-create lg-full lg-random lg-seq-block lg-seq-random sm-create sm-full sm-random sm-seq-block sm-seq-random syn-read syn-remove syn-write; do
  base="$PINTOS_ROOT/filesys/build/tests/filesys/base/$t"
  printf '\n== %s.result ==\n' "$t"
  cat "$base.result"
done

cat "$PINTOS_ROOT/filesys/build/results" 2>/dev/null || true
```

실패 로그:

```bash
t=sm-create
base="$PINTOS_ROOT/filesys/build/tests/filesys/base/$t"
cat "$base.output"
cat "$base.errors"
```

## 실패 시 확인할 포인트

- Project 2 파일 syscall이 Project 4 변경 후 깨지지 않았는지 확인한다.
- inode sector, file length, free map update가 서로 일관되는지 확인한다.
- 동시성 테스트 실패는 filesys lock 범위와 inode lock 범위를 먼저 본다.
- full 상황에서 할당 실패 후 free map이 rollback되는지 확인한다.
- base가 깨진 상태에서는 extended filesystem 테스트로 넘어가지 않는다.
