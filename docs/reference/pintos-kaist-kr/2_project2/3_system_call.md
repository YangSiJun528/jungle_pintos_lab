### System Calls (시스템 콜)

**system call infrastructure(시스템 콜 기반 구조)를 구현하세요.**

`userprog/syscall.c`의 system call handler(시스템 콜 처리기)를 구현하세요. 제공되는 skeleton implementation(골격 구현)은 process(프로세스)를 terminate(종료)하는 방식으로 system call을 "handle(처리)"합니다. 구현은 system call number(시스템 콜 번호)를 가져오고, 이어서 system call argument(인자)를 가져온 뒤, 적절한 action(동작)을 수행해야 합니다.

#### System Call Details (시스템 콜 세부 사항)

첫 번째 project에서는 operating system(운영체제)이 user program(사용자 프로그램)으로부터 control(제어)을 되찾는 한 가지 방법을 이미 다뤘습니다. timer(타이머)와 I/O device(입출력 장치)에서 발생하는 interrupt(인터럽트)입니다. 이들은 CPU 외부의 entity(요소)가 일으키므로 "external" interrupt라고 합니다.

operating system은 software exception(소프트웨어 예외)도 처리합니다. 이는 program code에서 발생하는 event(이벤트)입니다. page fault(페이지 폴트)나 division by zero(0으로 나누기) 같은 error(오류)가 여기에 해당합니다. exception은 user program이 operating system에 service(서비스), 즉 "system call"을 요청하는 수단이기도 합니다.

전통적인 x86 architecture(아키텍처)에서는 system call이 다른 software exception과 같은 방식으로 처리되었습니다. 하지만 x86-64에서는 제조사가 system call을 위한 특수 instruction(명령어)인 `syscall`을 도입했습니다. 이는 system call handler를 빠르게 호출하는 방법을 제공합니다.

오늘날 x86-64에서 system call을 invoke(호출)하는 가장 일반적인 수단은 `syscall` instruction입니다. Pintos에서 user program은 system call을 만들기 위해 `syscall`을 invoke합니다. `syscall` instruction을 invoke하기 전에, 두 가지 예외를 제외하고 normal fashion(일반 관례)에 따라 system call number와 추가 argument가 register(레지스터)에 설정되어 있어야 합니다.

  - `%rax`는 system call number입니다.
  - 네 번째 argument는 `%rcx`가 아니라 `%r10`입니다.

따라서 system call handler `syscall_handler()`가 control을 얻으면 system call number는 rax에 있고, argument는 %rdi, %rsi, %rdx, %r10, %r8, %r9 순서로 전달됩니다.

caller(호출자)의 register는 handler에 전달된 `struct intr_frame`을 통해 접근할 수 있습니다. (`struct intr_frame`은 kernel stack(커널 스택)에 있습니다.)

x86-64 convention(관례)에서 function return value(함수 반환값)는 RAX register에 둡니다. 값을 return하는 system call은 `struct intr_frame`의 `rax` member(멤버)를 수정하여 값을 반환할 수 있습니다.

#### Implement the following system calls. (다음 system call을 구현하세요.)

나열된 prototype(원형)은 `include/lib/user/syscall.h`를 include(포함)하는 user program이 보는 것입니다. 이 header와 `include/lib/user`의 다른 모든 header는 user program에서만 사용합니다. 각 system call의 system call number는 `include/lib/syscall-nr.h`에 정의되어 있습니다.

* * *
    
    
    void halt (void);
    

> `power_off()`(`src/include/threads/init.h`에 선언)을 호출하여 Pintos를 terminate합니다. deadlock(교착 상태) 가능성 등에 대한 일부 information(정보)을 잃게 되므로 거의 사용하지 않아야 합니다.

* * *
    
    
    void exit (int status);
    

> 현재 user program을 terminate하고 `status`를 kernel(운영체제 핵심부)에 반환합니다. process의 parent(부모)가 아래 설명된 것처럼 이를 `wait`하면 이 status가 반환됩니다. 관례상 `status`가 `0`이면 success(성공)를, nonzero(0이 아닌 값)는 error를 나타냅니다.

