# argument passing helper 사용법

이 문서는 Project 2 `argument_passing`에서 command line 파싱과 user stack 구성을 이해할 때 사용할 수 있는 Pintos 내부 helper들을 정리한다. 구현 코드를 대신 작성하지 않고, 각 helper가 어떤 성격인지와 사용할 때 주의할 점을 설명한다.

## 한 줄 결론

command line 파싱에는 Pintos 내부의 `strtok_r()`를 쓰는 것이 가장 자연스럽다. 문자열 길이 계산과 복사에는 `strlen()`, `strlcpy()`, `memcpy()`, `memset()` 등을 함께 쓴다. stack layout 확인에는 `hex_dump()`가 유용하다.

## 주요 helper 목록

| helper | 위치 | 용도 |
|---|---|---|
| `strtok_r()` | `pintos/lib/string.c`, `pintos/include/lib/string.h` | command line을 token으로 나눔 |
| `strlen()` | `pintos/lib/string.c`, `pintos/include/lib/string.h` | 문자열 길이 계산 |
| `strlcpy()` | `pintos/lib/string.c`, `pintos/include/lib/string.h` | 문자열을 크기 제한 안에서 복사 |
| `memcpy()` | `pintos/lib/string.c`, `pintos/include/lib/string.h` | bytes 단위 복사 |
| `memset()` | `pintos/lib/string.c`, `pintos/include/lib/string.h` | 일정 범위를 같은 byte 값으로 채움 |
| `hex_dump()` | `pintos/lib/stdio.c`, `pintos/include/lib/stdio.h` | memory layout을 hex로 확인 |

## `strtok_r()`의 역할

`strtok_r()`는 문자열을 delimiter 기준으로 여러 token으로 나누는 함수다.

Project 2 argument passing에서는 보통 command line을 공백 `" "` 기준으로 나눌 때 사용한다.

예를 들어 다음 문자열이 있다고 하자.

```text
"echo hello  pintos"
```

공백 기준으로 나누면 token은 다음처럼 잡혀야 한다.

```text
echo
hello
pintos
```

중요한 점은 연속 공백이 있어도 빈 token을 만들지 않는다는 것이다. `strtok_r()`는 여러 delimiter가 붙어 있으면 하나처럼 취급한다. 그래서 `args-dbl-space` 같은 테스트의 요구와 잘 맞는다.

## `strtok_r()` 사용 모양

Pintos의 `strtok_r()`는 다음 형태다.

```c
char *strtok_r (char *s, const char *delimiters, char **save_ptr);
```

각 인자의 의미는 다음과 같다.

| 인자 | 의미 |
|---|---|
| `s` | 처음 tokenizing을 시작할 문자열. 두 번째 호출부터는 `NULL`을 넣는다. |
| `delimiters` | token을 나눌 문자 집합. 공백 기준이면 `" "` |
| `save_ptr` | 다음 탐색 위치를 저장할 `char *` 변수의 주소 |

개념적인 사용 흐름은 다음이다.

```text
첫 호출:
  strtok_r(command_line_copy, " ", &save_ptr)

반복 호출:
  strtok_r(NULL, " ", &save_ptr)

반환값:
  다음 token의 시작 주소
  더 이상 token이 없으면 NULL
```

## `strtok_r()`의 중요한 주의점

### 원본 문자열을 수정한다

`strtok_r()`는 delimiter를 `'\0'`로 바꿔서 token을 만든다. 즉 원본 문자열을 파괴적으로 수정한다.

예를 들어 개념적으로 다음 문자열이:

```text
echo hello pintos\0
```

tokenizing 이후에는 내부적으로 다음처럼 바뀔 수 있다.

```text
echo\0hello\0pintos\0
```

따라서 나중에도 전체 command line이 필요하다면 복사본을 따로 둬야 한다.

### string literal에 쓰면 안 된다

`strtok_r()`는 문자열을 직접 수정하므로 수정 가능한 buffer에만 써야 한다.

안전한 방향:

```text
수정 가능한 page/buffer에 command line을 복사한 뒤 tokenizing
```

피해야 할 방향:

```text
읽기 전용 문자열이나 보존해야 하는 원본을 그대로 tokenizing
```

### 첫 token과 전체 token 목록을 둘 다 생각해야 한다

argument passing에서는 첫 token이 executable file name이다.

하지만 user stack에는 첫 token을 포함한 전체 token 목록이 필요하다.

```text
command line: "grep foo bar"

load에 필요한 이름:
  "grep"

user stack에 필요한 argv:
  argv[0] = "grep"
  argv[1] = "foo"
  argv[2] = "bar"
```

즉 `load()`에 넘길 program name과 stack에 올릴 전체 arguments를 모두 보존할 수 있게 설계해야 한다.

## `strlcpy()` 사용 감각

`strlcpy()`는 source 문자열을 destination buffer에 복사하되, destination 크기를 넘지 않게 제한한다.

Pintos에서는 `strcpy` 대신 `strlcpy` 사용을 유도한다. 실제로 `pintos/include/lib/string.h`에서는 `strcpy`를 쓰면 `dont_use_strcpy_use_strlcpy`로 막히게 되어 있다.

사용 목적은 다음과 같다.

| 상황 | 이유 |
|---|---|
| command line을 page-sized buffer에 복사 | tokenizing이 원본을 수정하므로 복사본 필요 |
| program name을 별도 buffer에 보존 | `load()`에 넘길 이름이 tokenizing 과정에서 바뀌거나 사라지지 않게 하기 위함 |

주의할 점은 destination buffer 크기를 정확히 넘겨야 한다는 것이다.

## `strlen()` 사용 감각

`strlen()`은 null terminator 전까지의 문자열 길이를 반환한다. `'\0'` 자체는 길이에 포함하지 않는다.

argument string을 user stack에 복사할 때는 null terminator까지 포함해야 하므로, 필요한 byte 수를 생각할 때는 다음처럼 이해한다.

```text
복사할 byte 수 = strlen(token) + 1
```

`+ 1`은 마지막 `'\0'` 때문이다.

## `memcpy()` 사용 감각

`memcpy()`는 source에서 destination으로 지정한 byte 수만큼 그대로 복사한다.

argument passing에서 개념적으로 쓰이는 곳은 다음이다.

| 복사 대상 | 설명 |
|---|---|
| argument string bytes | user stack에 token 문자열을 복사 |
| pointer 값 | `argv[i]`에 해당하는 주소 값을 stack에 배치 |
| fake return address 값 | 0 값을 pointer 크기만큼 배치 |

주의할 점은 pointer 값을 복사할 때 "pointer가 가리키는 문자열"과 "pointer 값 자체"를 구분해야 한다는 것이다.

```text
문자열 복사:
  token의 bytes를 stack에 복사

pointer 복사:
  token이 놓인 user address 값을 argv 배열 칸에 복사
```

## `memset()` 사용 감각

`memset()`은 일정 byte 범위를 같은 값으로 채운다.

argument passing에서는 alignment padding을 0으로 채우거나, 초기화된 공간을 명확히 만들 때 생각할 수 있다.

다만 padding byte의 값 자체는 보통 user program이 직접 읽는 값은 아니다. 그래도 0으로 채워 두면 layout 확인과 debugging이 쉬워진다.

## `hex_dump()` 사용 감각

`hex_dump()`는 memory 내용을 hex와 ASCII 형태로 보여준다. reference도 argument passing debugging에 유용하다고 언급한다.

확인하고 싶은 것은 다음이다.

- 문자열들이 user stack에 null terminator 포함해서 들어갔는가
- `argv` pointer 배열이 올바른 주소를 담고 있는가
- `argv[argc] == NULL` sentinel이 있는가
- fake return address가 있는가
- 최종 `rsp` 주변 layout이 reference 그림과 비슷한가

주의할 점은 `hex_dump()` 출력이 많아지면 테스트 expected output을 깨뜨릴 수 있다는 것이다. debugging 중에만 쓰고, 제출 전에는 불필요한 출력이 남지 않게 해야 한다.

