# Pintos userprog 단계 AI 의존도 평가

이 문서는 Pintos Project 2: User Programs 단계에서 AI에게 던진 프롬프트와 AI 산출물이 실제 개발에 얼마나 영향을 줬는지 회고한 것이다. 형식과 점수 기준은 `docs/ai/005_thread_단계_AI_의존도_평가.md`를 따른다.

평가는 `docs/daily`, `docs/ai`에 남은 메모와 AI 산출물, git commit 이력, `~/.codex/history.jsonl`에 남은 대표 프롬프트 일부를 근거로 한다. 구현 요구사항 판단은 로컬 reference인 `docs/reference/pintos-kaist-original/2_project2/*`, 보조로 `docs/reference/pintos-kaist-kr/2_project2/*`와 `pintos/tests/userprog/*` 문서를 기준으로 한다.

## 결론

전체 의존도는 **높은 편, 5점 만점에 4.1점**으로 평가한다.

Thread 단계의 3.4점보다 높다. Thread 단계에서는 AI가 설계 검토와 디버깅 힌트에 크게 기여했지만, 구현 자체는 직접 수행했다는 기록이 더 강했다. 반면 userprog 단계에는 "시간 관계 상, AI를 써서 구현하고, 내 스타일에 바꾸는 식으로 어제까지 작업함", "process.c는 AI 많이 씀", "ref_cnt 쓰는거 생각도 못했음" 같은 직접 기록이 남아 있다.

다만 5점은 아니다. 모든 코드를 통째로 위임한 단계라기보다는, 요구사항 분석과 작업 분해를 AI에 강하게 의존하고, 막힌 구현 영역에서는 AI가 제시한 구조나 코드를 받아 내 스타일로 다시 정리한 형태에 가깝다. 특히 `argument_passing`, 기본 file syscall 일부는 직접 삽질과 테스트 해석 흔적이 많고, daily 메모에도 "일단 한 5개정도 시스템 콜 만들었는데, 이거는 AI 안 씀"이라는 기록이 있다.

결론적으로 이번 userprog 단계는 **직접 구현과 AI 보조의 경계가 Thread 단계보다 더 흐려진 개발 단계**다. 요구사항 정리와 디버깅 보조는 명확히 학습 보조였지만, `fork/exec/wait`, process lifecycle, cleanup, 동시성/file lock 쪽은 AI가 구현 방향과 코드 구조에 직접적인 영향을 줬다.

## 평가 기준

| 점수 | 의미 |
| --- | --- |
| 0 | AI 사용 흔적 없음 |
| 1 | 명령어, 문법, 자료 위치 확인 정도 |
| 2 | 개념 설명, 코드 읽기 보조, 단순 검증 |
| 3 | 구현 방향이나 테스트 전략에 실질적 영향 |
| 4 | 막힌 버그의 원인 후보를 AI가 크게 좁힘 |
| 5 | 핵심 설계나 코드 대부분을 AI 산출물에 의존 |

이 점수는 "학습에 나쁜가"가 아니라 **해당 작업을 진행하는 데 AI가 얼마나 결정적인 역할을 했는가**를 뜻한다.

## 기준 자료

주요 근거는 다음이다.

- `docs/daily/2026-05-01.md`: Project 2 시작, argument passing 구현과 디버깅, AI 힌트 기록
- `docs/daily/2026-05-02.md`: syscall dispatch, ABI, user memory/PML4 이해, AI 문서화 기록
- `docs/daily/2026-05-03.md`: user memory와 VM 구조 이해, "AI랑 씨름" 기록
- `docs/daily/2026-05-04.md`: `exec/fork/wait` 요구사항 분석 AI 요청, `020` 문서 생성 기록
- `docs/daily/2026-05-05.md`: file syscall 일부 직접 구현 기록, fork/exec/wait 착수 기록
- `docs/daily/2026-05-06.md`: AI를 써서 구현 완료, ref_cnt와 리팩토링 기록
- `docs/ai/008_Pintos_Project2_userprog_우선순위와_분담.md`
- `docs/ai/013_System_Calls_infra와_user_memory_요구사항.md`
- `docs/ai/014_System_V_AMD64_ABI와_system_call_흐름.md`
- `docs/ai/015_Project2_user_memory와_PML4_핵심.md`
- `docs/ai/020_Project2_system_call_구현_우선순위.md`
- `docs/ai/021_fd_table과_close_on_exit_요구사항.md`
- `docs/ai/022_filesys와_file_API_가벼운_명세.md`
- `docs/ai/023_fork_exec_wait_구현_작업_정리.md`
- `~/.codex/history.jsonl`의 대표 프롬프트
- git commit 이력의 userprog 관련 커밋

