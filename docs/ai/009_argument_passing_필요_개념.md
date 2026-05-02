# argument passing 필요 개념 정리

이 문서는 `docs/reference/pintos-kaist-original/2_project2/1_argument_passing.md`를 구현하기 전에 알아야 할 개념을 정리한다. 구현 코드는 포함하지 않고, 어떤 상태를 만들어야 하는지와 헷갈리기 쉬운 지점을 설명한다.

## 한 줄 결론

`argument_passing`은 command line을 token으로 파싱한 뒤, user program의 시작 함수인 `_start(argc, argv)`가 정상적으로 인자를 읽을 수 있도록 register와 user stack을 미리 꾸며 주는 작업이다.

일반 함수 호출처럼 실제 `CALL` instruction을 실행하는 것은 아니다. kernel이 user process로 넘어가기 전에 "callee가 막 호출된 것처럼 보이는 상태"를 만들어 놓고 `do_iret()`으로 user mode에 진입한다.

## 지금 이해한 내용 점검

사용자가 정리한 이해는 큰 방향에서 맞다. 다만 몇 가지 표현은 더 정확히 잡는 것이 좋다.

| 이해한 내용 | 판단 | 보정 |
|---|---|---|
| 먼저 파싱 기능을 처리해야 한다 | 맞음 | command line을 공백 기준 token으로 나누고, 첫 token을 program name으로 사용해야 한다. |
| 그 다음 call stack을 구축해야 한다 | 거의 맞음 | 일반적인 kernel call stack이 아니라 user virtual memory 안의 initial user stack을 구축하는 것이다. |
| 제일 상단에는 return address가 있다 | 방향을 주의해야 함 | 최종 `rsp`가 가리키는 가장 낮은 주소 쪽에는 fake return address가 있다. 문자열들은 더 높은 주소 쪽에 놓인다. |
| 각 인자로 이동할 수 있는 pointer가 필요하다 | 맞음 | `argv` 배열은 각 argument string의 user virtual address를 담는 `char *` 배열이다. |
| 다양한 형식의 data가 stack에 같이 있다 | 맞음 | 문자열 bytes, alignment padding, null sentinel, pointer 배열, fake return address가 함께 있다. |
| alignment가 필요하다 | 맞음 | reference 기준으로 pointer 배열을 push하기 전에 stack pointer를 8-byte boundary에 맞춘다. |
| caller가 세팅하고 callee가 작업한 뒤 return address로 돌아간다 | 일반 함수 호출 설명으로는 맞음 | Pintos process 시작에서는 kernel이 caller처럼 초기 상태를 만들어 주지만, `_start()`는 `main()`이 return하면 `exit()`을 호출하므로 정상적으로 return address로 돌아가는 흐름을 기대하지 않는다. fake return address는 stack frame 모양을 맞추기 위한 값이다. |

## 일반 함수 호출과 Pintos process 시작의 차이

일반 x86-64 함수 호출에서는 caller가 인자를 register에 넣고, `CALL` instruction이 return address를 stack에 push한 뒤 callee로 jump한다. callee가 끝나면 return value를 `RAX`에 두고, `RET` instruction으로 return address를 pop해서 caller로 돌아간다.

Pintos에서 user program을 처음 시작할 때는 이 흐름과 비슷한 모양을 kernel이 직접 만들어 준다.

- user program의 entry point는 `_start(argc, argv)`다.
- `_start()`는 user library의 entry 함수이며, 내부적으로 `main(argc, argv)`를 호출하고 `main()`이 return하면 `exit()`을 호출한다.
- kernel은 `_start()`가 받을 `argc`와 `argv`를 x86-64 calling convention에 맞게 준비해야 한다.
- 실제 `CALL _start`를 실행하지 않으므로, kernel이 fake return address를 stack에 넣어 stack frame 모양을 맞춘다.

따라서 여기서 만드는 것은 "진짜 caller의 stack frame"이라기보다 "새 user process가 시작될 때 `_start()`가 기대하는 초기 register/stack 상태"다.

