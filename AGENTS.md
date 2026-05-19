# AI 작업 지침

Pintos 관련 답변과 판단은 가능한 한 아래 로컬 reference 자료를 기준으로 한다.

## 기준 자료

- Pintos 구현과 직접 관련된 답변은 `docs/reference` 아래의 제공된 reference 파일을 기준으로 한다.
- `docs/reference/pintos-kaist-*` 계열 문서는 현재 과제 기준인 CS330 64-bit KAIST Pintos 구현 요구사항의 기준 자료다.
- `docs/reference/kaist-oslab-pintos-slides-*` 계열 YouTube/강의 스크립트는 EE415 계열 32-bit Pintos 설명이 섞여 있으므로, 구현 요구사항 판단 근거로 우선하지 않는다. 이 자료는 개념 설명, 흐름 이해, 강의 메모 보조용으로만 사용한다.
- 외부 자료는 불확실하거나 과제 조건과 다를 수 있으므로, Pintos 구현 요구사항을 판단하는 근거로 사용하지 않는다.
- 외부 자료를 사용할 수 있는 범위는 OS 개념, C 언어, 도구 사용법, 일반 알고리즘 설명으로 한정한다.
- Linux, FreeBSD 같은 실제 OS 커널 소스는 개념 비교용으로만 참고할 수 있다. 해당 코드를 그대로 복사하거나 과제 구현 코드로 옮기지 않는다.

## Reference 우선순위

Pintos 관련 문서가 서로 다르거나 해석이 필요한 경우 아래 순서로 우선한다.

1. `docs/reference/pintos-kaist-original`
2. `docs/reference/pintos-kaist-kr`
3. `docs/reference/kaist-oslab-pintos-slides-*`

현재 과제는 CS330 기준으로 진행한다. EE415 계열 설명인 `kaist-oslab-pintos-slides-*` 자료는 참고용으로만 보고, 내용이 충돌하면 CS330 기준인 `pintos-kaist-*` 문서와 현재 저장소 코드를 우선한다.
