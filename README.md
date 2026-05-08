# Pintos Lab Personal

개인 Pintos 학습과 실습을 위한 작업 공간입니다.

크래프톤 정글 12기 Pintos 프로젝트를 기반으로 합니다.

팀 fork의 Git 히스토리와 IDE 전용 설정을 가져오지 않고, 깨끗한 Pintos 베이스 위에 개인 학습 자료와 한글 주석을 선별해 새 히스토리로 구성했습니다.

팀 레포지토리:
- 1~2주차: https://github.com/NearthYou/pintos_lab
- 3~4주차: https://github.com/giteunyeol/PIntos_VM_Week11_12

CLion은 로컬 IDE로 사용하고, 빌드와 실행은 Docker 컨테이너 내부 CLI에서 Pintos 기존 Makefile로 수행합니다.
루트 `CMakeLists.txt`와 `pintos/CMakeLists.txt`는 코드 색인 전용입니다.

## Docker CLI 실행

이미지는 저장소 루트에서 한 번 빌드합니다.

```bash
docker build --platform=linux/amd64 -t pintos-dev:22.04 .
```

컨테이너 셸로 들어갑니다.

```bash
docker run -it --name pintos-dev --platform=linux/amd64 \
  -v "$PWD:/workspace" \
  -w /workspace \
  pintos-dev:22.04 bash
```

컨테이너 내부에서 환경을 활성화합니다. 새 bash 셸에서는 자동으로 source되지만, 필요하면 직접 실행합니다.

```bash
source pintos/activate
```

컨테이너 셸을 나갈 때는 `exit`를 입력하거나 `Ctrl-D`를 누릅니다. 나간 뒤 같은 컨테이너에 다시 들어가려면 아래처럼 실행합니다.

```bash
docker start -ai pintos-dev
```

## 빌드와 테스트

전체 테스트:

```bash
cd /workspace/pintos/threads
make clean
make check
```

특정 테스트:

```bash
cd /workspace/pintos/threads
make clean
make build/tests/threads/alarm-zero.result
```

프로젝트별 예시:

```bash
cd /workspace/pintos/threads
make clean
make build/tests/threads/alarm-zero.result

cd /workspace/pintos/userprog
make clean
make build/tests/userprog/args-none.result

cd /workspace/pintos/vm
make clean
make build/tests/vm/pt-grow-stack.result

cd /workspace/pintos/filesys
make clean
make build/tests/filesys/base/sm-create.result
```

`make clean`은 각 프로젝트의 `build/`를 지웁니다. 이전 실행의 판정 파일이 남아 있지 않게 하려면 테스트 전에 실행합니다.

## 결과 파일

결과는 `pintos/<project>/build/tests/...` 아래에 생성됩니다.

- `.output`: Pintos 실행 출력
- `.errors`: 실행 중 표준 에러
- `.result`: 채점 결과. 통과하면 `PASS`
- `results`: 전체 테스트 요약. `make check`가 출력하는 `pass`/`FAIL` 목록

예시:

```bash
cat /workspace/pintos/threads/build/tests/threads/alarm-zero.result
cat /workspace/pintos/threads/build/tests/threads/alarm-zero.output
cat /workspace/pintos/threads/build/tests/threads/alarm-zero.errors
cat /workspace/pintos/threads/build/results
```