## x86-64 calling convention에서 필요한 부분

Project 2 argument passing에서 필요한 핵심은 다음뿐이다.

| 항목 | 의미 |
|---|---|
| `%rdi` | 첫 번째 integer/pointer 인자. 여기서는 `argc`가 들어간다. |
| `%rsi` | 두 번째 integer/pointer 인자. 여기서는 `argv`, 즉 `argv[0]`의 주소가 들어간다. |
| stack | fake return address와 `argv` 배열, 문자열 data를 담는다. |
| `RAX` | 일반 함수 return value register지만, argument passing 자체의 핵심은 아니다. |

reference에는 일반 함수 인자 순서로 `%rdi`, `%rsi`, `%rdx`, `%rcx`, `%r8`, `%r9`가 나온다. 하지만 `_start()`는 `argc`, `argv` 두 개만 받으므로 여기서는 `%rdi`, `%rsi`만 직접 중요하다.

## 파싱에서 해야 할 일

`process_exec()`는 처음에는 단순 file name처럼 보이는 문자열을 받지만, Project 2에서는 이것을 command line으로 봐야 한다.

예를 들어 다음 command line이 들어왔다고 하자.

```text
/bin/ls -l foo bar
```

파싱 결과는 다음 token 목록이어야 한다.

```text
argv[0] = "/bin/ls"
argv[1] = "-l"
argv[2] = "foo"
argv[3] = "bar"
argc = 4
```

주의할 점은 다음과 같다.

- 첫 token은 executable file name이자 `argv[0]`이다.
- `load()`는 전체 command line이 아니라 실행 파일 이름을 기준으로 executable을 찾아야 한다.
- 여러 개의 연속 공백은 하나의 공백과 같게 처리한다.
- 빈 문자열 token을 인자로 세면 안 된다.
- command line 길이는 합리적인 제한을 둘 수 있다. reference 예시는 한 page, 즉 4 KB 안에 들어가는 수준을 언급한다.

## user stack에 들어가는 것

최종적으로 user stack에는 다음 종류의 값이 들어간다.

| stack 요소 | 역할 |
|---|---|
| argument strings | 실제 문자열 byte들이다. 예: `"/bin/ls\0"`, `"-l\0"`, `"foo\0"`, `"bar\0"` |
| alignment padding | pointer 배열을 8-byte 정렬하기 위해 필요한 0 byte padding이다. |
| null sentinel | `argv[argc] == NULL`을 만족시키기 위한 마지막 null pointer다. |
| `argv[i]` pointers | 각 argument string의 user virtual address를 담는다. |
| fake return address | `_start()`가 일반 함수처럼 시작된 것처럼 stack 모양을 맞추기 위한 0 값이다. |

중요한 점은 `argv` 배열 안의 pointer가 kernel address가 아니라 user stack 안에 복사된 문자열들의 user virtual address를 가리켜야 한다는 것이다.

## 주소 방향과 stack layout 감각

stack은 아래 방향, 즉 낮은 주소 방향으로 자란다. 그러나 문자열 자체는 stack의 높은 주소 쪽에 먼저 복사되고, 그 아래에 pointer 배열과 fake return address가 놓인다.

개념적으로는 다음과 같다.

```text
높은 주소
  argument strings
  alignment padding
  argv[argc] == NULL
  argv[argc - 1]
  ...
  argv[1]
  argv[0]          <- %rsi points here
  fake return addr <- final %rsp points here
낮은 주소
```

따라서 "stack의 제일 상단"이라는 표현은 헷갈릴 수 있다.

- memory 그림에서 높은 주소 쪽 top에는 문자열들이 있다.
- 최종 `rsp`가 가리키는 현재 stack top은 낮은 주소 쪽 fake return address다.

Pintos reference의 예시에서도 최종 stack pointer는 fake return address 위치로 설정된다.