* * *
    
    
    pid_t fork (const char *thread_name);
    

> THREAD_NAME이라는 이름으로 현재 process의 clone(복제본)인 새 process를 만듭니다. `%RBX`, `%RSP`, `%RBP`, 그리고 `%R12` - `%R15`처럼 callee-saved register(피호출자 보존 레지스터)를 제외하면 register 값을 clone할 필요는 없습니다. child process(자식 프로세스)의 pid를 반환해야 하며, 실패하면 valid pid(유효한 pid)가 아니어야 합니다. child process에서는 return value가 `0`이어야 합니다. child는 file descriptor(파일 디스크립터)와 virtual memory space(가상 메모리 공간)를 포함한 resource(자원)를 **DUPLICATED(복제)**해야 합니다. parent process는 child process가 성공적으로 clone되었는지 알기 전에는 fork에서 return해서는 안 됩니다. 즉 child process가 resource duplicate에 실패하면 parent의 fork() call은 TID_ERROR를 return해야 합니다.
> 
> template(템플릿)은 대응되는 pagetable structure(페이지 테이블 구조)를 포함한 전체 user memory space(사용자 메모리 공간)를 copy하기 위해 `threads/mmu.c`의 `pml4_for_each()`를 활용하지만, 전달된 `pte_for_each_func`의 빠진 부분은 여러분이 채워야 합니다([virtual address](../5_appendix/3_virtual_address.md) 참조).

* * *
    
    
    int exec (const char *cmd_line);
    

> 현재 process를 `cmd_line`에 주어진 이름의 executable(실행 파일)로 변경하고, 주어진 argument를 전달합니다. 성공하면 절대 return하지 않습니다. 어떤 이유로 program을 load하거나 run할 수 없으면 process는 exit state(종료 상태) `-1`로 *terminate*됩니다. 이 function은 `exec`을 호출한 thread의 이름을 변경하지 않습니다. file descriptor는 `exec` call을 지나도 open 상태로 유지된다는 점에 유의하세요.

* * *
    
    
    int wait (pid_t pid);
    

> child process `pid`를 기다리고 child의 exit status(종료 상태)를 가져옵니다. `pid`가 아직 살아 있다면 terminate될 때까지 기다립니다. 그런 다음 `pid`가 exit에 전달한 status를 반환합니다. `pid`가 `exit()`을 호출하지 않고 kernel에 의해 terminate된 경우, 예를 들어 exception 때문에 kill(종료)된 경우 `wait(pid)`는 `-1`을 반환해야 합니다. parent process가 wait를 호출할 시점에 이미 terminate된 child process를 기다리는 것은 완전히 합법입니다. 하지만 kernel은 그래도 parent가 child의 exit status를 가져오거나 child가 kernel에 의해 terminate되었음을 알 수 있게 해야 합니다.
> 
> 다음 조건 중 하나라도 참이면 `wait`는 즉시 fail(실패)하고 `-1`을 반환해야 합니다.
> 
>   - `pid`가 calling process(호출 프로세스)의 direct child(직접 자식)를 가리키지 않습니다. calling process가 성공적인 `fork` call의 return value로 `pid`를 받았을 때, 그리고 그때에만 `pid`는 calling process의 direct child입니다. child는 inherit(상속)되지 않는다는 점에 주의하세요. A가 child B를 spawn(생성)하고 B가 child process C를 spawn했다면, B가 죽었더라도 A는 C를 wait할 수 없습니다. process A의 `wait(C)` call은 fail해야 합니다. 마찬가지로 orphaned process(고아 프로세스)는 parent process가 먼저 exit하더라도 새 parent에게 할당되지 않습니다.
>   - `wait`를 호출하는 process가 이미 `pid`에 대해 `wait`를 호출했습니다. 즉, process는 어떤 child에 대해서도 최대 한 번만 `wait`할 수 있습니다.

process는 원하는 수의 child를 spawn할 수 있고, 어떤 순서로든 이들을 `wait`할 수 있으며, 일부 또는 모든 child를 wait하지 않은 채 exit할 수도 있습니다. 여러분의 design은 wait가 발생할 수 있는 모든 방식을 고려해야 합니다. process의 모든 resource는 parent가 wait하든 하지 않든, child가 parent보다 먼저 exit하든 나중에 exit하든, `struct thread`를 포함하여 반드시 free되어야 합니다.

