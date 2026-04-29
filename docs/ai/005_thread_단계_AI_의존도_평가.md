# Pintos Thread 단계 AI 의존도 평가

이 문서는 Pintos Project 1: Threads 단계를 진행하면서 AI에게 던진 프롬프트와 그 결과가 실제 작업에 얼마나 영향을 줬는지 회고한 것이다. 이후 User Programs, Virtual Memory, File System 단계도 같은 형식으로 문서를 추가할 수 있도록 "Thread 단계" 평가로 범위를 명확히 둔다.

전체 채팅 원문이 모두 보존되어 있지는 않으므로, 평가는 `docs/daily`, `docs/ai`에 남은 메모, AI 산출물, git commit 이력, 그리고 `~/.codex/history.jsonl`에 남은 대표 프롬프트 일부를 근거로 한다.

## 결론

전체 의존도는 **중간보다 높은 편, 5점 만점에 3.4점**으로 평가한다.

다만 "AI가 코드를 대신 작성했다"는 의미의 의존도는 그보다 낮다. 메모와 발표자료에는 "가능한 한 전체 코드를 직접 구현하기", "AI 활용은 요구사항 분석과 막혔을 때 방향 확인", "코드 작성, 리팩토링, 최종 판단은 직접 수행"이라는 기준이 반복해서 남아 있다. 실제로 구현 메모도 실패 원인 추적, 테스트 재실행, 리팩토링 판단을 직접 기록하고 있다.

반대로 **문제 정의, 구현 방향 선택, 테스트 분해, 디버깅 힌트, 문서 기반 누락 사항 검토**에서는 AI 의존이 꽤 컸다. 특히 priority donation과 MLFQS는 AI가 원인 후보와 수정 방향을 좁혀 준 비중이 높아서, AI 없이도 완성은 가능했겠지만 완료 속도와 시행착오 비용이 크게 달라졌을 영역이다.

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

## 프롬프트 유형 분석

### 1. 자료 정리와 전사본 보정

가장 명확하게 남아 있는 원문 프롬프트는 강의 자동 자막을 정리하라는 요청이다.

```text
docs/reference/kaist-oslab-pintos-slides-original/scripts-raw 하위에 자동완성 자막 스크립트가 있음.
이를 기반으로 docs/reference/kaist-oslab-pintos-slides-original/scripts 에 시간 정보 없이, 큰 의미 단위로 heading과 문단을 나누고, 요약이나 새 해설은 추가하지 않았고, 자동자막의 오인식 표현만 보정한 문서를 생성하라.
파일 이름은 동일하다.

기존에 커밋된 내역은 제외하고, 아직 커밋되지 않거나 깃으로 관리되지 않는 파일만들 대상으로 한다.

올바른 단어나 표현은 참고 문서인 docs/reference/pintos-kaist-original 내부의 문서를 참고한다.
```

이 프롬프트는 구현 정답을 요구한 것이 아니라, 이미 있는 자료를 읽기 쉬운 형태로 정리하게 한 것이다. 다만 이후 priority donation과 MLFQS에서 이 전사본과 정리 문서가 구현 방향을 잡는 근거가 되었으므로, 간접 의존도는 낮지 않다.

### 2. 개념 학습과 코드 분석 보조

`docs/ai/000_1주차_학습_추천.md`, `docs/ai/001_쓰레드_동작_방식.md`는 Project 1 범위, thread 상태, context switch, synchronization, alarm clock, priority scheduling, donation, MLFQS를 학습 순서로 정리한 문서다.

메모에는 "실제 과제의 정답을 알 수 없고, 배경으로서 학습해야 하는 개념학습/코드분석 보조", "실제 코드를 따라가면서 학습할 때 방향성 잡는데 도움"이라고 적혀 있다. 즉 이 영역의 AI는 구현 자체보다 **어디를 읽어야 하는지와 어떤 흐름으로 이해해야 하는지**를 줄여 주는 역할이었다.

### 3. 테스트 실행 전략

