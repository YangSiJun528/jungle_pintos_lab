# vm 02: stack growth

이 단계는 Project 3 stack growth 정책과 stack page lazy allocation을 확인한다. 모든 명령은 Docker 컨테이너 안에서 실행한다.

## 대상 기능

- user stack 접근 fault 감지
- stack pointer 근처 접근만 growth로 인정
- 최대 stack 크기 제한
- syscall 중 stack pointer 기반 growth
- 큰 stack object 처리

## 참고한 reference 문서

- `docs/reference/pintos-kaist-kr/3_project3/3_stack_growth.md`
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
  build/tests/vm/pt-grow-stack.result \
  build/tests/vm/pt-grow-bad.result \
  build/tests/vm/pt-big-stk-obj.result \
  build/tests/vm/pt-grow-stk-sc.result
```

## 병렬 실행 명령

```bash
make -C "$PINTOS_ROOT/vm" clean
make -j"$(nproc)" -C "$PINTOS_ROOT/vm" \
  build/tests/vm/pt-grow-stack.result \
  build/tests/vm/pt-grow-bad.result \
  build/tests/vm/pt-big-stk-obj.result \
  build/tests/vm/pt-grow-stk-sc.result
```

## 결과 확인

```bash
for t in pt-grow-stack pt-grow-bad pt-big-stk-obj pt-grow-stk-sc; do
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
t=pt-grow-stack
base="$PINTOS_ROOT/vm/build/tests/vm/$t"
cat "$base.output"
cat "$base.errors"
```

## 실패 시 확인할 포인트

- fault address가 user address이고 stack limit 안인지 확인한다.
- fault address와 `rsp`의 거리 조건을 너무 넓거나 좁게 잡지 않았는지 확인한다.
- syscall path에서 kernel이 저장한 user `rsp`를 page fault handler가 참조하는지 확인한다.
- stack page도 SPT에 anonymous page로 등록되고 frame allocation을 거치는지 확인한다.
- `pt-grow-bad`가 통과하지 않으면 무분별한 stack growth를 허용하고 있을 가능성이 높다.
