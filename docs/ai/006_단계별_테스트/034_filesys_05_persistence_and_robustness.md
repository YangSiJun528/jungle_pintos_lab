# filesys 05: persistence and robustness

이 단계는 extended filesystem의 persistence checker와 robustness 항목을 마무리로 확인한다. 모든 명령은 Docker 컨테이너 안에서 실행한다.

## 대상 기능

- test 실행 후 tar로 파일 시스템 내용을 꺼내 persistence 검사
- directory tree와 file growth 결과가 reboot-like 흐름 뒤에도 보존되는지 확인
- remove, open, empty path 등 robustness case 재확인
- symlink와 synchronization 결과의 persistence 확인

## 참고한 reference 문서

- `docs/reference/pintos-kaist-kr/project4/indexed_and_extensible_files.md`
- `docs/reference/pintos-kaist-kr/project4/subdirectories.md`
- `docs/reference/pintos-kaist-kr/project4/synchronization.md`
- `pintos/tests/filesys/extended/Make.tests`
- `pintos/tests/filesys/extended/Rubric.persistence`
- `pintos/tests/filesys/extended/Rubric.robustness`

## 공통 준비

```bash
export PINTOS_ROOT=/workspace/pintos
source "$PINTOS_ROOT/activate"
```

## 순차 실행 명령

`*-persistence.result` target은 대응되는 원본 테스트를 먼저 실행한 뒤 persistence checker를 실행한다.

```bash
make -C "$PINTOS_ROOT/filesys" clean
make -C "$PINTOS_ROOT/filesys" \
  build/tests/filesys/extended/dir-empty-name-persistence.result \
  build/tests/filesys/extended/dir-mk-tree-persistence.result \
  build/tests/filesys/extended/dir-mkdir-persistence.result \
  build/tests/filesys/extended/dir-open-persistence.result \
  build/tests/filesys/extended/dir-over-file-persistence.result \
  build/tests/filesys/extended/dir-rm-cwd-persistence.result \
  build/tests/filesys/extended/dir-rm-parent-persistence.result \
  build/tests/filesys/extended/dir-rm-root-persistence.result \
  build/tests/filesys/extended/dir-rm-tree-persistence.result \
  build/tests/filesys/extended/dir-rmdir-persistence.result \
  build/tests/filesys/extended/dir-under-file-persistence.result \
  build/tests/filesys/extended/dir-vine-persistence.result \
  build/tests/filesys/extended/grow-create-persistence.result \
  build/tests/filesys/extended/grow-dir-lg-persistence.result \
  build/tests/filesys/extended/grow-file-size-persistence.result \
  build/tests/filesys/extended/grow-root-lg-persistence.result \
  build/tests/filesys/extended/grow-root-sm-persistence.result \
  build/tests/filesys/extended/grow-seq-lg-persistence.result \
  build/tests/filesys/extended/grow-seq-sm-persistence.result \
  build/tests/filesys/extended/grow-sparse-persistence.result \
  build/tests/filesys/extended/grow-tell-persistence.result \
  build/tests/filesys/extended/grow-two-files-persistence.result \
  build/tests/filesys/extended/syn-rw-persistence.result \
  build/tests/filesys/extended/symlink-file-persistence.result \
  build/tests/filesys/extended/symlink-dir-persistence.result \
  build/tests/filesys/extended/symlink-link-persistence.result
```

## 병렬 실행

`filesys/extended` persistence 테스트는 같은 build 디렉터리에서 `tmp.dsk`와 tar output을 공유한다. 이 문서에서는 같은 build 디렉터리의 `make -j`를 안전한 병렬 명령으로 쓰지 않는다.

병렬이 필요하면 `090_병렬_실행_가이드.md`처럼 `/tmp`에 Pintos 작업 복사본을 여러 개 만들고 각 복사본에서 분리 실행한다.

## 결과 확인

```bash
for t in dir-empty-name dir-mk-tree dir-mkdir dir-open dir-over-file dir-rm-cwd dir-rm-parent dir-rm-root dir-rm-tree dir-rmdir dir-under-file dir-vine grow-create grow-dir-lg grow-file-size grow-root-lg grow-root-sm grow-seq-lg grow-seq-sm grow-sparse grow-tell grow-two-files syn-rw symlink-file symlink-dir symlink-link; do
  base="$PINTOS_ROOT/filesys/build/tests/filesys/extended/$t-persistence"
  printf '\n== %s-persistence.result ==\n' "$t"
  cat "$base.result"
done

cat "$PINTOS_ROOT/filesys/build/results" 2>/dev/null || true
```

실패 로그:

```bash
t=grow-sparse-persistence
base="$PINTOS_ROOT/filesys/build/tests/filesys/extended/$t"
cat "$base.output"
cat "$base.errors"
```

## 실패 시 확인할 포인트

- 원본 테스트의 `.result`가 먼저 PASS인지 확인한다.
- persistence `.output`에서 `tar fs.tar /` 단계가 실패했는지 확인한다.
- inode, indirect block, directory entry가 disk에 writeback되는 시점을 확인한다.
- remove된 entry와 open file의 lifetime이 persistence 결과에 잘못 남지 않는지 확인한다.
- symlink target 문자열과 directory metadata가 disk sector에 안정적으로 저장되는지 확인한다.