## 프롬프트 유형 분석

### 1. 작업 우선순위와 4인 분담 계획

`docs/ai/008_Pintos_Project2_userprog_우선순위와_분담.md`는 Project 2 전체 흐름을 기능 단위로 나누고, 경험 있는 2명과 경험 적은 2명을 기준으로 분담안을 만든 문서다.

이 문서는 코드 구현은 아니지만, userprog 진행 순서에 강한 영향을 줬다. daily 기록에도 "AI가 정해준 분업 작업 기준으로는 `syscall_entry`/dispatch 를 만들어야 함", "일단 구현 우선순위 AI한테 레퍼런스 기반으로 요청함"이라고 남아 있다.

따라서 이 영역은 단순 문서화가 아니라 **개발 순서와 협업 구조를 정하는 의사결정 보조**에 해당한다.

### 2. argument passing 이해와 디버깅

5월 1일 daily에는 argument passing을 직접 구현하면서 AI에게 여러 번 도움을 받은 흔적이 길게 남아 있다.

대표적으로 다음 지점이다.

- `file_name`이 사실상 command line인지 확인
- `strtok_r()`와 문자열 pointer 배열 처리 방식 이해
- kernel stack에 큰 local array를 두면 `thread_current()` assertion panic이 날 수 있다는 힌트
- `argv` 마지막을 `NULL`로 두는 C 문법
- page fault 원인 분석
- `hex_dump()` 출력 해석
- 포인터 연산으로 stack에 값을 써야 하는 지점 확인

메모에는 "AI한테 기법 물어보니까 코드 나옴????", "의존도가 좀 높아진거 같은데 어쩔수 없긴 함", "그냥 구현해달라하기 진짜 모르겠음" 같은 표현도 있다. 즉 argument passing은 직접 시행착오가 많았지만, 막힌 지점에서 AI가 원인 후보와 구현 패턴을 꽤 직접적으로 좁혀 줬다.

의존도는 높지만, 완전히 위임한 영역은 아니다. hexdump를 직접 찍고, 출력과 stack layout을 계속 비교했으며, 최종적으로 "내가 다 구현하거나 하는게 목표"라는 방향도 남아 있다.

### 3. syscall dispatch와 ABI 이해

`docs/ai/014_System_V_AMD64_ABI와_system_call_흐름.md`는 `%rax`, `%rdi`, `%rsi`, `%rdx`, `%r10`, `%r8`, `%r9` 같은 register 기반 syscall argument 흐름과 `intr_frame->rax` 반환값 처리를 이해하기 위한 문서다.

이 영역의 AI 활용은 구현 코드 제공보다는 **개념과 ABI 연결 설명**에 가깝다. 다만 `syscall_handler`가 어떤 식으로 syscall number와 arguments를 읽어야 하는지에 직접 연결되므로 구현 방향에는 실질적인 영향을 줬다.

### 4. user memory와 PML4 개념 이해

5월 2일과 5월 3일 기록은 user memory/PML4 이해에 AI 의존이 높았음을 보여 준다. daily에는 "글도 보고, 유튜브 자료도 보는데, 아니 이해가 하나도 안됨", "AI한테 처음에 분석해달라 한거 보면서 작업하면 될 듯?", "html로 분석 요청했다가 하루종일 AI랑 씨름함"이라고 남아 있다.

