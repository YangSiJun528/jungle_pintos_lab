# vm 05: swap

이 단계는 physical memory가 부족할 때 anonymous/file-backed page를 eviction하고 swap 또는 file로 복구하는 경로를 확인한다. 모든 명령은 Docker 컨테이너 안에서 실행한다.

## 대상 기능

- frame eviction victim 선정
- anonymous page swap out/in
- file-backed page eviction과 reload/writeback
- swap slot 할당과 해제
- fork 중 swap된 page 복구

## 참고한 reference 문서

- `docs/reference/pintos-kaist-kr/3_project3/5_swapping.md`
- `docs/reference/pintos-kaist-kr/3_project3/2_anon.md`
- `docs/reference/pintos-kaist-kr/3_project3/1_vm_management.md`
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
  build/tests/vm/swap-file.result \
  build/tests/vm/swap-anon.result \
  build/tests/vm/swap-iter.result \
  build/tests/vm/swap-fork.result
```

## 병렬 실행 명령

```bash
make -C "$PINTOS_ROOT/vm" clean
make -j"$(nproc)" -C "$PINTOS_ROOT/vm" \
  build/tests/vm/swap-file.result \
  build/tests/vm/swap-anon.result \
  build/tests/vm/swap-iter.result \
  build/tests/vm/swap-fork.result
```

## 결과 확인

```bash
for t in swap-file swap-anon swap-iter swap-fork; do
  base="$PINTOS_ROOT/vm/build/tests/vm/$t"
  printf '\n== %s.result ==\n' "$t"
  cat "$base.result"
  printf '== %s.errors ==\n' "$t"
  cat "$base.errors"
done

cat "$PINTOS_ROOT/vm/build/results" 2>/dev/null || true
```

실패 로그:

```bash
t=swap-anon
base="$PINTOS_ROOT/vm/build/tests/vm/$t"
cat "$base.output"
cat "$base.errors"
```

## 실패 시 확인할 포인트

- victim frame이 pinned 상태일 때 eviction하지 않는지 확인한다.
- swap bitmap slot을 page in 이후 해제하는지 확인한다.
- anonymous page와 file-backed page의 swap/writeback 정책을 섞지 않았는지 확인한다.
- dirty file-backed page는 file로, anonymous page는 swap으로 보내는지 확인한다.
- `swap-fork` 실패는 swapped page를 fork할 때 parent/child 내용이 보존되는지 확인한다.
