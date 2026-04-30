# vm 04: mmap and file-backed pages

이 단계는 memory mapped file과 file-backed page의 dirty/writeback 경로를 확인한다. 모든 명령은 Docker 컨테이너 안에서 실행한다.

## 대상 기능

- `mmap`, `munmap` syscall
- file-backed page lazy loading
- dirty page writeback
- overlapping mapping, misaligned offset, zero length 방어
- mmap 영역과 code/data/stack 영역 충돌 방어
- fork/exit/close/remove와 mmap lifetime 정리

## 참고한 reference 문서

- `docs/reference/pintos-kaist-kr/3_project3/4_memory_mapped_files.md`
- `docs/reference/pintos-kaist-kr/3_project3/1_vm_management.md`
- `docs/reference/pintos-kaist-kr/3_project3/7_FAQ.md`
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
  build/tests/vm/mmap-read.result \
  build/tests/vm/mmap-close.result \
  build/tests/vm/mmap-unmap.result \
  build/tests/vm/mmap-overlap.result \
  build/tests/vm/mmap-twice.result \
  build/tests/vm/mmap-write.result \
  build/tests/vm/mmap-ro.result \
  build/tests/vm/mmap-exit.result \
  build/tests/vm/mmap-shuffle.result \
  build/tests/vm/mmap-bad-fd.result \
  build/tests/vm/mmap-clean.result \
  build/tests/vm/mmap-inherit.result \
  build/tests/vm/mmap-misalign.result \
  build/tests/vm/mmap-null.result \
  build/tests/vm/mmap-over-code.result \
  build/tests/vm/mmap-over-data.result \
  build/tests/vm/mmap-over-stk.result \
  build/tests/vm/mmap-remove.result \
  build/tests/vm/mmap-zero.result \
  build/tests/vm/mmap-bad-fd2.result \
  build/tests/vm/mmap-bad-fd3.result \
  build/tests/vm/mmap-zero-len.result \
  build/tests/vm/mmap-off.result \
  build/tests/vm/mmap-bad-off.result \
  build/tests/vm/mmap-kernel.result
```

## 병렬 실행 명령

```bash
make -C "$PINTOS_ROOT/vm" clean
make -j"$(nproc)" -C "$PINTOS_ROOT/vm" \
  build/tests/vm/mmap-read.result \
  build/tests/vm/mmap-close.result \
  build/tests/vm/mmap-unmap.result \
  build/tests/vm/mmap-overlap.result \
  build/tests/vm/mmap-twice.result \
  build/tests/vm/mmap-write.result \
  build/tests/vm/mmap-ro.result \
  build/tests/vm/mmap-exit.result \
  build/tests/vm/mmap-shuffle.result \
  build/tests/vm/mmap-bad-fd.result \
  build/tests/vm/mmap-clean.result \
  build/tests/vm/mmap-inherit.result \
  build/tests/vm/mmap-misalign.result \
  build/tests/vm/mmap-null.result \
  build/tests/vm/mmap-over-code.result \
  build/tests/vm/mmap-over-data.result \
  build/tests/vm/mmap-over-stk.result \
  build/tests/vm/mmap-remove.result \
  build/tests/vm/mmap-zero.result \
  build/tests/vm/mmap-bad-fd2.result \
  build/tests/vm/mmap-bad-fd3.result \
  build/tests/vm/mmap-zero-len.result \
  build/tests/vm/mmap-off.result \
  build/tests/vm/mmap-bad-off.result \
  build/tests/vm/mmap-kernel.result
```

## 결과 확인

```bash
for t in mmap-read mmap-close mmap-unmap mmap-overlap mmap-twice mmap-write mmap-ro mmap-exit mmap-shuffle mmap-bad-fd mmap-clean mmap-inherit mmap-misalign mmap-null mmap-over-code mmap-over-data mmap-over-stk mmap-remove mmap-zero mmap-bad-fd2 mmap-bad-fd3 mmap-zero-len mmap-off mmap-bad-off mmap-kernel; do
  base="$PINTOS_ROOT/vm/build/tests/vm/$t"
  printf '\n== %s.result ==\n' "$t"
  cat "$base.result"
done

cat "$PINTOS_ROOT/vm/build/results" 2>/dev/null || true
```

실패 로그:

```bash
t=mmap-write
base="$PINTOS_ROOT/vm/build/tests/vm/$t"
cat "$base.output"
cat "$base.errors"
```

## 실패 시 확인할 포인트

- mapping 시작 주소와 file offset이 page-aligned인지 검증하는지 확인한다.
- mapping 구간 전체가 비어 있는지 SPT 기준으로 확인한다.
- `munmap`과 process exit에서 dirty page만 file에 writeback하는지 확인한다.
- `mmap-remove`, `mmap-close` 실패는 mapping이 original fd lifetime에 잘못 묶여 있는지 본다.
- `mmap-shuffle` 실패는 eviction, dirty bit 확인, writeback 순서를 같이 점검한다.
