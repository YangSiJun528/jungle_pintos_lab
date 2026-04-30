# vm 03: paging and frame

이 단계는 frame table, eviction 전 단계의 page 설치, fork/copy 경로, page merge 계열 테스트를 확인한다. 모든 명령은 Docker 컨테이너 안에서 실행한다.

## 대상 기능

- frame allocation과 page-table install
- 여러 page의 순차/무작위 접근
- child process와 page 내용 독립성
- fork 이후 page copy 또는 COW 전 기본 복제 동작
- page merge 계열의 동시 실행 안정성

## 참고한 reference 문서

- `docs/reference/pintos-kaist-kr/3_project3/1_vm_management.md`
- `docs/reference/pintos-kaist-kr/3_project3/2_anon.md`
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
  build/tests/vm/page-linear.result \
  build/tests/vm/page-parallel.result \
  build/tests/vm/page-merge-seq.result \
  build/tests/vm/page-merge-par.result \
  build/tests/vm/page-merge-stk.result \
  build/tests/vm/page-merge-mm.result \
  build/tests/vm/page-shuffle.result
```

## 병렬 실행 명령

```bash
make -C "$PINTOS_ROOT/vm" clean
make -j"$(nproc)" -C "$PINTOS_ROOT/vm" \
  build/tests/vm/page-linear.result \
  build/tests/vm/page-parallel.result \
  build/tests/vm/page-merge-seq.result \
  build/tests/vm/page-merge-par.result \
  build/tests/vm/page-merge-stk.result \
  build/tests/vm/page-merge-mm.result \
  build/tests/vm/page-shuffle.result
```

## 결과 확인

```bash
for t in page-linear page-parallel page-merge-seq page-merge-par page-merge-stk page-merge-mm page-shuffle; do
  base="$PINTOS_ROOT/vm/build/tests/vm/$t"
  printf '\n== %s.result ==\n' "$t"
  cat "$base.result"
done

cat "$PINTOS_ROOT/vm/build/results" 2>/dev/null || true
```

실패 로그:

```bash
t=page-merge-par
base="$PINTOS_ROOT/vm/build/tests/vm/$t"
cat "$base.output"
cat "$base.errors"
```

## 실패 시 확인할 포인트

- frame table entry와 physical page ownership이 일관되는지 확인한다.
- page fault 중 같은 SPT entry를 중복 claim하지 않도록 lock이 있는지 확인한다.
- fork에서 parent page 내용을 child page로 정확히 복사하는지 확인한다.
- executable/file-backed/anonymous page 타입별 initializer가 유지되는지 확인한다.
- `page-shuffle` timeout은 eviction이 아직 없거나 frame reclaim이 제대로 동작하지 않는 신호일 수 있다.
