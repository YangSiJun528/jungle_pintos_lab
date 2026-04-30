# filesys 02: FAT and file growth

이 단계는 Project 4의 indexed/extensible file 구현과 파일 성장 경로를 확인한다. 모든 명령은 Docker 컨테이너 안에서 실행한다.

## 대상 기능

- direct, indirect, doubly-indirect block 확장
- 파일 끝을 넘어 쓰기
- sparse file 처리
- root directory와 subdirectory 내부 파일 성장
- 파일 크기와 seek/tell 결과 일관성

## 참고한 reference 문서

- `docs/reference/pintos-kaist-kr/4_project4/1_indexed_and_extensible_files.md`
- `docs/reference/pintos-kaist-kr/4_project4/0_introduction.md`
- `pintos/filesys/Make.vars`
- `pintos/tests/filesys/extended/Make.tests`

## 공통 준비

```bash
export PINTOS_ROOT=/workspace/pintos
source "$PINTOS_ROOT/activate"
```

## 순차 실행 명령

`filesys/Make.vars` 기본 설정은 no-VM Project 4 테스트 기준이다.

```bash
make -C "$PINTOS_ROOT/filesys" clean
make -C "$PINTOS_ROOT/filesys" \
  build/tests/filesys/extended/grow-create.result \
  build/tests/filesys/extended/grow-file-size.result \
  build/tests/filesys/extended/grow-root-sm.result \
  build/tests/filesys/extended/grow-root-lg.result \
  build/tests/filesys/extended/grow-dir-lg.result \
  build/tests/filesys/extended/grow-seq-sm.result \
  build/tests/filesys/extended/grow-seq-lg.result \
  build/tests/filesys/extended/grow-sparse.result \
  build/tests/filesys/extended/grow-tell.result \
  build/tests/filesys/extended/grow-two-files.result
```

## 병렬 실행

`filesys/extended` 테스트는 같은 build 디렉터리에서 `tmp.dsk`를 공유한다. 이 문서에서는 같은 build 디렉터리의 `make -j`를 안전한 병렬 명령으로 쓰지 않는다.

병렬이 필요하면 `090_병렬_실행_가이드.md`처럼 `/tmp`에 Pintos 작업 복사본을 여러 개 만들고 각 복사본에서 분리 실행한다.

## 결과 확인

```bash
for t in grow-create grow-file-size grow-root-sm grow-root-lg grow-dir-lg grow-seq-sm grow-seq-lg grow-sparse grow-tell grow-two-files; do
  base="$PINTOS_ROOT/filesys/build/tests/filesys/extended/$t"
  printf '\n== %s.result ==\n' "$t"
  cat "$base.result"
done

cat "$PINTOS_ROOT/filesys/build/results" 2>/dev/null || true
```

실패 로그:

```bash
t=grow-sparse
base="$PINTOS_ROOT/filesys/build/tests/filesys/extended/$t"
cat "$base.output"
cat "$base.errors"
```

## 실패 시 확인할 포인트

- inode length 갱신과 data block 할당 순서가 crash 없이 일관되는지 확인한다.
- sparse file에서 중간 hole을 zero-fill로 읽는지 확인한다.
- indirect block 자체를 free map에 할당하고 초기화하는지 확인한다.
- 파일 성장 실패 시 이미 할당한 sector를 rollback하는지 확인한다.
- root directory 성장과 일반 file 성장 경로를 혼동하지 않았는지 확인한다.
