# WEEK10 filesys base 선별 테스트

이 단계는 WEEK10 이슈로 열린 `filesys/base` 테스트만 따로 실행한다. 모든 명령은 Docker 컨테이너 안에서 실행한다.

## 대상 이슈

| Issue | 테스트 |
| --- | --- |
| #129 | `lg-create` |
| #130 | `lg-full` |
| #131 | `lg-random` |
| #132 | `lg-seq-block` |
| #133 | `lg-seq-random` |
| #134 | `sm-create` |
| #135 | `sm-full` |
| #136 | `sm-random` |
| #137 | `sm-seq-block` |
| #138 | `sm-seq-random` |
| #139 | `syn-remove` |
| #140 | `syn-write` |

## 대상 기능

- 작은 파일과 큰 파일 생성
- 파일 시스템 full 상황 처리
- 순차 block 접근과 random 접근
- 동시 write와 remove 중 파일 내용, inode, free map 일관성 유지
- Project 2 userprog 파일 syscall과 Project 4 filesys 구현의 연동 회귀 확인

## 참고한 reference 문서

- `docs/reference/pintos-kaist-kr/4_project4/0_introduction.md`
- `docs/reference/pintos-kaist-kr/4_project4/1_indexed_and_extensible_files.md`
- `pintos/filesys/Make.vars`
- `pintos/tests/filesys/base/Make.tests`
- `pintos/tests/filesys/base/Rubric`

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
  build/tests/filesys/base/syn-remove.result \
  build/tests/filesys/base/syn-write.result
```

## 병렬 실행 명령

`filesys/base`는 persistence 테스트처럼 같은 `tmp.dsk`를 이어서 재사용하는 흐름이 아니므로, 개별 `.result` 타깃을 병렬로 실행해도 된다.

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
  build/tests/filesys/base/syn-remove.result \
  build/tests/filesys/base/syn-write.result
```

## 결과 확인

```bash
cat "$PINTOS_ROOT/filesys/build/tests/filesys/base/lg-create.result"
cat "$PINTOS_ROOT/filesys/build/tests/filesys/base/lg-full.result"
cat "$PINTOS_ROOT/filesys/build/tests/filesys/base/lg-random.result"
cat "$PINTOS_ROOT/filesys/build/tests/filesys/base/lg-seq-block.result"
cat "$PINTOS_ROOT/filesys/build/tests/filesys/base/lg-seq-random.result"
cat "$PINTOS_ROOT/filesys/build/tests/filesys/base/sm-create.result"
cat "$PINTOS_ROOT/filesys/build/tests/filesys/base/sm-full.result"
cat "$PINTOS_ROOT/filesys/build/tests/filesys/base/sm-random.result"
cat "$PINTOS_ROOT/filesys/build/tests/filesys/base/sm-seq-block.result"
cat "$PINTOS_ROOT/filesys/build/tests/filesys/base/sm-seq-random.result"
cat "$PINTOS_ROOT/filesys/build/tests/filesys/base/syn-remove.result"
cat "$PINTOS_ROOT/filesys/build/tests/filesys/base/syn-write.result"

cat "$PINTOS_ROOT/filesys/build/results"
```

실패 로그는 같은 경로의 `.output`, `.errors`를 확인한다.

```bash
cat "$PINTOS_ROOT/filesys/build/tests/filesys/base/lg-create.output"
cat "$PINTOS_ROOT/filesys/build/tests/filesys/base/lg-create.errors"
cat "$PINTOS_ROOT/filesys/build/tests/filesys/base/syn-write.output"
cat "$PINTOS_ROOT/filesys/build/tests/filesys/base/syn-write.errors"
```

## 실패 시 확인할 포인트

- `lg-*`, `sm-*` 실패는 inode 확장, file length 갱신, free map rollback을 먼저 확인한다.
- `*-random` 실패는 `file_seek()`, `file_read_at()`, `file_write_at()`이 offset과 sector 경계를 일관되게 다루는지 확인한다.
- `*-full` 실패는 공간 부족 중간 실패 후 할당된 sector가 남지 않는지 확인한다.
- `syn-write` 실패는 여러 자식 프로세스가 같은 파일에 쓰는 동안 filesys, inode, file offset 동기화 범위가 충분한지 확인한다.
- `syn-remove` 실패는 열린 파일을 remove한 뒤에도 기존 file handle로 read/write가 가능한지 확인한다.