`docs/ai/015_Project2_user_memory와_PML4_핵심.md`, `017`, `018`, `019` 계열 문서는 Project 2에서 필요한 user/kernel virtual memory, `KERN_BASE`, `pml4`, `upage`, `kpage` 이해를 정리한 산출물이다.

이 부분은 과제 구현 코드를 직접 생성한 것은 아니지만, user pointer 검증 helper와 bad pointer/boundary 테스트 대응 방식에 중요한 배경이 되었다. 의존도는 높다.

### 5. user memory robustness와 exception 처리

history에는 bad pointer, boundary, bad read/write/jump 계열 테스트 분석 요청이 남아 있다. 이후 "나 이거 유효성 검사 충분히 잘 하고 있는 줄 알았는데, 다 실패함. 이유가 뭐야?"라는 프롬프트도 있다.

이 영역은 AI가 단순 개념 설명을 넘어 실패 테스트 묶음의 원인 후보를 좁혀 준 영역이다. `614d610 fix(userprog): user memory robustness와 process cleanup 처리` 커밋은 `exception.c`, `process.c`, `syscall.c`, `thread.h`까지 건드리므로 영향 범위도 넓다.

### 6. fd table과 file syscall

`docs/ai/021_fd_table과_close_on_exit_요구사항.md`, `docs/ai/022_filesys와_file_API_가벼운_명세.md`는 fd table, close-on-exit, `filesys.h`, `file.h` API를 정리한 문서다.

다만 daily에는 "일단 한 5개정도 시스템 콜 만들었는데, 이거는 AI 안 씀"이라고 남아 있다. 따라서 file syscall 전체를 AI가 구현했다고 보기는 어렵다.

이 영역의 AI 의존은 주로 **요구사항 확인, fd 의미 확인, API 명세 정리, open/remove semantics 확인**에 있었다. 구현 주체는 상대적으로 직접성이 더 강하다.

### 7. fork/exec/wait와 process lifecycle

userprog 단계에서 AI 의존도가 가장 높은 영역이다.

history에는 "fork/exec/wait 구현하려면 뭘 하면 되나요? 현재 코드 기준으로 어떤 작업을 수행해야 할지, 레퍼런스 기반으로 말해주세요.", "pintos/userprog/process.c 보고 process_fork와 wait이 어떤 식으로 구현되었는지 분석해서 설명해주세요. 추가된 구조체들도" 같은 프롬프트가 남아 있다.

브랜치 커밋 `747202b`의 메시지는 "대충하긴 했는데 process.c는 AI 많이 씀"이다. 이 커밋은 main의 `4bfbf95 feat(userprog): fork와 wait 상태 관리 처리`와 대응되는 작업 흐름으로 보이며, `process.c`, `syscall.c`, `thread.c`, `thread.h`에 걸친 핵심 process lifecycle 변경이다.

또한 5월 6일 daily의 "AI를 써서 구현하고, 내 스타일에 바꾸는 식", "ref_cnt 쓰는거 생각도 못했음" 기록은 child/process 상태 수명 관리와 reference count 성격의 구조가 AI에게서 온 영향이 크다는 근거다.

### 8. 동시성, file lock, cleanup 리팩토링

마지막 안정화 단계도 AI 의존이 높다.

`7f0d5c8 fix(userprog): 파일시스템 락 적용해서 테스트 성공하게 수정`은 `filesys.c`, `filesys.h`, `synch.c`, `thread.c`, `process.c`, `syscall.c`를 건드렸다. `9f37ea1 테스트 성공하게 수정` 브랜치 커밋도 같은 작업 흐름에 있다.

5월 6일 daily에는 "테스트는 성공하지만 동시성 등의 이슈가 있을 수 있으므로 AI 평가 맡기고 처리중"이라고 적혀 있다. 즉 public test 통과 이후에도 동시성/cleanup 리스크를 AI에게 리뷰시키고 수정한 것으로 보인다.