**initial process(초기 프로세스)가 exit할 때까지 Pintos가 terminate되지 않도록 보장해야 합니다.** 제공된 Pintos code는 `threads/init.c`의 `main()`에서 `userprog/process.c`의 `process_wait()`를 호출하여 이를 시도합니다. `process_wait()`를 function 맨 위의 comment(주석)에 따라 구현하고, wait system call을 `process_wait()` 기반으로 구현하는 것을 제안합니다.

이 system call을 구현하려면 나머지 어떤 system call보다 훨씬 많은 작업이 필요합니다.

* * *
    
    
    bool create (const char *file, unsigned initial_size);
    

> 처음에 `initial_size` bytes(바이트) 크기인 `file`이라는 새 file(파일)을 만듭니다. 성공하면 true, 실패하면 false를 반환합니다. 새 file을 만드는 것은 그것을 open하는 것이 아닙니다. 새 file을 open하려면 별도의 `open` system call이 필요합니다.

* * *
    
    
    bool remove (const char *file);
    

> `file`이라는 file을 delete(삭제)합니다. 성공하면 true, 실패하면 false를 반환합니다. file이 open되어 있든 closed(닫힌) 상태이든 remove될 수 있으며, open file을 remove해도 그 file이 close되지는 않습니다. 자세한 내용은 [FAQ](7_FAQ.md)의 Removing an Open File을 참조하세요.

* * *
    
    
    int open (const char *file);
    

> `file`이라는 file을 open합니다. "file descriptor"(fd, 열린 파일을 가리키는 정수 handle)라고 부르는 nonnegative integer handle(음이 아닌 정수 핸들)을 반환하거나, file을 open할 수 없으면 `-1`을 반환합니다. file descriptor 0과 1은 console(콘솔)을 위해 예약되어 있습니다. fd 0(`STDIN_FILENO`)은 standard input(표준 입력), fd 1(`STDOUT_FILENO`)은 standard output(표준 출력)입니다. `open` system call은 이 file descriptor 중 어느 것도 반환하지 않으며, 이들은 아래에 명시적으로 설명된 경우에만 system call argument로 valid합니다. 각 process는 독립적인 file descriptor set(집합)을 가집니다. file descriptor는 child process가 inherit합니다. 하나의 file이 한 process 또는 서로 다른 process에 의해 여러 번 open되면, 각 open은 새 file descriptor를 반환합니다. 같은 file에 대한 서로 다른 file descriptor는 close에 대한 별도 call에서 독립적으로 close되며 file position(파일 위치)을 공유하지 않습니다. **extra를 수행하려면 0부터 시작하는 integer를 반환하는 linux scheme(리눅스 방식)을 따라야 합니다.**

* * *
    
    
    int filesize (int fd);
    

> `fd`로 open된 file의 size를 bytes 단위로 반환합니다.

* * *
    
    
    int read (int fd, void *buffer, unsigned size);
    

> `fd`로 open된 file에서 `size` bytes를 읽어 `buffer`에 넣습니다. 실제로 읽은 byte 수를 반환합니다(end of file(파일 끝)에서는 `0`). end of file 이외의 condition(조건) 때문에 file을 읽을 수 없으면 `-1`을 반환합니다. `fd` 0은 `input_getc()`를 사용하여 keyboard(키보드)에서 읽습니다.

* * *
    
    
    int write (int fd, const void *buffer, unsigned size);
    

