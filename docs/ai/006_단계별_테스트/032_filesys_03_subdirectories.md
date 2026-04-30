# filesys 03: subdirectories

이 단계는 subdirectory, current directory, path handling을 확인한다. 모든 명령은 Docker 컨테이너 안에서 실행한다.

## 대상 기능

- `mkdir`, `chdir`, `readdir`, `isdir`, `inumber`
- absolute path와 relative path 처리
- `.`, `..` directory entry 처리
- directory remove 조건
- directory와 file 이름 충돌 방어
- current working directory가 process별로 유지되는지 확인

## 참고한 reference 문서

- `docs/reference/pintos-kaist-kr/project4/subdirectories.md`
- `docs/reference/pintos-kaist-kr/project4/synchronization.md`
- `pintos/tests/filesys/extended/Make.tests`
- `pintos/tests/filesys/extended/Rubric.functionality`
- `pintos/tests/filesys/extended/Rubric.robustness`

## 공통 준비

```bash
export PINTOS_ROOT=/workspace/pintos
source "$PINTOS_ROOT/activate"
```

## 순차 실행 명령

```bash
make -C "$PINTOS_ROOT/filesys" clean
make -C "$PINTOS_ROOT/filesys" \
  build/tests/filesys/extended/dir-empty-name.result \
  build/tests/filesys/extended/dir-mk-tree.result \
  build/tests/filesys/extended/dir-mkdir.result \
  build/tests/filesys/extended/dir-open.result \
  build/tests/filesys/extended/dir-over-file.result \
  build/tests/filesys/extended/dir-rm-cwd.result \
  build/tests/filesys/extended/dir-rm-parent.result \
  build/tests/filesys/extended/dir-rm-root.result \
  build/tests/filesys/extended/dir-rm-tree.result \
  build/tests/filesys/extended/dir-rmdir.result \
  build/tests/filesys/extended/dir-under-file.result \
  build/tests/filesys/extended/dir-vine.result
```

## 병렬 실행

`filesys/extended` 테스트는 같은 build 디렉터리에서 `tmp.dsk`를 공유한다. 이 문서에서는 같은 build 디렉터리의 `make -j`를 안전한 병렬 명령으로 쓰지 않는다.

병렬이 필요하면 `090_병렬_실행_가이드.md`처럼 `/tmp`에 Pintos 작업 복사본을 여러 개 만들고 각 복사본에서 분리 실행한다.

## 결과 확인

```bash
for t in dir-empty-name dir-mk-tree dir-mkdir dir-open dir-over-file dir-rm-cwd dir-rm-parent dir-rm-root dir-rm-tree dir-rmdir dir-under-file dir-vine; do
  base="$PINTOS_ROOT/filesys/build/tests/filesys/extended/$t"
  printf '\n== %s.result ==\n' "$t"
  cat "$base.result"
done

cat "$PINTOS_ROOT/filesys/build/results" 2>/dev/null || true
```

실패 로그:

```bash
t=dir-mk-tree
base="$PINTOS_ROOT/filesys/build/tests/filesys/extended/$t"
cat "$base.output"
cat "$base.errors"
```

## 실패 시 확인할 포인트

- path parser가 `/`, repeated slash, empty component를 어떻게 처리하는지 확인한다.
- directory open count와 removed flag 때문에 cwd 삭제가 잘못 허용/거부되지 않는지 확인한다.
- `.`과 `..` entry를 생성하고 traversal에서 정확히 해석하는지 확인한다.
- file inode와 directory inode를 구분하는 metadata가 있는지 확인한다.
- process fork/exec 이후 current directory reference가 누수되거나 조기 close되지 않는지 확인한다.