## alignment가 필요한 이유

여기서 말하는 것은 `align` 또는 `alignment`다. `alian`이 아니라 `align`이 맞다.

reference는 pointer 배열을 push하기 전에 stack pointer를 8의 배수 주소로 내리라고 한다. 이유는 다음과 같다.

- `argv` 배열은 `char *` pointer들의 배열이다.
- 64-bit 환경에서 pointer 하나는 8 bytes다.
- pointer가 8-byte boundary에 맞춰 있으면 CPU가 word-aligned access를 할 수 있다.
- reference가 요구하는 layout과 expected output/debugging 기준도 이 정렬을 전제로 한다.

정리하면, 문자열을 복사한 뒤 바로 pointer들을 쌓기 전에 padding을 넣어 `rsp`를 8-byte aligned 상태로 맞춘다.

## 어디까지 구현해야 하는가

argument passing 단계에서 해야 하는 범위는 다음이다.

| 구현 범위 | 설명 |
|---|---|
| command line 복사/보존 | 파싱 과정에서 원본 문자열을 망가뜨려도 되는지, `load()`에 넘길 program name을 어떻게 보존할지 결정해야 한다. |
| tokenization | 공백 기준으로 command line을 token 목록으로 만든다. 연속 공백은 하나처럼 처리한다. |
| program name 추출 | 첫 token을 executable file name으로 사용한다. |
| `load()` 호출 기준 정리 | executable을 찾을 때 전체 command line이 아니라 program name을 사용해야 한다. |
| user stack 문자열 배치 | 각 token 문자열을 user stack에 null terminator 포함해서 복사한다. |
| `argv` pointer 배열 배치 | 문자열 주소들을 `argv[0]`부터 읽을 수 있게 배열을 만든다. |
| `argv[argc] == NULL` | C 표준이 기대하는 null sentinel을 둔다. |
| alignment | pointer 배열을 놓기 전에 stack pointer를 8-byte boundary에 맞춘다. |
| register 설정 | `%rdi = argc`, `%rsi = argv`가 되도록 interrupt frame을 설정한다. |
| fake return address | 최종 stack frame에 fake return address 0을 둔다. |

반대로 이 단계에서 주된 구현 범위가 아닌 것은 다음이다.

| 범위 밖에 가까운 것 | 이유 |
|---|---|
| system call 전체 구현 | argument passing과 별도 단계다. 다만 테스트 출력 확인을 위해 `write(fd=1)`은 빨리 필요할 수 있다. |
| user pointer validation helper | syscall에서 user pointer를 읽을 때 중요하지만, initial argument stack 구성 자체와는 별개다. |
| `fork`, `exec`, `wait` 전체 lifecycle | 나중 단계다. 다만 `process_exec()` 흐름을 건드리므로 이후 `exec`와 연결될 것을 고려해야 한다. |
| file system 내부 구현 | executable loading에 file system을 사용하지만 Project 2의 argument passing은 file system 자체를 구현하는 과제가 아니다. |

## 구현 위치 감각

Project 문서는 argument passing을 `process_exec()`에서 확장하라고 한다. 현재 skeleton의 `process_exec()` 흐름은 대략 다음 순서다.

1. interrupt frame을 준비한다.
2. 기존 process context를 cleanup한다.
3. executable을 `load()`한다.
4. 성공하면 `do_iret()`으로 user mode에 진입한다.

argument passing은 이 흐름에서 `load()`에 어떤 file name을 넘길지, 그리고 `load()` 후 얻은 user stack pointer 위에 어떤 초기 stack layout을 만들지를 다루는 작업이다.

주의할 점은 command line 전체가 필요하다는 것이다. `load()`에는 첫 token만 필요하지만, stack에는 모든 token이 필요하다.

## 테스트 관점

argument passing만 좁게 보면 주요 테스트는 다음이다.