> `buffer`에서 `size` bytes를 open file `fd`에 씁니다. 실제로 쓴 byte 수를 반환하며, 어떤 byte를 쓸 수 없으면 이 값은 `size`보다 작을 수 있습니다. end-of-file을 지나 쓰면 일반적으로 file이 extend(확장)되지만, basic file system(기본 파일 시스템)은 file growth(파일 증가)를 구현하지 않습니다. 기대되는 동작은 end-of-file까지 가능한 많은 byte를 쓰고 실제로 쓴 수를 반환하거나, 전혀 쓸 수 없으면 `0`을 반환하는 것입니다. `fd` 1은 console에 씁니다. console에 쓰는 여러분의 code는 size가 몇백 byte보다 크지 않은 한 buffer 전체를 한 번의 `putbuf()` call로 써야 합니다. 더 큰 buffer를 나누는 것은 합리적입니다. 그렇지 않으면 서로 다른 process가 출력한 text line(텍스트 줄)이 console에서 interleave(뒤섞임)되어 사람 독자와 grading script(채점 스크립트)를 모두 혼란스럽게 할 수 있습니다.

* * *
    
    
    void seek (int fd, unsigned position);
    

> open file `fd`에서 다음에 read 또는 write할 byte 위치를 file 시작부터 byte 단위로 표현한 `position`으로 변경합니다. 따라서 `position`이 `0`이면 file의 시작입니다. file의 현재 end를 지난 seek는 error가 아닙니다. 이후 `read`는 0 bytes를 얻어 end of file을 나타냅니다. 이후 `write`는 file을 extend하고 쓰이지 않은 gap(빈 구간)을 zero(0)로 채웁니다. 하지만 Pintos에서는 project 4가 완료될 때까지 file 길이가 고정이므로, end of file을 지난 `write`는 error를 반환합니다. 이 semantics는 file system에 구현되어 있으며 system call implementation에서 특별히 노력할 필요가 없습니다.

* * *
    
    
    unsigned tell (int fd);
    

> open file `fd`에서 다음에 read 또는 write할 byte의 위치를 file 시작부터 byte 단위로 표현해 반환합니다.

* * *
    
    
    void close (int fd);
    

> file descriptor `fd`를 close합니다. process가 exit하거나 terminate되면, 각 open file descriptor에 대해 이 function을 호출한 것처럼 모든 open file descriptor가 implicitly(암묵적으로) close됩니다.

file은 다른 syscall(시스템 콜)도 정의합니다. 지금은 무시하세요. 일부는 project 3에서, 나머지는 project 4에서 구현하게 되므로 extensibility(확장성)를 염두에 두고 system을 design하세요.

어떤 수의 user process든 system call을 동시에 만들 수 있도록 system call을 synchronize(동기화)해야 합니다. 특히 여러 thread에서 동시에 `filesys` directory에 제공된 file system code를 호출하는 것은 safe(안전)하지 않습니다. system call implementation은 file system code를 critical section(임계 구역)으로 취급해야 합니다. `process_exec()`도 file에 접근한다는 점을 잊지 마세요. 현재로서는 `filesys` directory의 code를 수정하지 않는 것을 권장합니다.

각 system call마다 user-level function(사용자 수준 함수)을 `lib/user/syscall.c`에 제공했습니다. 이 함수들은 user process가 C program에서 각 system call을 invoke할 방법을 제공합니다. 각 함수는 약간의 inline assembly code(인라인 어셈블리 코드)를 사용해 system call을 invoke하고, 적절한 경우 system call의 return value를 반환합니다.

이 part를 완료한 뒤, 그리고 앞으로도 계속, Pintos는 bulletproof(사용자 프로그램이 망가뜨릴 수 없을 정도로 견고)해야 합니다. user program이 할 수 있는 어떤 동작도 OS를 crash(충돌), panic(패닉), assertion failure(단언 실패), 또는 다른 malfunction(오동작)에 빠뜨려서는 안 됩니다. 이 점은 매우 중요합니다. 강사진의 test는 system call을 매우 다양한 방식으로 깨뜨리려고 시도할 것입니다. 모든 corner case(모서리 사례)를 생각하고 처리해야 합니다. user program이 OS를 halt(중지)하게 만들 수 있는 유일한 방법은 halt system call을 invoke하는 것이어야 합니다.

system call에 invalid argument(잘못된 인자)가 전달되면, 허용되는 option은 error value(오류 값)를 반환하는 것(값을 반환하는 call의 경우), undefined value(정의되지 않은 값)를 반환하는 것, 또는 process를 terminate하는 것입니다.
