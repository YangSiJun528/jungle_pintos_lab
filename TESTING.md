# Pintos 테스트 메모

아래 명령은 Docker 컨테이너 안에서 실행한다. 현재 위치와 상관없이 실행할 수 있도록 Pintos 루트 경로를 먼저 잡는다.

```bash
export PINTOS_ROOT=/workspace/pintos
source "$PINTOS_ROOT/activate"
```

## 전체 테스트 실행

Project 1, threads:

```bash
make -C "$PINTOS_ROOT/threads" clean
make -C "$PINTOS_ROOT/threads" check
```

Project 2, userprog:

```bash
make -C "$PINTOS_ROOT/userprog" clean
make -C "$PINTOS_ROOT/userprog" check
```

Project 3, vm:

```bash
make -C "$PINTOS_ROOT/vm" clean
make -C "$PINTOS_ROOT/vm" check
```

Project 4, filesys:

```bash
make -C "$PINTOS_ROOT/filesys" clean
make -C "$PINTOS_ROOT/filesys" check
```

## 특정 테스트 실행

Project 1의 `alarm-zero`만 실행:

```bash
make -C "$PINTOS_ROOT/threads" build/tests/threads/alarm-zero.result
```

Project 2의 `args-none`만 실행:

```bash
make -C "$PINTOS_ROOT/userprog" build/tests/userprog/args-none.result
```

Project 3의 `pt-grow-stack`만 실행:

```bash
make -C "$PINTOS_ROOT/vm" build/tests/vm/pt-grow-stack.result
```

Project 4의 `sm-create`만 실행:

```bash
make -C "$PINTOS_ROOT/filesys" build/tests/filesys/base/sm-create.result
```

## 전체 결과 확인

`make check`를 실행하면 각 과제 아래에 `build/results`가 생긴다.

```bash
cat "$PINTOS_ROOT/threads/build/results"
```

```bash
cat "$PINTOS_ROOT/userprog/build/results"
```

```bash
cat "$PINTOS_ROOT/vm/build/results"
```

```bash
cat "$PINTOS_ROOT/filesys/build/results"
```

## 참고

- `.result`는 Pintos 실행이 끝난 뒤 checker가 돌아야 만들어진다.
- 컴파일 실패, 커널 패닉, 외부 timeout처럼 테스트 실행 자체가 끝까지 가지 못하면 `.result`가 없을 수 있다.
- `make clean`은 해당 과제의 `build/`를 지운다.
- `make check`가 실패해도 `build/results`, `.result`, `.errors`, `.output`은 남는다.
- 테스트가 도중에 인터럽트되면 해당 테스트의 `.result`가 만들어지지 않을 수 있다.
- 인터럽트된 경우 Make가 작성 중이던 `.output`을 지우고 `.errors`만 남길 수 있다.