## 대표 JSONL 프롬프트 근거

아래는 `~/.codex/history.jsonl`에서 userprog 단계와 직접 관련 있는 대표 프롬프트 일부다. 전체 세션이 아니라 의존도 판단에 영향을 준 유형별 사례만 골랐다. 원문 오탈자는 유지했고, 지나치게 긴 테스트 명령은 `...`로 줄였다.

```jsonl
{"session_id":"019de139-cac0-7a72-833c-71556cf00f42","ts":1777607368,"text":"이 기준으로 테스트 진행한다는데, 문서 참고해서 어떤게 적절한지 평가하고. \n과제부터 어떤 순서로, 풀어야 허떤 테스트가 통과하고, 작업을 4명이서 나누는데, 개발 경험 있는 2명, 경험 적은 2명을 기준으로 한다면 어떻게 하는게 좋을지도 알려주세요."}
{"session_id":"019de712-a24b-72a0-91e4-41cc74d036bf","ts":1777700507,"text":"System Calls infra 구축, user memory 관리 를 위한 요구사항을 정리. 013번 문서를 생성해주세요.\n\n코드 구현이나 구체적인 구현 힌트를 제공하지 마세요.\n\n당신의 역할은 단순히 요구사항을 읽기 좋게 취합하여 정리해서 제공해주는 것 뿐입니다."}
{"session_id":"019de712-a24b-72a0-91e4-41cc74d036bf","ts":1777703621,"text":"x86-64 convention(관례)에서 function return value(함수 반환값)는 RAX register에 둡니다. 값을 return하는 system call은 struct intr_frame의 rax member(멤버)를 수정하여 값을 반환할 수 있습니다.\n\n이게 무슨 말인가요? x86-64 convention 관점에서 설명. pintos 구현이 아니라"}
{"session_id":"019de868-a4b7-75f2-98ee-b1263ab92e64","ts":1777720687,"text":"userprog\n\n*-bad-ptr, *-boundary, bad-read, bad-write, bad-jump 계열\n\n테스트코드 분석하기. 어떤 요구사항이 있는지"}
{"session_id":"019df21c-3c24-76e0-b13f-ace8f259a927","ts":1777883817,"text":"생각해보니 시스템 콜 exit이랑 write(1) (최소)를 먼저 해야 테스트도 가능하고 동작을 할 수 있는거 같음. 이거부터 하게 이거에 대한 요구사항을 레퍼런스 기반으로 정리해서 020번 문서로 작성"}
{"session_id":"019df21c-3c24-76e0-b13f-ace8f259a927","ts":1777905302,"text":"이거 fd 0하고 1이 뭐더라?"}
{"session_id":"019df69c-52dc-7821-ae0f-c41fbb716cff","ts":1777963962,"text":"이러면 일단 동시성 & fork/exec/wait 빼고는 다 구현함.\n\n이거 테스트 하려면 fork를 먼저 해야하는지 아니면 지금 상태로도 충분한지 확인하고\n\n만약 테스트 가능하면 반복하면서 구현 & 동시성 문제 수정하고 아니면 기능 개발한 후 테스트해야할 듯?"}
{"session_id":"019df703-607d-7db1-905b-1affa2776c41","ts":1777965716,"text":"fork/exec/wait 구현하려면 뭘 하면 되나요? 현재 코드 기준으로 어떤 작업을 수행해야 할지, 레퍼런스 기반으로 말해주세요."}
{"session_id":"019df355-59e5-7300-b715-789c050a3642","ts":1777977869,"text":"pintos/userprog/process.c 보고 process_fork와 wait이 어떤 식으로 구현되었는지 분석해서 설명해주세요. 추가된 구조체들도"}
{"session_id":"019df89f-0180-7653-8d62-5aabc634429f","ts":1777996021,"text":"이것들 실패하는데 이유가 뭘까?cat \"$PINTOS_ROOT/userprog/build/tests/userprog/exec-boundary.result\"\ncat \"$PINTOS_ROOT/userprog/build/tests/userprog/exec-missing.result\"\ncat \"$PINTOS_ROOT/userprog/build/tests/userprog/exec-bad-ptr.result\"\ncat \"$PINTOS_ROOT/userprog/build/tests/userprog/exec-read.result\"\ncat \"$PINTOS_ROOT/userprog/build/tests/userprog/wait-simple.result\"\ncat \"$PINTOS_ROOT/userprog/build/tests/userprog/wait-twice.result\"\ncat \"$PINTOS_ROOT/userprog/build/tests/userprog/wait-killed.result\"\ncat \"$PINTOS_ROOT/userprog/build/tests/userprog/wait-bad-pid.result\"\ncat \"$PINTOS_ROOT/userprog/build/tests/userprog/multi-recurse.result\""}
{"session_id":"019df89f-0180-7653-8d62-5aabc634429f","ts":1777998947,"text":"나 이거 유효성 검사 충분히 잘 하고 있는 줄 알았는데, 다 실패함. 이유가 뭐야? make -C \"$PINTOS_ROOT/userprog\" clean\nmake -j\"$(nproc)\" -C \"$PINTOS_ROOT/userprog\" ..."}
{"session_id":"019dfc61-fbe3-7a20-85ee-16a7b1e525e1","ts":1778056584,"text":"발표 내용 수정 대충 이런 느낌이고, 1분 30초 내로 진행해야 함. ... 7. 단점? - 의사결정에 AI 의존도 높아짐. 일부지만 코드 출력해주기도 함(작성은 안함) - 다만 이 부분은 빠른 개발을 위해서 선택한 영역이므로 감수할만 했음."}
```

