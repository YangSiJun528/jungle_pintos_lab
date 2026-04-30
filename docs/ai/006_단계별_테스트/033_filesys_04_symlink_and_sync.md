# filesys 04: symlink and synchronization

이 단계는 symlink와 file system synchronization 관련 extended 테스트를 확인한다. 모든 명령은 Docker 컨테이너 안에서 실행한다.

## 대상 기능

- symbolic link 생성과 path resolution
- symlink가 file과 directory를 가리킬 때의 처리
- symlink chain과 loop 방어
- 여러 process의 read/write 동기화
- directory/file operation 중 lock ordering

## 참고한 reference 문서

- `docs/reference/pintos-kaist-kr/project4/subdirectories.md`
- `docs/reference/pintos-kaist-kr/project4/synchronization.md`
- `pintos/tests/filesys/extended/Make.tests`
- `pintos/tests/filesys/extended/Rubric.functionality`

## 공통 준비

```bash
export PINTOS_ROOT=/workspace/pintos
source "$PINTOS_ROOT/activate"
```

## 순차 실행 명령

```bash
make -C "$PINTOS_ROOT/filesys" clean
make -C "$PINTOS_ROOT/filesys" \
  build/tests/filesys/extended/symlink-file.result \
  build/tests/filesys/extended/symlink-dir.result \
  build/tests/filesys/extended/symlink-link.result \
  build/tests/filesys/extended/syn-rw.result
```

## 병렬 실행

`filesys/extended` 테스트는 같은 build 디렉터리에서 `tmp.dsk`를 공유한다. 이 문서에서는 같은 build 디렉터리의 `make -j`를 안전한 병렬 명령으로 쓰지 않는다.

병렬이 필요하면 `090_병렬_실행_가이드.md`처럼 `/tmp`에 Pintos 작업 복사본을 여러 개 만들고 각 복사본에서 분리 실행한다.

## 결과 확인

```bash
for t in symlink-file symlink-dir symlink-link syn-rw; do
  base="$PINTOS_ROOT/filesys/build/tests/filesys/extended/$t"
  printf '\n== %s.result ==\n' "$t"
  cat "$base.result"
  printf '== %s.errors ==\n' "$t"
  cat "$base.errors"
done

cat "$PINTOS_ROOT/filesys/build/results" 2>/dev/null || true
```

실패 로그:

```bash
t=symlink-link
base="$PINTOS_ROOT/filesys/build/tests/filesys/extended/$t"
cat "$base.output"
cat "$base.errors"
```

## 실패 시 확인할 포인트

- symlink inode가 target path 문자열을 안정적으로 저장하는지 확인한다.
- 마지막 path component에서 symlink를 따라갈지 말지 syscall별 정책을 확인한다.
- symlink depth 제한을 두어 cycle에서 무한 recursion이 나지 않는지 확인한다.
- `syn-rw` 실패는 inode lock, file position 공유 여부, directory lock 순서를 본다.
- lock을 잡은 상태에서 user memory 접근이나 blocking I/O로 deadlock을 만들지 않는지 확인한다.