## helper 사용 순서 감각

argument passing에서 helper를 쓰는 흐름은 개념적으로 다음과 같다.

```text
1. command line을 수정 가능한 buffer에 복사한다.
2. strtok_r()로 token을 나눈다.
3. 각 token의 길이를 strlen()으로 확인한다.
4. token 문자열들을 user stack에 복사한다.
5. stack pointer를 8-byte boundary에 맞춘다.
6. argv[argc] == NULL sentinel을 둔다.
7. 각 token 문자열의 user address를 argv 배열에 둔다.
8. fake return address를 둔다.
9. intr_frame의 rdi, rsi, rsp를 최종값으로 맞춘다.
10. 필요하면 hex_dump()로 layout을 확인한다.
```

이 순서는 개념 흐름이다. 실제 구현에서는 token을 먼저 저장할지, 문자열을 복사하면서 주소를 저장할지 등 여러 선택지가 있다.

## 자주 하는 실수

| 실수 | 왜 문제인가 |
|---|---|
| 전체 command line을 그대로 `load()`에 넘김 | executable 이름이 `"grep foo"`처럼 되어 file open에 실패할 수 있다. |
| `strtok_r()`가 원본을 수정한다는 점을 잊음 | 나중에 필요한 command line 정보가 사라질 수 있다. |
| `strlen(token)` byte만 복사 | null terminator가 빠져 user program이 문자열 끝을 못 찾는다. |
| kernel address를 `argv[i]`에 넣음 | user program이 접근할 수 없는 주소가 된다. |
| `argv[argc] == NULL`을 빼먹음 | user program이나 테스트가 argv 끝을 잘못 판단한다. |
| pointer 배열 alignment를 무시함 | `args.c`가 `argv` alignment를 검사한다. |
| debugging 출력이 남아 있음 | Pintos 테스트 checker의 expected output과 달라진다. |

## `strtok_r()` 주석 번역

아래는 `pintos/lib/string.c`에 있는 `strtok_r()` 설명 주석의 번역이다.

`strtok_r()`는 `DELIMITERS`로 구분된 문자열을 token들로 나눈다. 이 함수를 처음 호출할 때는 `S`에 token으로 나눌 문자열을 넘겨야 하고, 이후 호출에서는 `S`에 null pointer를 넘겨야 한다.

`SAVE_PTR`은 tokenizer의 현재 위치를 추적하기 위해 사용하는 `char *` 변수의 주소다. 각 호출의 반환값은 문자열 안의 다음 token이며, 더 이상 남은 token이 없으면 null pointer를 반환한다.

이 함수는 인접한 여러 delimiter를 하나의 delimiter처럼 취급한다. 따라서 반환되는 token의 길이는 절대 0이 되지 않는다. `DELIMITERS`는 하나의 문자열을 처리하는 중에도 호출마다 바뀔 수 있다.

`strtok_r()`는 문자열 `S`를 수정한다. delimiter 문자를 null byte로 바꾸기 때문이다. 따라서 `S`는 수정 가능한 문자열이어야 한다. 특히 string literal은 C에서 수정 가능하지 않다. 하위 호환성 때문에 `const`가 붙어 있지 않을 수 있어도 string literal을 수정하면 안 된다.

사용 예시는 다음과 같다.

```c
char s[] = "  String to  tokenize. ";
char *token, *save_ptr;

for (token = strtok_r (s, " ", &save_ptr); token != NULL;
     token = strtok_r (NULL, " ", &save_ptr))
  printf ("'%s'\n", token);
```

출력은 다음과 같다.

```text
'String'
'to'
'tokenize.'
```

## 관련 파일

- `pintos/include/lib/string.h`
- `pintos/lib/string.c`
- `pintos/include/lib/stdio.h`
- `pintos/lib/stdio.c`
- `docs/reference/pintos-kaist-original/2_project2/1_argument_passing.md`
- `pintos/tests/userprog/args.c`