이 원문들은 userprog 단계의 AI 사용이 단순 질의응답을 넘어섰음을 보여 준다. 특히 작업 분담, 요구사항 문서화, 테스트 묶음 분석, `process.c` 구조 분석, 실패 원인 분석, 구현 이후 리스크 평가까지 AI가 반복적으로 개입했다.

## 영역별 의존도

| 영역 | 의존도 | 근거 | 평가 |
| --- | ---: | --- | --- |
| Project 2 전체 우선순위와 분담 | 3.5 | `008` 문서, 5/1 daily, JSONL 2466 | 개발 순서와 팀 분담 기준에 실질적 영향을 줬다. |
| argument passing | 3.7 | 5/1 daily의 AI 힌트, hexdump 해석, kernel stack panic 분석 | 직접 구현 흔적도 강하지만, 막힌 지점에서 AI가 원인과 구현 패턴을 크게 좁혔다. |
| syscall dispatch/ABI | 3 | `014` 문서, 5/2 daily, JSONL 2564~2565 | 구현보다 개념 연결 의존. 그래도 dispatch 구조 이해에 영향이 있다. |
| user memory/PML4 개념 | 4 | `015`, `017`~`019`, 5/2~5/3 daily | 이해 난도가 높아 AI 정리와 시각화 의존이 컸다. |
| user memory robustness | 4.2 | bad pointer/boundary 테스트 분석, `614d610` | 실패 묶음의 원인 후보와 처리 방향을 AI가 크게 좁혔다. |
| 기본 process syscall | 2.5 | `23a17b6`, 5/4 daily | `exit`, `write` 우선순위는 AI 문서 영향이 있지만 구현 의존은 중간 이하로 보인다. |
| file syscall/fd table | 3.2 | `021`, `022`, 5/5 daily | 요구사항/API 정리 의존은 높지만, file syscall 일부는 직접 구현 기록이 있다. |
| fork/exec/wait/process lifecycle | 4.6 | `023`, `4bfbf95`, 브랜치 `747202b`, 5/6 daily | 가장 높은 의존 영역. `process.c` 구조와 수명 관리에 AI 영향이 직접 남아 있다. |
| cleanup/multi-oom/rox | 4 | `614d610`, `773b070`, 5/6 daily | 테스트 통과와 resource cleanup 안정화에 AI 리뷰/힌트가 컸다. |
| file system lock/동시성 | 4.2 | `7f0d5c8`, `9f37ea1`, 5/6 daily | public test 이후 동시성 위험 평가와 lock 적용 방향에 AI가 개입했다. |
| 테스트 전략 | 3.5 | `006_단계별_테스트`, JSONL 2824~2828 | 테스트 묶음과 확인 순서를 AI가 정리해 피드백 루프를 줄였다. |
| 발표/회고 문서화 | 4 | `024` 발표자료, JSONL 2892 | AI 사용 방식 자체를 설명하는 발표자료도 AI 기반으로 구성했다. |
| 최종 판단/스타일 반영 | 3 | 5/6 daily의 "내 스타일에 바꾸는 식", style skill 사용 | AI 산출물을 그대로 둔 것은 아니고, 직접 스타일과 판단으로 다시 정리했다. |