`docs/ai/002_schd_단계별_테스트.md`는 scheduler 관련 테스트를 단계별로 묶었다. `schd-1`, `schd-2`, `schd-3`, `MLFQS`로 기능을 나눠 필요한 `make` target과 `.result` 확인 명령을 모았다.

이 문서는 코드 구현보다 **피드백 루프를 짧게 만든 도구**다. 테스트를 직접 찾고 실행할 수도 있었지만, AI가 테스트 범위를 기능 단위로 나눠 준 덕분에 구현-검증 반복 속도가 빨라졌다. 의존도는 중간이다.

### 4. 설계 대안 검토

Priority scheduling에서는 `ready_list`와 waiters를 정렬 상태로 유지할지, 매번 max를 찾을지, condition variable의 `semaphore_elem`에 thread 또는 priority 정보를 넣을지 같은 선택이 있었다.

메모에는 "AI랑 이야기 더 충분한 문맥을 주고 하니까 이걸 추천해줌", "AI한테 여러가지 대화하면서 더 찾아보기", "물어보니까 1번이 좋다고 함"이 남아 있다. 여기서 AI는 단순 설명이 아니라 설계 대안 선택에 영향을 줬다. 다만 최종 구현과 커밋은 직접 판단과 테스트를 거쳐 이루어졌으므로 의존도는 높음 직전의 중간 수준이다.

### 5. 디버깅 힌트와 문서 기반 리뷰

Priority donation과 MLFQS에서 의존도가 가장 높았다.

Priority donation에서는 몇 시간 막힌 뒤 "AI 힌트"를 받았고, 메모에는 "거의 짚어주다시피" 했다고 되어 있다. 이 지점은 핵심 버그 원인 후보를 AI가 상당히 좁혀 준 것으로 보인다.

MLFQS에서는 테스트 실패 분석, `thread_current()` 사용 가능성, `mlfqs-recent-1` 테스트의 의미, 문서 기반 미충족 사항 리뷰까지 AI가 도왔다. 특히 `thread_set_nice()`, parent 상속, 모든 thread 대상 갱신, 반올림, 갱신 순서 같은 항목은 AI 피드백이 상세하게 남아 있어 구현 품질 검토에 꽤 직접적인 영향을 줬다.

### 6. 구현과 리팩토링

구현 자체는 직접 수행한 흔적이 강하다. 메모에는 테스트 실패를 보고 `printf`로 추적하고, 공식 문서/FAQ를 다시 확인하고, 코드를 갈아엎고, 이름을 통일하고, 최종 테스트를 돌린 과정이 남아 있다.

다만 "롤백은 AI 시키면 잘 해준다", "AI한테 동작 문제있는거 없나 검증" 같은 기록도 있으므로, 구현 주변 작업과 사후 검증에는 AI가 보조자로 들어왔다. 코드 작성 주체는 직접이지만, 완성도 확인에는 의존이 있었다.

## 대표 JSONL 프롬프트 근거

아래는 `~/.codex/history.jsonl`에서 Thread 단계와 직접 관련 있는 대표 프롬프트 일부를 그대로 옮긴 것이다. 전체 세션을 모두 싣지는 않고, 의존도 판단에 영향을 준 유형별 사례만 골랐다.

