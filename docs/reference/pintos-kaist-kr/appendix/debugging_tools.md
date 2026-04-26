# Debugging Tools (디버깅 도구)

Pintos를 debugging(디버깅)하는 데 사용할 수 있는 도구가 많이 있습니다. 이 appendix(부록)는 그중 몇 가지를 소개합니다.

## `printf()`

`printf()`의 가치를 과소평가하지 마세요. Pintos에서 `printf()`가 구현된 방식 덕분에 kernel thread(커널 스레드)든 interrupt handler(인터럽트 처리기)든, 어떤 lock(잠금)이 잡혀 있는지와 거의 관계없이 kernel(운영체제 핵심부)의 사실상 어디에서나 호출할 수 있습니다. `printf()`는 data(데이터)를 검사하는 데만 유용한 것이 아닙니다. kernel이 유용한 error message(오류 메시지) 없이 crash(충돌)하거나 panic(패닉)하더라도 언제 어디서 문제가 생기는지 알아내는 데 도움이 됩니다. 전략은 실패한다고 의심되는 code(코드) 조각 곳곳에 서로 다른 string(문자열), 예를 들어 "<1>", "<2>", ... 를 출력하는 `printf()` 호출을 뿌리는 것입니다. <1>조차 출력되지 않으면 그 지점 이전에 나쁜 일이 일어난 것이고, <1>은 보이지만 <2>는 보이지 않으면 두 지점 사이에 나쁜 일이 일어난 것입니다. 이렇게 배운 것을 바탕으로 의심되는 더 작아진 code region(영역)에 더 많은 `printf()` 호출을 넣을 수 있습니다. 결국 문제를 하나의 statement(문장)로 좁힐 수 있습니다. 관련 technique(기법)은 [Triple Faults](https://casys-kaist.github.io/pintos-kaist/appendix/appendix/debugging_tools.md#tripple-faults)를 참조하세요.

## ASSERT

Assertion(단언)은 문제가 눈에 띄기 전에 일찍 잡아낼 수 있어 유용합니다. 이상적으로 각 function(함수)은 argument(인자)의 validity(유효성)를 check(확인)하는 assertion 집합으로 시작해야 합니다. function local variable(지역 변수)의 initializer(초기화 식)는 assertion이 check되기 전에 evaluate(평가)되므로, initializer에서 argument가 valid하다고 가정하지 않도록 주의하세요. 문제가 생길 가능성이 있다고 의심되는 function body(본문) 곳곳에 assertion을 넣을 수도 있습니다. loop invariant(반복문 불변식)를 check하는 데 특히 유용합니다.

Pintos는 assertion을 check하기 위해 `<debug.h>`에 정의된 `ASSERT` macro(매크로)를 제공합니다.
    
    
    #define ASSERT(expression) { /* Omit details */ }
    

expression(식)의 값을 test(검사)합니다. 0(false)으로 evaluate되면 kernel panic이 발생합니다. panic message는 실패한 expression, file(파일)과 line number(줄 번호), 그리고 backtrace(호출 스택 추적)를 포함하므로 문제를 찾는 데 도움이 됩니다. 자세한 내용은 [Backtraces](#Backtraces)를 참조하세요.

## Function and Parameter Attributes (함수와 매개변수 속성)

`<debug.h>`에 정의된 이 macro들은 compiler(컴파일러)에게 function 또는 function parameter(매개변수)의 special attribute(특수 속성)를 알려 줍니다. expansion(확장)은 GCC-specific(GCC 전용)입니다.

* * *
    
    
    #define UNUSED { /* Omit details */ }
    

> function parameter 뒤에 붙여, 그 parameter가 function 안에서 사용되지 않을 수도 있음을 compiler에게 알려 줍니다. 원래라면 나타날 warning(경고)을 suppress(억제)합니다.

* * *
    
    
    #define NO_RETURN { /* Omit details */ }
    

> function prototype(원형)에 붙여, function이 절대 return(반환)하지 않는다고 compiler에게 알려 줍니다. compiler가 warning과 code generation(코드 생성)을 더 정교하게 조정할 수 있게 합니다.

* * *
    
    
    #define NO_INLINE { /* Omit details */ }
    

> function prototype에 붙여, compiler가 function을 in-line(인라인)으로 emit(내보내기)하지 않도록 합니다. 때때로 backtrace 품질을 높이는 데 유용합니다(아래 참조).

* * *
    
    
    #define PRINTF_FORMAT(format, first) { /* Omit details */ }
    

> function prototype에 붙여, 해당 function이 argument number format(1부터 시작) 위치에 `printf()` 같은 format string(형식 문자열)을 받고, 대응되는 value argument(값 인자)가 argument number first에서 시작한다고 compiler에게 알려 줍니다. 이를 통해 잘못된 argument type(타입)을 전달하면 compiler가 알려 줄 수 있습니다.

## Backtraces

kernel이 panic하면 "backtrace"를 print합니다. 이는 panic 당시 실행 중이던 function 안의 address(주소) 목록으로, program이 현재 위치에 어떻게 도달했는지 요약한 것입니다. code의 아무 지점에서나 backtrace를 print하기 위해 `<debug.h>`에 prototype된 `debug_backtrace()` 호출을 넣을 수도 있습니다. 역시 `<debug.h>`에 선언된 `debug_backtrace_all()`은 모든 thread(스레드)의 backtrace를 print합니다.

backtrace의 address는 raw hexadecimal number(원시 16진수 숫자)로 나열되어 해석하기 어렵습니다. 이를 function name(함수 이름)과 source file line number(소스 파일 줄 번호)로 translate(변환)하는 `backtrace` tool(도구)을 제공합니다. 첫 argument로 `kernel.o`의 이름을 주고, 나머지 argument로 backtrace를 구성하는 hexadecimal number(`0x` prefix 포함)를 주면 됩니다. 그러면 각 address에 대응하는 function name과 source file line number를 output(출력)합니다.

backtrace의 translated form(변환된 형태)이 garbled(깨져 있음)되어 있거나 말이 되지 않는다면, 예를 들어 function A가 function B 위에 표시되지만 B가 A를 호출하지 않는다면, kernel thread의 stack(스택)을 corrupt(손상)시키고 있다는 좋은 sign(징후)입니다. backtrace는 stack에서 추출되기 때문입니다. 또는 backtrace에 전달한 `kernel.o`가 backtrace를 만든 kernel과 다른 것일 수도 있습니다.

때때로 backtrace는 corruption(손상) 없이도 혼란스러울 수 있습니다. compiler optimization(컴파일러 최적화)이 놀라운 behavior(동작)를 만들 수 있기 때문입니다. 어떤 function이 마지막 action(동작)으로 다른 function을 호출했다면(tail call, 꼬리 호출), 호출한 function이 backtrace에 전혀 나타나지 않을 수 있습니다. 마찬가지로 function A가 절대 return하지 않는 function B를 호출하면, compiler가 unrelated function C를 A 대신 backtrace에 나타나게 optimize할 수 있습니다. function C는 단순히 memory에서 우연히 A 바로 뒤에 있는 function입니다. threads project에서 test failure(테스트 실패)의 backtrace에 이런 일이 흔히 보입니다.

### Example (예시)

다음 예를 보세요. Pintos가 file system project의 실제 제출물에서 가져온 다음 call stack(호출 스택)을 print했다고 합시다.
    
    
    Call stack: 0x800426eff 0x8004202fb 0x80042dc22 0x80042cf67 0x800422319
    0x80042325a 0x40012c 0x400a96 0x400ac8.
    

그러면 아래처럼 `backtrace` utility(유틸리티)를 invoke하고, backtrace information(정보)을 command line(명령줄)에 cut and paste(잘라 붙이기)합니다. 여기서는 `kernel.o`가 current directory(현재 디렉터리)에 있다고 가정합니다. 아래 내용은 여기 margin(여백)을 넘치지만, 실제로는 하나의 shell command line(셸 명령줄)에 모두 입력해야 합니다.
    
    
    backtrace 0x800426eff 0x8004202fb 0x80042dc22 0x80042cf67 0x800422319
    0x80042325a 0x400012c 0x400a96 0x4000ac8
    

backtrace output은 대략 다음과 같습니다.
    
    
    0x0000000800426eff: debug_panic (lib/debug.c:86)
    0x00000008004202fb: file_seek (filesys/file.c:405)
    0x000000080042dc22: seek (userprog/syscall.c:744)
    0x000000080042cf67: syscall_handler (userprog/syscall.c:444)
    0x0000000800422319: intr_handler (threads/interrupt.c:334)
    0x000000080042325a: intr_entry (threads/intr-stubs.S:38)
    0x000000000040012c: (unknown)
    0x0000000000400a96: (unknown)
    0x0000000000400ac8: (unknown)
    

자신의 kernel binary(커널 바이너리)에서 위 command를 실행해도 정확히 같은 address를 보지는 않을 것입니다. compile한 source code와 compiler가 다를 가능성이 높기 때문입니다. backtrace의 첫 line은 kernel panic을 구현하는 function인 `debug_panic()`을 가리킵니다. backtrace는 흔히 kernel panic에서 나오므로 `debug_panic()`은 backtrace에 표시되는 첫 function인 경우가 많습니다. 두 번째 line은 이 경우 assertion failure(단언 실패)의 결과로 panic한 function이 `file_seek()`임을 보여 줍니다. 이 예에 사용된 source code tree에서 `filesys/file.c`의 line 405는 다음 assertion입니다.
    
    
    ASSERT (file_ofs >= 0);
    

이 line은 assertion failure message에도 인용되었습니다. 따라서 `file_seek()`은 negative file offset argument(음수 파일 오프셋 인자)를 전달받았기 때문에 panic했습니다. 세 번째 line은 `seek()`가 offset argument를 validate(검증)하지 않고 `file_seek()`를 호출했을 것으로 보인다는 것을 나타냅니다. 이 제출물에서 `seek()`는 seek system call(시스템 콜)을 구현합니다. 네 번째 line은 system call handler인 `syscall_handler()`가 `seek()`를 invoke(호출)했음을 보여 줍니다. 다섯 번째와 여섯 번째 line은 interrupt handler entry path(인터럽트 처리기 진입 경로)입니다. 나머지 line은 `KERN_BASE` 아래의 address입니다. 이는 kernel이 아니라 user program의 address를 refer(참조)한다는 뜻입니다.

## GDB

GDB debugger(디버거)의 supervision(감독) 아래 Pintos를 실행할 수 있습니다. 먼저 `\--gdb` option(옵션)으로 Pintos를 시작합니다. 예: `pintos --gdb -- run mytest`. 둘째, 같은 machine(컴퓨터)의 두 번째 terminal(터미널)을 열고 gdb를 사용하여 'kernel.o'에 대해 GDB를 invoke합니다.
    
    
    gdb kernel.o
    

그리고 다음 GDB command(명령)를 입력합니다.
    
    
    target remote localhost:1234
    

이제 GDB는 local network connection(로컬 네트워크 연결)을 통해 simulator(시뮬레이터)에 연결됩니다. 이제 일반 GDB command를 무엇이든 입력할 수 있습니다. 'c' command를 입력하면 simulated BIOS(시뮬레이션된 BIOS)가 control을 가져가고 Pintos를 load(적재)한 뒤 Pintos가 usual way(일반 방식)로 실행됩니다. Ctrl+C로 어느 지점에서든 process(프로세스)를 pause(중지)할 수 있습니다.

### Using GDB (GDB 사용)

terminal command prompt(명령 프롬프트)에서 `info gdb`를 입력하면 GDB manual(매뉴얼)을 읽을 수 있습니다. 흔히 유용한 GDB command 몇 가지입니다.

  - c [GDB Command]

> hCtrl+Ci 또는 다음 breakpoint(중단점)까지 execution(실행)을 계속합니다.

  - break *function* [GDB Command]

  - break *file:line* [GDB Command]

  - break *address* [GDB Command]

> function, file 안의 line, 또는 address에 breakpoint를 설정합니다. 16진수 address를 지정하려면 '0x' prefix를 사용하세요. Pintos가 실행을 시작할 때 GDB가 stop하게 하려면 break main을 사용하세요.

  - p *expression* [GDB Command]

> 주어진 expression(식)을 evaluate하고 값을 print합니다. expression에 function call(함수 호출)이 포함되어 있으면 실제로 그 function이 execute(실행)됩니다.

  - l *address* [GDB Command]

> address 주변의 code 몇 line을 list(나열)합니다. 16진수 address를 지정하려면 '0x' prefix를 사용하세요.

  - bt [GDB Command]

> 위에서 설명한 backtrace program output과 비슷한 stack backtrace를 print합니다.

  - p/a *address* [GDB Command]

> address를 차지하는 function 또는 variable(변수)의 name을 print합니다. 16진수 address를 지정하려면 '0x' prefix를 사용하세요.

  - diassemble *function* [GDB Command]

> function을 disassemble(역어셈블)합니다.

이 글을 읽은 사람을 위한 추가 tip(팁): [pwndbg](https://github.com/pwndbg/pwndbg) 같은 gdb script(스크립트)를 install(설치)하세요. 훨씬 편해집니다.

### FAQ

  - Can I use GDB inside Emacs? (Emacs 안에서 GDB를 사용할 수 있나요?)

> 네, 가능합니다. Emacs는 GDB를 subprocess(하위 프로세스)로 실행하는 특별 support(지원)를 제공합니다. M-x gdb를 입력하고 prompt(프롬프트)에 pintos-gdb command를 입력하세요. Emacs manual에는 "Debuggers"라는 section에 debugging feature(기능) 사용법 정보가 있습니다.

  - GDB is doing something weird. (GDB가 이상하게 동작합니다.)

> GDB를 사용하는 동안 strange behavior(이상한 동작)를 발견하면 세 가지 possibility(가능성)가 있습니다. 수정한 Pintos의 bug, Bochs의 GDB interface나 GDB 자체의 bug, 또는 원본 Pintos code의 bug입니다. 첫 번째와 두 번째는 꽤 가능성이 높으며 둘 다 진지하게 고려해야 합니다. 세 번째는 가능성이 낮기를 바라지만 역시 가능합니다.

## Triple Faults (트리플 폴트)

page fault handler(페이지 폴트 처리기) 같은 CPU exception handler(CPU 예외 처리기)가 없거나 defective(결함 있음)하여 invoke될 수 없으면, CPU는 "double fault" handler를 invoke하려고 합니다. double fault handler 자체도 없거나 defective하면 이를 "triple fault"라고 합니다. triple fault는 즉시 CPU reset(리셋)을 일으킵니다.

따라서 machine이 loop(반복)로 reboot(재부팅)되는 상황에 빠졌다면 아마 "triple fault"일 것입니다. triple fault 상황에서는 `printf()` debugging을 사용하지 못할 수 있습니다. `printf()`에 필요한 모든 것이 initialize되기도 전에 reboot가 일어날 수 있기 때문입니다.

triple fault를 debug하는 방법이 있습니다.

Pintos code의 한 지점을 골라 infinite loop(무한 루프) `for (;;);`를 삽입하고 recompile(재컴파일)한 뒤 실행하세요. 가능성은 두 가지입니다.

  - machine이 reboot하지 않고 hang(멈춤)합니다. 이 경우 infinite loop가 실행 중임을 알 수 있습니다. 즉 reboot를 일으킨 원인은 infinite loop를 삽입한 지점 *이후*에 있어야 합니다. 이제 infinite loop를 code sequence(코드 순서)에서 더 뒤로 옮기세요.

  - machine이 loop로 reboot합니다. 이 경우 machine이 infinite loop까지 도달하지 못했음을 알 수 있습니다. 따라서 reboot를 일으킨 원인은 infinite loop를 삽입한 지점 *이전*에 있어야 합니다. 이제 infinite loop를 code sequence에서 더 앞쪽으로 옮기세요.

infinite loop를 "binary search(이진 탐색)" 방식으로 옮기면 이 technique을 사용해 모든 것이 잘못되는 정확한 지점을 pin down(찾아낼) 수 있습니다. 길어도 몇 분이면 충분합니다.

## Tips (팁)

`threads/palloc.c`의 page allocator(페이지 할당자)와 `threads/malloc.c`의 block allocator(블록 할당자)는 free 시점에 memory의 모든 byte를 '0xcc'로 clear(초기화)합니다. 따라서 `0xcccccccc` 같은 pointer(포인터)를 dereference(역참조)하려는 시도나 `0xcc`에 대한 다른 reference를 본다면 이미 free된 page를 재사용하려고 할 가능성이 큽니다. 또한 byte 0xcc는 "invoke interrupt 3"의 CPU opcode(명령 코드)이므로, `Interrupt 0x03 (#BP Breakpoint Exception)` 같은 error를 본다면 Pintos가 free된 page나 block의 code를 execute(실행)하려고 한 것입니다. expression `sec_no < d->capacity`에 대한 assertion failure는 Pintos가 close되고 free된 inode(아이노드)를 통해 file에 access하려고 했음을 나타냅니다. inode를 free하면 starting sector number(시작 섹터 번호)가 `0xcccccccc`로 clear되며, 이는 약 1.6 TB보다 작은 disk에서는 valid sector number(유효한 섹터 번호)가 아닙니다.