## 커밋별 평가

| 커밋 | 작업 | AI 의존도 | 판단 |
| --- | --- | ---: | --- |
| `8e5a7db` docs: ai 008 userprog 실행 계획 | Project 2 우선순위와 분담 계획 | 3.5 | 작업 순서와 협업 구조를 AI가 정리했다. |
| `9c91cc0`, `0b946a1`, `2ea71c3` docs: 학습/이해 목적의 AI 자료 추가 | argument passing, syscall, user memory 학습 문서 | 3.5 | 구현보다 학습 문서지만 개발 방향을 잡는 데 영향이 컸다. |
| `6d96b0c` feat(userprog): argument passing 기능 구현 | command line parsing과 user stack 구성 | 3.7 | 직접 구현과 hexdump 검증이 있지만, AI 힌트와 코드 예시 영향도 명확하다. |
| `1e9087e` refactor(userprog): argument passing | argument passing 정리 | 2.5 | 이미 잡은 구현을 다듬는 성격. |
| `0b979b0` feat(userprog): system call dispatch | syscall dispatch 구조 | 3 | ABI/`intr_frame` 이해 문서와 대화의 영향을 받았다. |
| `d3aa943` feat(userprog): user memory valid 헬퍼 함수 추가 | user pointer validation helper | 3.8 | user memory/PML4 문서와 bad pointer 요구사항 정리 영향이 크다. |
| `fab66c7` fix(userprog): user memory valid 헬퍼 무한루프 버그 수정 | validation helper 버그 수정 | 4 | 막힌 버그 원인 축소에 AI 도움을 받은 흐름과 겹친다. |
| `23a17b6` feat(userprog): 기본 process syscall 처리 | `exit`, `write` 등 기본 process syscall | 2.5 | 우선순위는 AI 문서 기반이나, 직접 구현 흔적이 비교적 강하다. |
| `9b466e4` feat(userprog): fd 기반 파일 syscall 처리 | fd table과 file syscall | 3.2 | `021`, `022` 문서 영향이 크지만, daily에 일부 syscall 직접 구현 기록이 있다. |
| `286ece5` fix(userprog): fd 정리와 exec 상태 다듬기 | fd cleanup, exec 상태 정리 | 4 | fd/process lifecycle 경계 작업으로 AI 분석 의존이 높다. |
| `4bfbf95` feat(userprog): fork와 wait 상태 관리 처리 | fork/wait state와 process lifecycle | 4.6 | 브랜치 커밋 `747202b`의 "process.c는 AI 많이 씀" 기록과 직접 연결된다. |
| `614d610` fix(userprog): user memory robustness와 process cleanup 처리 | robustness와 cleanup | 4.2 | bad pointer, exception, cleanup 실패 원인 분석에 AI 의존이 높다. |
| `7f0d5c8` fix(userprog): 파일시스템 락 적용해서 테스트 성공하게 수정 | file system lock과 동시성 안정화 | 4.2 | 테스트 성공 직전 리스크 평가와 수정 방향을 AI가 크게 도왔다. |
| `1901ecd`, `9a9bc12` docs: 10주차 발표자료 | AI/AGENTS 발표자료 | 4 | 발표자료 자체가 history와 AI 사용 방식 분석을 AI에 맡긴 산출물이다. |

