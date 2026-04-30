# filesys extra: buffer cache

이 단계는 Project 4 extra인 buffer cache 테스트를 따로 확인한다. 모든 명령은 Docker 컨테이너 안에서 실행한다.

## 대상 기능

- disk read/write를 buffer cache로 우회
- cache hit/miss와 eviction
- dirty cache block writeback
- metadata/data block cache 정책
- VM-enabled filesys 설정에서 buffer-cache 테스트 실행

## 참고한 reference 문서

- `docs/reference/pintos-kaist-kr/4_project4/3_buffer_cache.md`
- `docs/reference/pintos-kaist-kr/4_project4/4_synchronization.md`
- `pintos/filesys/Make.vars`
- `pintos/tests/filesys/buffer-cache/Make.tests`
- `pintos/tests/filesys/buffer-cache/Rubric`

## 공통 준비

```bash
export PINTOS_ROOT=/workspace/pintos
source "$PINTOS_ROOT/activate"
```

## extra 테스트 설정

`buffer-cache`는 기본 no-VM `filesys/Make.vars`에서는 꺼져 있다. 테스트하려면 `pintos/filesys/Make.vars`에서 VM 관련 줄을 활성화해야 한다.

```makefile
os.dsk: DEFINES += -DVM
KERNEL_SUBDIRS += vm
TEST_SUBDIRS += tests/vm tests/filesys/buffer-cache
GRADING_FILE = $(SRCDIR)/tests/filesys/Grading.with-vm
```

설정 변경 후 build를 새로 만든다.

## 순차 실행 명령

```bash
make -C "$PINTOS_ROOT/filesys" clean
make -C "$PINTOS_ROOT/filesys" \
  build/tests/filesys/buffer-cache/bc-easy.result
```

## 병렬 실행

`filesys/buffer-cache` 테스트도 내부에서 `tmp.dsk`를 사용한다. 같은 build 디렉터리에서 `make -j`를 안전한 병렬 명령으로 쓰지 않는다.

병렬이 필요하면 `090_병렬_실행_가이드.md`처럼 `/tmp`에 Pintos 작업 복사본을 여러 개 만들고 각 복사본에서 분리 실행한다.

## 결과 확인

```bash
base="$PINTOS_ROOT/filesys/build/tests/filesys/buffer-cache/bc-easy"
cat "$base.result"
cat "$base.output"
cat "$base.errors"

cat "$PINTOS_ROOT/filesys/build/results" 2>/dev/null || true
```

## 실패 시 확인할 포인트

- VM-enabled filesys 설정이 실제로 적용되어 `tests/filesys/buffer-cache`가 build에 포함됐는지 확인한다.
- cache read/write count가 checker 기대값에 맞게 감소하는지 확인한다.
- dirty block을 eviction 또는 shutdown 시 writeback하는지 확인한다.
- block device 접근 경로가 cache를 우회하지 않는지 확인한다.
- cache lock과 inode/filesys lock 사이에서 deadlock이 생기지 않는지 확인한다.