```jsonl
{"session_id":"019dc02e-36f3-7ba3-82c7-d0587e81a199","ts":1777093542,"text":"docs/reference/kaist-oslab-pintos-slides/scripts-raw 하위에 자동완성 자막 스크립트가 있음. 이를 기반으로 docs/reference/kaist-oslab-pintos-slides/scripts 에 시간 정보 없이 큰 의미로 나뉘어진, 원본에서 잘못 인식된 표현만 보정한 전사본을 생성하라.\n\n기존에 커밋된 내역은 제외하고, 아직 커밋되지 않거나 깃으로 관리되지 않는 파일만들 대상으로 한다.\n\n올바른 단어나 표현은 참고 문서인 docs/reference/pintos-kaist 내부의 문서를 참고한다."}
{"session_id":"019dc3bb-dcbf-7392-8930-8124a4f424d3","ts":1777125263,"text":"실제 구현이나 그런거 말고, 코드나 그런거 확인하지 말고,\n\ndocs/reference 에 정리된 자료 기준으로 다음 추측이 맞는지 평가.\n\n\n- 사고 흐름 메모: Priority donation 구상\n    - 뭔가 의존하고 있는 lock이나 그런 관계를 표현할 수 있어야 할거 같은데, 그냥 다 구현해야하나? 너무 빡센데\n    - lock 객체 찾아보니까 `synch.h`에서 세마포나 컨디션이나 waiters를 가지고 있음.\n        - `synch.c`의 구현을 보면 가장 앞에 있는 스레드를 뽑아서 실행시킴 (`list_pop_front` 사용)\n        - 근데 보면... `sema_down()`같이 대기해야 할 때 그냥 넣고 대기타는데?\n        - 과제에서는 여기 수정해야 한다는 말이 없어서... 뭔가 잘못 생각하는거 같은데\n        - 일단 간단한건 `thread_get_priority()`에서 waiters를 매번 합해서 반환해주는거임.\n            - 여기서 또 문제는... nested donation를 처리해줘야 한다는데, 이걸 어떻게 탐색할 수 있는가...\n            - 일단 하위 사고로 빼기\n        - `thread_set_priority()`는 그냥 변경하고 `ready_list` 순회해서 보고 나보다 큰거 있으면 `yield`에 넣기\n            - sleep_list야 내가 다 관리하니까 정렬 순서 보장이 가능한데, `ready_list`는 기존 구현을 변경해야 해서 성능 좀 낮지만 쉽게 구현\n            - 최적화하려면 max를 별도 변수에 넣어둘 수 있을거 같은데, 아마 4QMS 뭐시긴가 그거 할 때 갈아엎어질거 같아서 굳이?"}
{"session_id":"019dcdc8-c820-7b22-a738-4b25fb8269c0","ts":1777277758,"text":"지금 schd-n 1,2 계획은 괜찮음? 3번은 나중에 할거라서 지금 1,2 부분에서 누락된거 없는지만 확인"}
{"session_id":"019dcf2d-1b48-7761-891a-b8531f44cd32","ts":1777313608,"text":"이거 테스트 로직이 어떻게 됨. 테스트 자체만 말하는거야. 어떤 결과를 기대하는지만 설명해줘.\n\npriority-donate-nest, priority-donate-sema, priority-donate-chain"}
{"session_id":"019dd71f-a2f7-72a2-b675-83590d2a7ed7","ts":1777442536,"text":"이거 왜 나는거임? 이유만 설명\n\n\nhreads/init.o:/workspace/pintos/threads/build/../../include/threads/fixed-point.h:31: first defined here\nld: tests/threads/mlfqs/mlfqs-block.o: in function `fp_add':\n/workspace/pintos/threads/build/../../include/threads/fixed-point.h:40: multiple definition of `fp_add'; threads/init.o:/workspace/pintos/threads/build/../../include/threads/fixed-point.h:40: first defined here\nld: tests/threads/mlfqs/mlfqs-block.o: in function `fp_sub':\n/workspace/pintos/threads/build/../../include/threads/fixed-point.h:45: multiple definition of `fp_sub'; threads/init.o:/workspace/pintos/threads/build/../../include/threads/fixed-point.h:45: first defined here\nld: tests/threads/mlfqs/mlfqs-block.o: in function `fp_add_i':\n/workspace/pintos/threads/build/../../include/threads/fixed-point.h:50: multiple definition of `fp_add_i'; threads/init.o:/workspace/pintos/threads/build/../../include/threads/fixed-point.h:50: first defined here\nld: tests/threads/mlfqs/mlfqs-block.o: in function `fp_sub_i':\n/workspace/pintos/threads/build/../../include/threads/fixed-point.h:55: multiple definition of `fp_sub_i'; threads/init.o:/workspace/pintos/threads/build/../../include/threads/fixed-point.h:55: first defined here\nld: tests/threads/mlfqs/mlfqs-block.o: in function `fp_mul':\n/workspace/pintos/threads/build/../../include/threads/fixed-point.h:60: multiple definition of `fp_mul'; threads/init.o:/workspace/pintos/threads/build/../../include/threads/fixed-point.h:60: first defined here\nld: tests/threads/mlfqs/mlfqs-block.o: in function `fp_mul_i':\n/workspace/pintos/threads/build/../../include/threads/fixed-point.h:65: multiple definition of `fp_mul_i'; threads/init.o:/workspace/pintos/threads/build/../../include/threads/fixed-point.h:65: first defined here\nld: tests/threads/mlfqs/mlfqs-block.o: in function `fp_div':\n/workspace/pintos/threads/build/../../include/threads/fixed-point.h:"}
{"session_id":"019dd8e8-ec47-7302-ae2c-958e939248c6","ts":1777460688,"text":"tests/threads/mlfqs/mlfqs-recent-1 테스트 분석, 어떤 것을 어떤 식으로 테스트하나요?"}
{"session_id":"019dda02-11fc-77b2-81ad-a5b8bdb3a0e1","ts":1777479121,"text":"docs/ai/005_적절한이름 \n메모에 이 핀토스 프러젝트 하면서 너한테 쓴 프롬프트 분석해서 얼마나 의존적이였는지 각 영역과 커밋으로 한 작업 보고 평가하고 문서로 작성해줘."}
```

이 원문들은 AI 사용이 단순 질의응답을 넘어선 지점을 보여 준다. 전사본 정리, 문서 기준 설계 검증, 단계별 테스트 계획 확인, 테스트 의미 분석, fixed-point 링크 오류 원인 설명 요청, MLFQS 테스트 분석 요청이 모두 포함되어 있다.

## 영역별 의존도

| 영역 | 의존도 | 근거 | 평가 |
| --- | ---: | --- | --- |
| 초기 세팅, 문서 변환 | 3 | 4/24 메모에 Codex로 초기 세팅, CLion/devcontainer/디버거 연동, 자료 md 저장 기록 | 개발 환경과 자료 기반을 빠르게 만든 데 AI가 실질적으로 기여했다. |
| Project 1 학습 경로 | 2.5 | `000_1주차_학습_추천.md`, `001_쓰레드_동작_방식.md` | 개념 이해의 출발점을 잡는 데 도움. 구현 의존은 아님. |
| Alarm Clock | 1.5 | 4/26 메모에 pair programming, `list.h/.c` 사용 패턴만 AI 도움 기록 | 구현은 직접/페어 중심. AI는 API 사용법 확인 수준. |
| Priority Scheduling 기본 | 3 | 정렬 유지 vs max 방식, ready/waiters 설계 선택을 AI와 논의 | 구현 방향 선택에 영향. 다만 TODO와 테스트 기반 구현은 직접 수행. |
| Semaphore/Condition waiters | 3.5 | `semaphore_elem`에 식별값을 넣는 선택을 AI와 검토 | condition variable 구조 이해와 설계 개선에 AI 기여가 큼. |
| Priority Donation | 4 | nested/multiple 개념, 실패 테스트, 정렬 깨짐 문제에서 AI 힌트 기록 | 가장 의존도가 높은 영역 중 하나. 핵심 원인 파악에 AI가 크게 기여. |
| MLFQS/fixed-point | 4 | fixed-point 배치, static inline, 테스트 의미 분석, 문서 미충족 리뷰 | 구현은 직접이지만 요구사항 누락 검출과 버그 원인 축소에 AI 영향이 큼. |
| 테스트 전략 | 3 | `002_schd_단계별_테스트.md`, 단계별 make target | 정답을 만든 것은 아니지만 구현 루프를 크게 줄였다. |
| 발표/문서화 | 3 | ASSERT 발표자료, Advanced Scheduler 구현 방향 문서 | 설명 구조와 문서 산출물에 AI 활용 비중이 높다. |
| 최종 판단/리팩토링 | 2 | 발표자료에 직접 수행 기준 명시, 4/29 리팩토링 메모 | AI 리뷰는 받았지만 최종 판단과 정리는 직접 수행. |

## 커밋별 평가

| 커밋 | 작업 | AI 의존도 | 판단 |
| --- | --- | ---: | --- |
| `7931d7c` docs: add Pintos study references | Project 1 참고문서, AI 학습 문서 000/001, daily 초기 기록 추가 | 3 | 자료 수집/정리와 학습 경로 설정에 AI가 강하게 개입했다. 구현 커밋은 아니다. |
| `595ec97` docs: document Docker development workflow | Docker 개발 워크플로 문서 | 2 | 환경 문서화는 AI 보조 가능성이 있으나 핵심 과제 구현 의존은 낮다. |
| `8b820d6` feat(thread): Alarm Clock 기능 구현 | `timer_sleep()`, sleep list, wakeup tick 기반 구현 | 1.5 | 메모상 pair programming과 직접 구현 중심. AI는 list 사용법 확인 정도. |
| `bcc48cf` docs: 테스트용 문서 작성 | 테스트 실행 메모 작성 | 2.5 | 테스트 절차 정리에는 AI 도움 가능성이 있으나 구현 로직과는 분리된다. |
| `afbe284` feat(thread): Alarm Clock 최적화 | Alarm Clock 최적화 | 1.5 | 기존 구현 개선. AI 의존 흔적은 낮다. |
| `a50966f` feat(thread): Alarm Clock 우선순위 기반 처리 기능 추가 | alarm wakeup 이후 priority 기반 처리 | 2 | priority scheduling으로 넘어가는 경계 작업. AI보다 구현/테스트 의존이 컸다. |
| `89c532c` chore: 이름 변경 wakeup_tick -> wakeup_ticks | naming 변경 | 0.5 | 단순 리네이밍. |
| `0a9c4d1` todo: 우선순위 스케줄링 초반 계획 세우기 | priority scheduling TODO와 초기 훅 추가 | 3 | 설계 방향을 잡는 단계라 AI 대화 영향이 있었을 가능성이 높다. |
| `863e189` feat(thread): Priority Scheduling 일부 구현 | ready/waiters 우선순위 처리 일부 구현 | 3 | 정렬 유지, preemption 지점 검토 등 AI가 설계 판단에 영향을 줬다. |
| `aa2b700` docs: Priority Scheduling 테스트 편하게 하기 위한 문서 | scheduler 단계별 테스트 문서 `002` 추가 | 3 | 테스트 분해와 명령 모음은 AI 도움을 명시적으로 받은 영역이다. |
| `a196458` feat(thread): 세마포어 우선순위 스케줄링 최적화 | semaphore waiters 우선순위 최적화 | 3 | waiters 정렬과 깨움 순서 설계에 AI/문서 검토 영향이 있다. |
| `4b4f6f0` chore: 기존 변경 의도치 않게 된 부분 원문으로 복구 | 원문 복구 | 1 | 구현 의존보다 git 정리 성격. |
| `71b9a89` feat(thread): 우선순위 스케줄링 기능 구현 완료 | priority scheduling, donation 기본 구조 완성 | 4 | donation 구조와 condition waiter 설계가 포함된 핵심 커밋. AI 힌트와 설계 검토 의존이 높다. |
| `5a76151` chore: 주석 & 스타일 다듬기 | synch.c 주석/스타일 | 1 | 직접 정리 성격. |
| `4c93615` fix(thread): 우선순위 낮출 때 선점 안되는 문제 | effective priority가 낮아질 때 yield/갱신 문제 수정 | 4 | 4/28 메모에 AI 힌트로 테스트코드에서 원인을 찾았다고 남아 있다. |
| `f77e5bb` chore: 주석 & 스타일 다듬기 | 주석/스타일 정리 | 1 | 낮음. |
| `223ad43` fix(thread): donors 비교 연산 잘 되게 수정 | donor 비교/정렬 수정 | 3.5 | donation 관련 후속 버그 수정. AI 피드백으로 개념 재정리한 시기와 겹친다. |
| `24522b3` refactor(thread): priority dontain 중복 코드 extract | donation 중복 로직 추출 | 2.5 | 리팩토링 자체는 직접 수행 기준이 있으나 AI 검증 가능성이 있다. |
| `c777f22` feat(thread): list cmp 함수 네이밍 규칙 적용 | compare 함수 네이밍 통일 | 1 | 스타일/명명 정리. |
| `51a17ad` refactor(thread): 불필요한 코드 제거 | 불필요 코드 제거 | 1 | 낮음. |
| `5086dff` docs: 9주차 발표자료 준비 | ASSERT 발표자료와 정리 문서 추가 | 3 | 발표 구조와 설명 문서 산출물에 AI 활용 가능성이 높다. |
| `deff15d` refactor(thread): 추후 작업을 위한 함수 extract | synch.c 함수 추출 | 2 | 구조 정리. 직접 리팩토링 중심으로 보인다. |
| `043a762` docs: 9주차 발표자료 보완 | ASSERT 발표 보완, PDF 생성 | 2.5 | 문서/발표 보완. 구현 의존은 아님. |
| `86f7c62` docs: Advanced Scheduler 요구사항 분석 문서 추가 | `004_advanced_scheduler_구현_방향.md` 추가 | 3.5 | MLFQS 구현 전 요구사항 정리에 AI가 크게 기여한 산출물. |
| `80cb8c9` feat(thread): 고정소수점 헬퍼 함수 모음 | `fixed-point.h` 추가 | 3 | fixed-point를 `.h`에 둘지, `static inline` 패턴을 어떻게 쓸지 AI와 논의한 기록이 있다. |
| `0ed953d` feat(thread): mlfqs 기능 구현 | MLFQS 전체 구현 | 4 | 테스트 실패 분석, 모든 thread 갱신 누락, 갱신 순서, 반올림 등 AI 리뷰가 직접적인 품질 개선에 작용했다. |
| `6fe34df` docs(user-program): KAIST OS Lab user-program 전사본 추가 | Project 2 강의 전사본 추가 | 2.5 | Project 1 구현 이후 자료 전사/정리 성격. 구현 의존 평가는 별도 대상이 아니다. |

`docs(daily)` 커밋들은 작업 로그 자체이므로 의존도 평가 대상이라기보다 근거 자료다. 다만 4/27, 4/28, 4/29 daily 문서에는 AI 사용 방식이 구체적으로 남아 있어 위 커밋 평가의 핵심 근거가 된다.

## 의존도가 높았던 지점

### Priority Donation

처음에는 `donations`, `wait_on_lock`, `d_elem`, `base_priority` 같은 구조를 직접 정리했고, multiple/nested를 나눠 의사코드도 작성했다. 이 점은 이해와 설계 시도가 있었다는 근거다.

하지만 테스트가 막힌 뒤 AI가 "priority donation 전에 순서가 바뀔 수 있다"는 문제를 짚어 줬고, 이후 sema, condvar, ready list를 다시 봐야 한다는 방향이 나왔다. 메모의 표현상 이 힌트는 단순한 일반론이 아니라 문제 해결의 핵심에 가까웠다. 그래서 이 영역은 **높은 의존도**로 평가한다.

### MLFQS

MLFQS는 공식과 갱신 주기가 명확해서 처음에는 "자료 그대로 구현"에 가까웠다. 그러나 실제로는 fixed-point 위치, ready_threads 정의, idle thread 처리, current thread 포함 여부, 모든 thread 갱신, 반올림, 갱신 순서가 계속 문제였다.

특히 4/29 메모에는 AI가 문서 기준 미충족 사항을 거의 코드 리뷰처럼 나열한 기록이 있다. 이 피드백은 `thread_set_nice()`, parent 상속, blocked thread 갱신, priority 재계산 대상, `load_avg`/`recent_cpu` 반올림, timer tick 갱신 순서까지 포함한다. 구현 품질을 public test 통과 수준에서 reference 요구사항 충족 수준으로 끌어올리는 데 AI 의존이 컸다.

## 의존도가 낮았던 지점

Alarm Clock은 상대적으로 낮다. 4/26 메모에는 pair programming으로 개발했고, `list.h/.c` 사용법을 AI에게 물어본 정도라고 기록되어 있다. 실제 구현은 `timer_sleep()`의 busy waiting 제거, sleep list, wakeup tick 관리처럼 과제 요구를 직접 코드로 옮긴 성격이 강하다.

단순 리네이밍, 주석 정리, 함수 추출, 불필요 코드 제거 같은 커밋도 AI 의존도가 낮다. 이런 작업은 코드 소유권을 유지한 상태에서 직접 판단한 정리 작업으로 보는 것이 맞다.

## 학습 관점 평가

좋았던 점은 다음과 같다.

- AI 사용 목적을 "정답 생성"이 아니라 "요구사항 분석, 방향 확인, 막힌 지점 힌트"로 제한하려고 했다.
- AI 답변을 바로 커밋하지 않고, 테스트 실패와 공식 문서를 다시 대조했다.
- daily 메모에 AI 사용 지점과 찝찝했던 부분을 남겨서 나중에 의존도를 추적할 수 있게 했다.
- 문서 기반 리뷰를 요청해 reference 요구사항과 구현의 차이를 확인했다.

위험했던 점은 다음과 같다.

- priority donation처럼 어려운 버그에서 AI가 핵심 원인을 짚어 준 뒤 해결된 구간이 있다.
- MLFQS에서는 AI 피드백이 요구사항 누락 목록을 상당히 상세히 제공했다.
- "AI 없이 찾았는가"보다 "AI가 알려준 뒤 이해했는가"에 가까운 지점이 생겼다.
- 실제 프롬프트/응답 원문이 대부분 남아 있지 않아, 나중에 어디까지 도움을 받았는지 정확히 복기하기 어렵다.

## 다음 프로젝트에서의 사용 기준

다음부터는 AI 사용을 세 단계로 나누는 것이 좋다.

| 단계 | 허용할 사용 | 기록할 것 |
| --- | --- | --- |
| Green | 공식 문서 요약, API 사용법, 테스트 명령 정리 | 프롬프트 요약만 기록 |
| Yellow | 설계 대안 비교, 실패 원인 후보 요청 | 내가 먼저 세운 가설과 AI가 바꾼 판단 기록 |
| Red | 특정 버그 원인 지목, 코드 리뷰식 누락 목록, 패치 생성 | 원문 프롬프트와 응답 핵심을 반드시 기록 |

특히 Red 단계는 사용하지 말자는 뜻이 아니라, **학습 의존도가 높아지는 순간**이므로 기록을 남기자는 뜻이다. AI 힌트를 받은 뒤에는 바로 수정하지 말고, 다음 세 가지를 먼저 적으면 좋다.

1. 내가 AI 없이 세웠던 가설
2. AI가 새로 제시한 관점
3. 왜 그 관점이 맞는지 내가 코드와 테스트로 확인한 근거

이렇게 남기면 AI 도움을 받아도 최종 이해가 내 것으로 남는다.

## 최종 평가

이번 Pintos Thread 단계는 **AI 보조를 많이 받은 직접 구현 프로젝트**에 가깝다. 코드 작성 자체를 통째로 위임한 프로젝트는 아니지만, 어려운 scheduler 버그와 MLFQS 요구사항 검증에서는 AI가 문제 해결의 방향을 강하게 잡아 줬다.

따라서 "AI 의존적이었나?"라는 질문에는 이렇게 답할 수 있다.

> 구현의 손은 직접 움직였지만, 방향 감각과 디버깅 가속에는 AI 의존이 분명히 있었다. 특히 priority donation과 MLFQS는 AI 없이 같은 시간 안에 같은 완성도까지 도달하기 어려웠을 가능성이 높다.

학습 목표가 "코드를 직접 구현해 보는 것"이었다면 기준은 어느 정도 지켰다. 다만 다음 단계의 목표가 "OS scheduler 문제를 독립적으로 진단하는 능력"이라면, 앞으로는 AI가 핵심 원인을 짚어 주기 전에 스스로 가설과 반례를 더 남기는 방식으로 사용량을 줄이는 것이 좋다.