브랜치 `userprog/idkwtd`의 임시 커밋들도 중요하다.

| 커밋 | 작업 | AI 의존도 | 판단 |
| --- | --- | ---: | --- |
| `747202b` 대충하긴 했는데 process.c는 AI 많이 씀 | fork/wait 관련 process.c 중심 변경 | 4.7 | 커밋 메시지 자체가 AI 구현 의존을 직접 인정한다. |
| `773b070` 테스트 다 통과하긴 함 | robustness/cleanup 후속 안정화 | 4 | 테스트 통과까지 AI 도움을 받은 흐름으로 보인다. |
| `9f37ea1` 테스트 성공하게 수정 | file lock 포함 최종 안정화 | 4.2 | main의 `7f0d5c8`과 같은 성격의 안정화 작업이다. |

## 의존도가 높았던 지점

### fork/exec/wait와 process.c

가장 높은 의존 영역이다. `fork`, `wait`, `exec`는 reference 요구사항 자체도 많고, parent-child 관계, child status 수명, address space 복제, fd 복제, exit cleanup이 모두 얽힌다.

이 영역에서는 AI가 "무엇을 해야 하는가"를 정리한 수준을 넘어, 현재 코드의 `process_fork`, `process_wait`, 추가 구조체를 분석하고, 상태 이름과 수명 관리까지 같이 검토했다. `ref_cnt` 아이디어를 AI가 제공한 것으로 보이는 daily 기록도 있다.

따라서 이 영역은 **학습 보조와 구현 보조의 경계가 가장 흐려진 영역**이다.

### user memory robustness

처음에는 "포인터가 user 영역인지, mapping되어 있는지 검사하면 된다" 정도로 이해가 정리되었지만, 실제 테스트는 bad pointer, boundary, bad jump/read/write처럼 훨씬 넓게 걸렸다.

AI는 테스트 묶음을 분석하고, pointer/string/buffer 검증, invalid pointer 처리, kernel panic 방지, exception 처리 방향을 좁히는 데 기여했다. 특히 "유효성 검사 충분히 잘 하고 있는 줄 알았는데 다 실패함" 이후의 흐름은 단순 문법 질문이 아니라 실패 원인 분석에 해당한다.

### 동시성과 file system lock

마지막에는 "테스트는 성공하지만 동시성 등의 이슈가 있을 수 있으므로 AI 평가 맡기고 처리중"이라는 기록이 있다. 즉 public test 통과 이후에도 file system lock과 cleanup 위험을 AI에게 점검시켰다.

`7f0d5c8`은 file system lock을 적용해 테스트를 성공시킨 커밋이고, 영향 파일도 `filesys`, `thread`, `process`, `syscall`에 걸쳐 있다. 이 정도면 단순 리뷰가 아니라 안정화 방향에 AI가 꽤 직접 관여한 것으로 보는 게 맞다.

## 의존도가 낮았던 지점

기본 file syscall 일부는 상대적으로 낮다. 5월 5일 daily에 "일단 한 5개정도 시스템 콜 만들었는데, 이거는 AI 안 씀"이라고 직접 기록되어 있다.

또한 `argument_passing`은 AI 힌트가 많았지만, hexdump를 직접 찍고 stack layout을 직접 비교하며 밤새 디버깅한 흔적도 강하다. 의존도는 높지만, 완전 위임으로 보기는 어렵다.

단순 스타일 정리, 문서 보완, 이름 다듬기, 테스트 결과 확인 문서 수정도 핵심 구현 의존도와는 분리해야 한다.

## 학습 관점 평가

좋았던 점은 다음과 같다.