| 테스트 | 보는 것 |
|---|---|
| `args-none` | 인자가 program name 하나뿐인 경우 |
| `args-single` | 인자 하나 추가 |
| `args-multiple` | 여러 인자 |
| `args-many` | 많은 인자 |
| `args-dbl-space` | 연속 공백 처리 |

이 테스트들은 대체로 child program이 `argc`, `argv[i]`, `argv[argc]`를 읽어 expected output을 찍을 수 있는지를 본다.

## 디버깅할 때 볼 것

실패하면 보통 다음을 본다.

- `argc` 값이 token 개수와 맞는가
- `%rdi`에 `argc`가 들어갔는가
- `%rsi`가 `argv[0]` 위치를 가리키는가
- `argv[argc]`가 null인가
- `argv[i]`가 user stack 안의 올바른 문자열 주소를 가리키는가
- 문자열이 null terminator까지 복사되었는가
- 연속 공백을 빈 인자로 세지 않았는가
- 최종 `rsp`가 fake return address를 가리키는가
- pointer 배열을 놓기 전에 8-byte alignment를 맞췄는가

reference는 `hex_dump()`가 argument passing debugging에 유용하다고 언급한다. stack layout이 헷갈릴 때는 최종 `rsp` 근처를 dump해서 문자열, pointer, null sentinel, fake return address가 기대 순서로 있는지 확인하면 된다.

## 가장 중요한 mental model

`argument_passing`은 "문자열을 잘 자르는 문제"와 "초기 user stack을 ABI 비슷한 모양으로 꾸미는 문제"가 합쳐진 작업이다.

kernel은 user program의 caller처럼 행동하지만, 실제로 `CALL`을 하는 것은 아니다. 대신 `_start(argc, argv)`가 이미 호출된 것처럼 register와 stack을 만들어 놓고 user mode로 넘긴다. 이때 `_start()`가 `main(argc, argv)`를 호출하고, `main()`이 return하면 `exit()`으로 process가 종료된다.

따라서 구현의 목표는 단순하다.

```text
user program이 시작되는 순간:
  %rdi == argc
  %rsi == argv
  argv[0..argc-1] == 각 argument string의 user address
  argv[argc] == NULL
  %rsp == fake return address가 놓인 stack 위치
```

이 상태를 맞추면 argument passing의 핵심 요구사항은 충족된다.

## 번외: 일반 함수 호출에서 인자가 6개를 넘으면

x86-64 일반 함수 호출에서는 integer/pointer 인자 기준으로 앞의 6개를 register로 전달한다.

| 인자 순서 | 전달 위치 |
|---:|---|
| 1번째 | `%rdi` |
| 2번째 | `%rsi` |
| 3번째 | `%rdx` |
| 4번째 | `%rcx` |
| 5번째 | `%r8` |
| 6번째 | `%r9` |
| 7번째 이후 | stack |

예를 들어 일반 함수 `f(a, b, c, d, e, f, g, h)`를 호출한다면 앞의 여섯 인자는 register로 전달되고, 일곱 번째 인자부터는 stack에 놓여 callee가 stack을 통해 접근한다.

다만 Pintos argument passing에서는 command line argument가 많아져도 `_start()`에 전달되는 함수 인자는 여전히 `argc`, `argv` 두 개뿐이다. 따라서 command line의 각 token을 `%rdi`, `%rsi`, `%rdx` 등에 하나씩 넣는 것이 아니다.

```text
_start(argc, argv)
```

이 형태만 맞추면 된다.

- `%rdi`에는 전체 인자 개수인 `argc`가 들어간다.
- `%rsi`에는 `argv[0]`이 시작되는 user stack 주소가 들어간다.
- 실제 여러 command line argument 문자열은 `argv` 배열과 user stack의 문자열 영역을 통해 접근한다.

## 참고 자료

- `docs/reference/pintos-kaist-original/2_project2/1_argument_passing.md`
- `pintos/userprog/process.c`
- `pintos/tests/userprog/args.c`
- `pintos/tests/userprog/Make.tests`
