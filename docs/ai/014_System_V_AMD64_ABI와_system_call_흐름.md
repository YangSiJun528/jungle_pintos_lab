# System V AMD64 ABI와 system call 흐름

## 기준 자료

- [System V AMD64 ABI - Function Calling Sequence](https://www.ucw.cz/~hubicka/papers/abi/node10.html)
- [System V AMD64 ABI - AMD64 Linux Kernel Conventions](https://www.ucw.cz/~hubicka/papers/abi/node33.html)
- [Intel 64 and IA-32 Architectures Software Developer Manuals](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- [INT n/INTO/INT3/INT1 - Call to Interrupt Procedure](https://www.felixcloutier.com/x86/intn%3Ainto%3Aint3%3Aint1)
- [IRET/IRETD/IRETQ - Interrupt Return](https://www.felixcloutier.com/x86/iret%3Airetd%3Airetq)

Felix Cloutier의 x86 reference는 Intel SDM 내용을 명령어별로 분리해 둔 비공식 자료다. 정확한 원문 기준은 Intel SDM이다.

## 먼저 구분해야 하는 것

ABI, instruction, system call은 서로 다른 층의 이야기다.

| 구분 | 설명 | 예시 |
| --- | --- | --- |
| ISA / instruction | CPU 명령어가 실제로 어떤 레지스터와 스택을 바꾸는지 정의한다. | `INT`, `IRETQ`, `SYSCALL`, `SYSRET` |
| ABI | 컴파일된 코드끼리 서로 맞물리기 위한 약속이다. | 함수 인자 위치, 반환값 위치, 보존해야 할 레지스터 |
| system call ABI | user mode가 kernel mode에 요청을 보낼 때 사용하는 별도의 약속이다. | syscall 번호 위치, syscall 인자 위치, 결과 위치 |

따라서 "반환값은 `RAX`에 둔다"는 말은 CPU 명령어 자체의 법칙이라기보다 ABI의 약속이다. 반대로 "`IRETQ`가 무엇을 pop해서 어디로 복원하는가"는 instruction 동작에 가까운 이야기다.

## ABI란 무엇인가

ABI는 Application Binary Interface의 줄임말이다. 소스 코드 수준의 API가 아니라, 컴파일이 끝난 binary code가 서로 호출될 때 지켜야 하는 규칙이다.

ABI가 정하는 대표적인 내용은 다음과 같다.

- 함수 인자를 어느 레지스터나 스택 위치에 둘지
- 함수 반환값을 어느 레지스터에 둘지
- 함수 호출 전후에 어떤 레지스터 값을 보존해야 하는지
- 스택 정렬을 어떻게 맞출지
- 구조체나 큰 값은 레지스터로 넘길지 메모리로 넘길지

컴파일러는 ABI를 기준으로 코드를 만든다. 그래서 호출하는 쪽과 호출받는 쪽이 같은 ABI를 따르면, 서로의 소스 코드를 몰라도 함수 호출이 맞물린다.

## 일반 함수 호출: System V AMD64 ABI

Linux, macOS 등 Unix 계열 x86-64 환경에서 흔히 말하는 System V AMD64 ABI 기준으로, 정수와 포인터 인자는 앞에서부터 다음 레지스터를 사용한다.

| 순서 | 위치 |
| --- | --- |
| 1번째 인자 | `RDI` |
| 2번째 인자 | `RSI` |
| 3번째 인자 | `RDX` |
| 4번째 인자 | `RCX` |
| 5번째 인자 | `R8` |
| 6번째 인자 | `R9` |
| 7번째 이후 | stack |

정수나 포인터 반환값은 보통 `RAX`에 둔다. 더 큰 정수 반환값 등 일부 경우에는 `RDX:RAX`처럼 둘 이상의 레지스터를 쓰거나, 메모리를 통한 반환 규칙이 적용될 수 있다.

일반 함수 호출의 큰 흐름은 다음과 같다.

```text
caller
  1. ABI 규칙에 맞게 인자를 레지스터와 스택에 배치한다.
  2. call 명령으로 callee로 이동한다.

callee
  3. ABI 규칙에 맞게 인자를 읽는다.
  4. 일을 수행한다.
  5. 반환값을 RAX 등에 둔다.
  6. ret 명령으로 caller로 돌아간다.

caller
  7. ABI 규칙에 따라 RAX를 반환값으로 해석한다.
```

여기서 중요한 점은 `RAX`가 특별히 "함수 반환 전용 하드웨어 레지스터"라서가 아니라, ABI가 그렇게 약속했기 때문에 caller가 `RAX`를 반환값으로 읽는다는 점이다.

## 인자가 7개 이상이면 어떻게 되는가

일반 System V AMD64 함수 호출에서 정수/포인터 인자 1-6개는 `RDI`, `RSI`, `RDX`, `RCX`, `R8`, `R9`를 사용한다. 더 이상 사용할 인자 레지스터가 없으면 나머지 인자는 stack에 놓인다.

예를 들어 정수 인자 8개짜리 일반 함수 호출은 개념적으로 다음처럼 배치된다.

| 인자 | 위치 |
| --- | --- |
| arg1 | `RDI` |
| arg2 | `RSI` |
| arg3 | `RDX` |
| arg4 | `RCX` |
| arg5 | `R8` |
| arg6 | `R9` |
| arg7 | stack |
| arg8 | stack |

System V AMD64 ABI 문서는 메모리로 전달되는 인자들이 stack에 놓인다고 설명한다. 이 내용은 Intel instruction manual의 주제가 아니라 ABI 문서의 주제다.

## 일반 함수 호출과 system call은 다르다

system call은 일반 C 함수 호출처럼 보일 수 있지만, 실제로는 user mode에서 kernel mode로 넘어가는 경계다.

일반 함수 호출은 같은 권한 수준 안에서 `call`과 `ret`로 이동한다. 반면 system call은 CPU의 특수한 진입 경로를 통해 kernel로 들어가고, kernel이 일을 마친 뒤 user mode 실행 문맥으로 돌아간다.

| 구분 | 일반 함수 호출 | system call |
| --- | --- | --- |
| 목적 | 같은 프로그램 내부 또는 라이브러리 함수 호출 | user mode에서 kernel 기능 요청 |
| 권한 전환 | 보통 없음 | user mode에서 kernel mode로 전환 |
| 진입 방식 | `call` | OS/ABI에 따라 `syscall`, software interrupt 등 |
| 복귀 방식 | `ret` | OS/CPU 경로에 따라 `sysret`, `iretq` 등 |
| 반환값 해석 | ABI에 따라 `RAX` | system call ABI에 따라 `RAX` |

즉, "system call이 반환한다"는 말은 C 함수가 `return`하는 것과 완전히 같은 기계 동작을 뜻하지 않는다. user code 입장에서 결과가 `RAX`에 놓이므로 함수 반환처럼 보이는 것이다.

## Linux x86-64 system call ABI

System V AMD64 ABI의 Linux kernel convention은 user-level 함수 호출과 system call 경계의 규칙이 조금 다르다고 설명한다.

Linux x86-64 system call ABI의 핵심은 다음과 같다.

| 항목 | 위치 |
| --- | --- |
| syscall 번호 | `RAX` |
| 1번째 인자 | `RDI` |
| 2번째 인자 | `RSI` |
| 3번째 인자 | `RDX` |
| 4번째 인자 | `R10` |
| 5번째 인자 | `R8` |
| 6번째 인자 | `R9` |
| 반환값 | `RAX` |

일반 함수 호출에서는 4번째 정수/포인터 인자가 `RCX`에 놓이지만, Linux system call ABI에서는 4번째 인자가 `R10`에 놓인다. `syscall` 계열 진입 경로가 `RCX`와 `R11`을 특수하게 사용하거나 손상시킬 수 있기 때문이다.

또 하나 중요한 차이는 Linux system call ABI가 system call 인자를 6개로 제한한다는 점이다. 일반 함수 호출에서는 7번째 이후 인자가 stack으로 갈 수 있지만, Linux system call ABI에서는 7번째 system call 인자를 stack으로 직접 전달하지 않는다.

## interrupt와 interrupt frame

`INT n` 같은 software interrupt나 예외, 외부 interrupt가 발생하면 CPU는 현재 실행을 잠시 멈추고 handler로 이동한다. 이때 CPU는 나중에 원래 흐름으로 돌아가기 위한 최소 실행 문맥을 stack에 저장한다.

64-bit interrupt gate 기준으로 중요한 저장 항목은 다음과 같다.

- 이전 `SS:RSP`
- 이전 `RFLAGS`
- 돌아갈 위치인 이전 `CS:RIP`
- 일부 예외에서의 error code

그 뒤 CPU는 IDT의 gate descriptor를 보고 handler의 `CS:RIP`로 이동한다. 즉, handler는 일반 함수처럼 caller가 `call`한 것이 아니라, CPU가 interrupt 규칙에 따라 강제로 진입시킨 실행 흐름이다.

운영체제는 이 CPU 저장 문맥에 더해 general-purpose register들을 추가로 저장할 수 있다. 이렇게 저장된 값들의 묶음을 C 코드에서 다루기 쉽게 표현하면 흔히 interrupt frame, trap frame 같은 형태가 된다.

## IRETQ는 무엇을 하는가

`IRETQ`는 64-bit interrupt return 명령이다. 일반 함수의 `ret`와 다르게, 단순히 return address만 꺼내는 것이 아니라 interrupt 진입 때 저장된 실행 문맥을 복원한다.

Intel 계열 reference 기준으로 `IRETQ`는 64-bit mode에서 대략 다음 값을 stack에서 꺼내 복원한다.

- `RIP`
- `CS`
- `RFLAGS`
- 필요한 경우 `RSP`
- 필요한 경우 `SS`

그래서 interrupt handler가 끝난 뒤에는 CPU가 저장된 `RIP` 위치로 돌아가고, 저장된 flags와 stack 문맥도 함께 복원된다.

## 왜 frame의 RAX를 바꾸면 반환값처럼 보이는가

핵심은 다음 두 사실이 합쳐지는 것이다.

1. ABI 또는 system call ABI는 반환값을 `RAX`에 둔다고 약속한다.
2. interrupt/system call 복귀 경로는 저장된 user register 문맥을 복원한다.

따라서 kernel이 저장된 user 문맥 안의 `RAX` 값을 바꾸고 user mode로 복귀하면, user code는 복귀 직후 실제 `RAX` 레지스터에서 그 값을 보게 된다.

이것은 C handler 함수의 반환값을 사용하는 것과 다르다. C handler가 어떤 값을 `return`하더라도, 그 값이 자동으로 user mode의 system call 반환값이 되는 것은 아니다. system call 반환값은 user mode로 복원될 레지스터 문맥, 특히 `RAX`에 무엇이 들어가느냐에 의해 결정된다.

개념 흐름은 다음과 같다.

```text
user code
  system call 진입

CPU / kernel entry
  user register context 저장

kernel handler
  요청 처리
  저장된 user RAX 자리에 결과값 배치

return to user
  저장된 register context 복원

user code
  ABI 규칙에 따라 RAX를 반환값으로 해석
```

## CPU instruction 규칙과 ABI 규칙의 연결

Intel instruction reference에서 확인할 수 있는 것은 CPU가 interrupt 진입과 복귀 때 어떤 값을 저장하고 복원하는가이다.

- `INT n`은 IDT를 통해 handler로 진입하고, return address와 flags 등 interrupt 복귀에 필요한 정보를 stack에 저장한다.
- `IRETQ`는 interrupt handler에서 돌아갈 때 `RIP`, `CS`, `RFLAGS`, 필요 시 `RSP`, `SS`를 복원한다.

System V AMD64 ABI에서 확인할 수 있는 것은 software가 레지스터를 어떤 의미로 사용하기로 약속했는가이다.

- 일반 함수 호출에서 정수/포인터 인자 1-6개는 `RDI`, `RSI`, `RDX`, `RCX`, `R8`, `R9`를 사용한다.
- 일반 함수 호출에서 7번째 이후 인자는 stack으로 간다.
- 정수/포인터 반환값은 보통 `RAX`에서 읽는다.
- Linux system call ABI에서는 syscall 번호가 `RAX`, 결과도 `RAX`에 놓인다.

둘을 합치면 다음처럼 이해할 수 있다.

CPU는 "`RAX`가 반환값이다"라고 해석하지 않는다. CPU는 그저 레지스터와 stack을 저장하고 복원한다. "`RAX`를 반환값으로 읽자"는 해석은 ABI를 따르는 user code와 compiler, runtime의 약속이다.

## 정리

- ABI는 binary code 사이의 호출 약속이다.
- Intel instruction reference는 CPU 명령어의 실제 동작을 설명한다.
- 일반 System V AMD64 함수 호출에서 인자 1-6개는 register, 7번째 이후는 stack으로 간다.
- 일반 함수 호출의 정수/포인터 반환값은 보통 `RAX`에 둔다.
- Linux x86-64 system call ABI는 일반 함수 호출과 다르게 4번째 인자에 `R10`을 쓰고, system call 인자를 6개로 제한한다.
- interrupt나 system call 복귀에서 반환값처럼 보이는 값은 C handler 함수의 return value가 아니라, user mode로 복원될 register context 안의 `RAX` 값이다.