- AI에게 reference 기반 요구사항 정리만 요구한 프롬프트가 많다.
- `AGENTS.md`를 통해 외부 Pintos 해답이나 코드 생성 의존을 줄이려는 장치를 만들었다.
- daily 문서에 "AI를 쓴 지점", "AI 안 쓴 지점", "의존도가 높아진 느낌"을 비교적 솔직하게 남겼다.
- AI 답변을 바로 정답으로 두지 않고, 로컬 reference와 테스트 파일, hexdump, `.result`를 함께 확인했다.
- AI 산출물을 "내 스타일에 바꾸는 식"으로 다시 정리했다.

위험했던 점은 다음과 같다.

- 시간 압박 때문에 "요구사항 분석"을 넘어 구현 구조까지 AI에 맡긴 구간이 있다.
- `process.c`, child/process lifecycle, ref count, cleanup 같은 핵심 설계에 AI 영향이 직접 남았다.
- argument passing 중 "그냥 구현해달라하기"처럼 금지 경계에 가까운 프롬프트가 있었다.
- 테스트 실패 원인 분석을 AI에게 반복적으로 맡기면서 독립적인 디버깅 훈련량이 줄었을 수 있다.
- AI가 코드를 일부 출력했다는 회고가 있어, 이후에는 어떤 코드를 실제로 사용했는지 더 명확히 기록할 필요가 있다.

## 다음 단계에서의 사용 기준

VM이나 Filesystem 단계에서는 AI 사용을 다음처럼 제한해서 기록하는 편이 좋다.

| 단계 | 허용할 사용 | 기록할 것 |
| --- | --- | --- |
| Green | reference 요약, 테스트 위치 확인, 개념 설명 | 프롬프트 요약만 기록 |
| Yellow | 설계 대안 비교, 테스트 실패 원인 후보 요청 | 내가 먼저 세운 가설과 AI가 바꾼 판단 |
| Red | 현재 코드 기반 구조 설계, 특정 버그 원인 지목, 패치/코드 출력 | 원문 프롬프트, AI 응답 핵심, 실제 반영 여부 |

특히 Red 단계는 금지가 아니라 **학습 의존도가 높아지는 순간**으로 관리해야 한다. 다음부터는 AI에게 묻기 전에 최소한 아래를 먼저 남기는 것이 좋다.

1. 내가 읽은 reference 위치
2. 내가 세운 원인 후보 2개 이상
3. 어떤 테스트 또는 로그가 그 후보를 지지/반박하는지
4. AI 답변 중 실제 코드에 반영한 내용
5. 반영하지 않은 내용과 이유

이렇게 하면 AI를 써도 "AI가 알려준 대로 했다"가 아니라 "AI가 제시한 후보를 내가 검증했다"로 남길 수 있다.

## 최종 평가

이번 Pintos userprog 단계는 **AI 보조를 많이 받은 직접 구현 프로젝트**에서 한 단계 더 나아가, 일부 핵심 영역에서는 **AI 구현 보조를 받아 완성한 프로젝트**에 가깝다.

`argument_passing`, syscall dispatch, fd table은 직접 학습과 구현 흔적이 남아 있다. 하지만 `fork/exec/wait`, process lifecycle, user memory robustness, cleanup, file system lock은 AI가 방향과 구조를 크게 잡아 준 것으로 보인다.

따라서 "AI 의존적이었나?"라는 질문에는 이렇게 답할 수 있다.

> Thread 단계는 AI가 디버깅과 요구사항 분석을 크게 도운 직접 구현이었다. userprog 단계는 여기에 더해, 시간 압박 속에서 AI가 일부 핵심 구현 구조까지 밀어 준 개발이었다.

학습 목표가 "Pintos userprog를 끝까지 완성하고 테스트를 통과시키는 것"이었다면 AI 사용은 실용적인 선택이었다. 다만 학습 목표가 "process lifecycle과 user memory robustness를 독립적으로 설계하고 디버깅하는 능력"이라면, 이번 단계의 `fork/exec/wait`와 cleanup 영역은 의존도가 높았다고 봐야 한다.
