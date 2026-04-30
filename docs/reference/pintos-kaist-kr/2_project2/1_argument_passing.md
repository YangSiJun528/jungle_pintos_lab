### Argument Passing (인자 전달)

**`process_exec()`에서 user program(사용자 프로그램)을 위한 argument(인자)를 설정하세요.**

#### x86-64 Calling Convention (호출 규약)

이 section(절)은 Unix의 64-bit x86-64 implementation(구현)에서 normal function call(일반 함수 호출)에 사용하는 convention(규약)의 중요한 점을 요약합니다. 간결함을 위해 일부 detail(세부 사항)은 생략했습니다. 더 자세한 내용은 [System V AMD64 ABI](https://en.wikipedia.org/wiki/X86_calling_conventions#System_V_AMD64_ABI)를 참조할 수 있습니다.

calling convention은 다음과 같이 동작합니다.

  1. user-level application(사용자 수준 애플리케이션)은 integer register(정수 레지스터)로 `%rdi`, `%rsi`, `%rdx`, `%rcx`, `%r8`, `%r9` 순서를 사용하여 argument를 전달합니다.
  2. caller(호출자)는 자신의 다음 instruction(명령어) address, 즉 return address(반환 주소)를 stack(스택)에 push(넣기)하고 callee(피호출자)의 첫 instruction으로 jump(점프)합니다. x86-64 instruction 하나인 `CALL`이 이 두 일을 모두 수행합니다.
  3. callee가 실행됩니다.
  4. callee에게 return value(반환값)가 있으면 register RAX에 저장합니다.
  5. callee는 stack에서 return address를 pop(꺼내기)하고, 그 address가 지정하는 위치로 jump하여 return합니다. 이때 x86-64 `RET` instruction을 사용합니다.

세 개의 int argument를 받는 function `f()`를 생각해 봅시다. 아래 diagram(그림)은 `f()`가 `f(1, 2, 3)`으로 invoke(호출)되었다고 할 때, 위 step 3의 시작 시점에 callee가 보는 sample stack frame(스택 프레임)과 register state(레지스터 상태)를 보여 줍니다. initial stack address(초기 스택 주소)는 임의입니다.
    
    
                                 +----------------+
    stack pointer --> 0x4747fe70 | return address |
                                 +----------------+
    RDI: 0x0000000000000001 | RSI: 0x0000000000000002 | RDX: 0x0000000000000003
    

#### Program Startup Details (프로그램 시작 세부 사항)

user program용 Pintos C library(라이브러리)는 `lib/user/entry.c`의 `_start()`를 user program의 entry point(진입점)로 지정합니다. 이 function은 `main()`을 감싸는 wrapper(래퍼)이며, `main()`이 return하면 `exit()`을 호출합니다.
    
    
    void
    _start (int argc, char *argv[]) {
        exit (main (argc, argv));
    }
    

kernel(운영체제 핵심부)은 user program이 실행을 시작하도록 허용하기 전에 initial function(초기 함수)의 argument를 register에 넣어야 합니다. argument는 normal calling convention과 같은 방식으로 전달됩니다.

다음 example command(예시 명령)를 위한 argument를 어떻게 처리할지 생각해 봅시다. `/bin/ls -l foo bar`.

  1. command를 word(단어)로 나눕니다: `/bin/ls`, `-l`, `foo`, `bar`.

  2. word를 stack의 top(맨 위)에 둡니다. 이들은 pointer(포인터)를 통해 참조될 것이므로 순서는 중요하지 않습니다.

  3. 각 string(문자열)의 address와 null pointer sentinel(널 포인터 표식)을 stack에 오른쪽에서 왼쪽 순서로 push합니다. 이것들이 argv의 element(원소)입니다. null pointer sentinel은 C standard(표준)가 요구하는 대로 `argv[argc]`가 null pointer가 되도록 보장합니다. 이 순서는 `argv[0]`이 가장 낮은 virtual address(가상 주소)에 오도록 합니다. word-aligned access(워드 정렬 접근)는 unaligned access(비정렬 접근)보다 빠르므로, 최고의 performance(성능)를 위해 첫 push 전에 stack pointer를 8의 배수로 내림하세요.

  4. `%rsi`가 `argv`(`argv[0]`의 address)를 가리키게 하고 `%rdi`를 `argc`로 설정합니다.

  5. 마지막으로 fake "return address"(가짜 반환 주소)를 push합니다. entry function은 절대 return하지 않지만, 그 stack frame은 다른 모든 function과 같은 structure(구조)를 가져야 합니다.

아래 table(표)은 user program 시작 직전의 stack과 관련 register 상태를 보여 줍니다. stack은 아래로 grow(성장)한다는 점에 주의하세요.

Address Name Data Type

0x4747fffc
argv[3][...]
'bar\0'
char[4]

0x4747fff8
argv[2][...]
'foo\0'
char[4]

0x4747fff5
argv[1][...]
'-l\0'
char[3]

0x4747ffed
argv[0][...]
'/bin/ls\0'
char[8]

0x4747ffe8
word-align
0
uint8_t[]

0x4747ffe0
argv[4]
0
char *

0x4747ffd8
argv[3]
0x4747fffc
char *

0x4747ffd0
argv[2]
0x4747fff8
char *

0x4747ffc8
argv[1]
0x4747fff5
char *

0x4747ffc0
argv[0]
0x4747ffed
char *

0x4747ffb8
return address
0
void (*) ()

> RDI: 4 | RSI: 0x4747ffc0

이 예에서 stack pointer는 `0x4747ffb8`로 initialize(초기화)됩니다. 위에 보인 것처럼 여러분의 code는 `include/threads/vaddr.h`에 정의된 `USER_STACK`에서 stack을 시작해야 합니다.

`<stdio.h>`에 선언된 non-standard(비표준) `hex_dump()` function은 argument passing code를 debugging(디버깅)하는 데 유용할 수 있습니다.

#### Implement the argument passing. (argument passing 구현)

현재 `process_exec()`는 새 process(프로세스)에 argument를 전달하는 기능을 지원하지 않습니다. `process_exec()`를 확장하여 이 기능을 구현하세요. 단순히 program file name(프로그램 파일 이름)을 argument로 받는 대신, space(공백)에서 단어로 나누도록 만듭니다. 첫 번째 word는 program name이고, 두 번째 word는 첫 번째 argument이며, 그 뒤도 마찬가지입니다. 즉 `process_exec("grep foo bar")`는 grep을 실행하면서 두 argument foo와 bar를 전달해야 합니다.

command line(명령줄) 안에서 여러 space는 하나의 space와 같으므로 `process_exec("grep foo bar")`는 원래 예제와 equivalent(동등)합니다. command line argument 길이에 reasonable limit(합리적인 제한)을 둘 수 있습니다. 예를 들어 argument가 single page(단일 페이지, 4 kB)에 들어가는 범위로 제한할 수 있습니다. pintos utility(유틸리티)가 kernel에 전달할 수 있는 command-line argument에는 이와 별개로 128 bytes 제한이 있습니다.

argument string(인자 문자열)은 원하는 방식으로 parse(파싱)해도 됩니다. 막혔다면 `include/lib/string.h`에 prototype(원형)이 있고 `lib/string.c`에 자세한 comment(주석)와 함께 구현된 `strtok_r()`를 살펴보세요. prompt(프롬프트)에서 `man strtok_r`을 실행하여 man page(매뉴얼 페이지)에서도 더 알아볼 수 있습니다.
