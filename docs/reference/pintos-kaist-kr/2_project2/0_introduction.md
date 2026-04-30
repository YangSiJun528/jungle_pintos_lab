# Project2: User Programs (사용자 프로그램)

이제 Pintos를 다뤄 보았고 그 infrastructure(기반 구조)와 thread package(스레드 패키지)에 익숙해지고 있으므로, user program(사용자 프로그램)을 실행할 수 있게 하는 system(시스템) 부분을 작업할 차례입니다. base code(기본 코드)는 이미 user program을 load(적재)하고 실행하는 기능을 지원하지만, I/O(입출력)나 interactivity(상호작용)는 불가능합니다. 이 project에서는 program이 system call(운영체제 서비스를 요청하는 호출)을 통해 OS(운영체제)와 상호작용할 수 있게 합니다. 이번 assignment(과제)에서는 `userprog` directory(디렉터리)에서 작업하지만, Pintos의 거의 모든 다른 부분과도 상호작용하게 됩니다. 관련 부분은 아래에서 설명합니다.

**project 2는 반드시 project 1 제출물 위에 build(빌드)해야 합니다.** project 1의 code가 project 2 code에 영향을 주지는 않지만, 이 project는 incremental project(점진적 프로젝트)이므로 project 1 test case(테스트 사례)도 통과해야 합니다.

또한 project2에는 extra challenge(추가 도전 과제)가 있음을 알려 둡니다. **이것은 선택 구현입니다.** extra challenge에 대해서는 test case 외에 skeleton code(골격 코드)가 없습니다. 모든 design(설계)은 여러분에게 달려 있습니다. **extra requirement(추가 요구사항)를 제출하고 test하려면 `userprog/Make.vars`를 edit(수정)해야 합니다.**

마지막으로, `TODO`가 없는 code가 항상 수정할 필요가 없다는 뜻은 아닙니다. testing code(테스트 코드)를 제외하면 project2의 어떤 source code(소스 코드)든 자유롭게 수정할 수 있습니다.

## Background (배경)

지금까지 Pintos 아래에서 실행한 모든 code는 operating system kernel(운영체제 핵심부)의 일부였습니다. 예를 들어 지난 assignment의 모든 test code는 system의 privileged part(권한 있는 부분)에 완전히 접근할 수 있는 상태로 kernel의 일부로 실행되었습니다. operating system 위에서 user program을 실행하기 시작하면 더 이상 그렇지 않습니다. 이 project는 그 결과를 다룹니다.

한 번에 둘 이상의 process(프로세스)가 실행될 수 있도록 허용합니다. 각 process는 하나의 thread(실행 흐름)를 가집니다(multithreaded process는 지원하지 않습니다). user program은 자신이 machine(컴퓨터) 전체를 가지고 있다는 illusion(착각) 아래 작성됩니다. 즉, 여러 process를 동시에 load하고 실행할 때에는 이 illusion을 유지하기 위해 memory(메모리), scheduling(실행 배정), 기타 state(상태)를 올바르게 관리해야 합니다.

이전 project에서는 test code를 여러분의 kernel에 직접 compile(컴파일)했기 때문에, kernel 안의 특정 function interface(함수 인터페이스)를 요구해야 했습니다. 이제부터는 user program을 실행하여 여러분의 operating system을 test합니다. 이 방식은 훨씬 큰 freedom(자유도)을 제공합니다. user program interface가 여기 설명된 specification(명세)을 만족하도록 해야 하지만, 그 제약 안에서는 원하는 대로 kernel code를 restructure(재구성)하거나 rewrite(다시 작성)해도 됩니다. 여러분의 모든 code는 `#ifdef VM`으로 둘러싸인 block(블록) 안에 위치해서는 안 됩니다. 이 block들은 project 3에서 구현할 virtual memory subsystem(가상 메모리 하위 시스템)을 enable(활성화)한 뒤 포함됩니다. 또한 code가 `#ifndef VM`으로 둘러싸여 있다면 project3에서는 그 code가 생략됩니다.

**시작하기 전에 [synchronization](../5_appendix/1_synchronization.md)과 [virtual addrees](../5_appendix/3_virtual_address.md)를 읽는 것을 강하게 권장합니다.**

### Source Files (소스 파일)

여러분이 하게 될 programming(프로그래밍)의 overview(개요)를 얻는 가장 쉬운 방법은 작업하게 될 각 부분을 훑어보는 것입니다. `userprog`에는 적은 수의 file만 있지만, 작업의 대부분은 여기에 있습니다.

  - `process.c`, `process.h`

ELF binary(실행 파일 형식의 바이너리)를 load하고 process를 시작합니다.

  - `syscall.c`, `syscall.h`

user process(사용자 프로세스)가 kernel 기능에 접근하고 싶을 때마다 system call을 invoke(호출)합니다. 이것은 skeleton system call handler(시스템 콜 처리기)입니다. 현재는 message(메시지)를 출력하고 user process를 terminate(종료)할 뿐입니다. 이 project의 part 2에서 system call에 필요한 나머지 모든 일을 수행하는 code를 추가합니다.

  - `syscall-entry.S`

system call handler를 bootstrap(초기 진입 준비)하는 작은 assembly code(어셈블리 코드)입니다. 이 code를 이해할 필요는 없습니다.

  - `exception.c`, `exception.h`

user process가 privileged operation(권한 있는 작업)이나 prohibited operation(금지된 작업)을 수행하면 `exception` 또는 `fault`로 kernel에 trap(트랩)됩니다. 이 file들은 exception을 처리합니다. 현재 모든 exception은 단순히 message를 출력하고 process를 terminate합니다. project 2의 일부 solution은 이 file의 `page_fault()`를 수정해야 하지만, 모든 solution이 그런 것은 아닙니다.

  - `gdt.c`, `gdt.h`

x86-64는 segmented architecture(세그먼트 기반 아키텍처)입니다. Global Descriptor Table(GDT, 전역 디스크립터 테이블)은 사용 중인 segment(세그먼트)를 설명하는 table(표)입니다. 이 file들은 GDT를 설정합니다. 어떤 project에서도 이 file들을 수정할 필요는 없습니다. GDT가 어떻게 동작하는지 궁금하다면 code를 읽어 볼 수 있습니다.

  - `tss.c`, `tss.h`

Task-State Segment(TSS, 태스크 상태 세그먼트)는 x86 architecture에서 task switching(태스크 전환)에 사용되었습니다. 하지만 x86-64에서는 task switching이 deprecated(사용 권장되지 않음)되었습니다. 그럼에도 TSS는 ring switching(권한 링 전환) 중 stack pointer(스택 포인터)를 찾기 위해 여전히 존재합니다.

즉 user process가 interrupt handler(인터럽트 처리기)에 들어가면, hardware(하드웨어)는 kernel의 stack pointer를 찾기 위해 tss를 참조합니다. 어떤 project에서도 이 file들을 수정할 필요는 없습니다. TSS가 어떻게 동작하는지 궁금하다면 code를 읽어 볼 수 있습니다.

### Using the File System (파일 시스템 사용하기)

이번 project에서는 file system code(파일 시스템 코드)와 interface(연결)해야 합니다. user program은 file system에서 load되며, 구현해야 하는 많은 system call이 file system을 다루기 때문입니다. 하지만 이 project의 초점은 file system이 아니므로, `filesys` directory에 단순하지만 완전한 file system을 제공했습니다. file system을 어떻게 사용할지, 특히 그 많은 limitation(제한)을 이해하기 위해 `filesys.h`와 `file.h` interface를 살펴보는 것이 좋습니다.

**이 project에서는 file system code를 수정할 필요가 없으며**, 따라서 수정하지 않는 것을 권장합니다. file system 작업은 이 project의 초점에서 벗어나게 만들 가능성이 큽니다.

**지금 file system routine(루틴)을 올바르게 사용하면 project 4에서 훨씬 쉬워집니다.** project 4에서는 file system implementation(구현)을 개선하게 됩니다. 그 전까지는 다음 limitation을 견뎌야 합니다.

  - internal synchronization(내부 동기화)이 없습니다. concurrent access(동시 접근)는 서로 간섭합니다. 한 번에 하나의 process만 file system code를 실행하도록 synchronization을 사용해야 합니다.
  - file size(파일 크기)는 creation time(생성 시점)에 고정됩니다. root directory(루트 디렉터리)는 file로 표현되므로 생성할 수 있는 file 수에도 제한이 있습니다.
  - file data는 single extent(단일 연속 영역)로 allocation(할당)됩니다. 즉 한 file의 data는 disk의 연속된 sector(섹터) 범위를 차지해야 합니다. 따라서 file system을 오래 사용하면 external fragmentation(외부 단편화)이 심각한 문제가 될 수 있습니다.
  - subdirectory(하위 디렉터리)가 없습니다.
  - file name은 14 character(문자)로 제한됩니다.
  - operation(작업) 중간의 system crash(시스템 충돌)는 자동으로 복구할 수 없는 방식으로 disk를 corrupt(손상)시킬 수 있습니다. 어차피 file system repair tool(복구 도구)도 없습니다.

**중요한 feature(기능) 하나는 포함되어 있습니다.**

`filesys_remove()`에 대한 Unix-like semantics(유닉스식 의미)가 구현되어 있습니다. 즉, open(열린) 상태인 file이 remove(삭제)되면 그 block은 deallocate(할당 해제)되지 않으며, 그 file을 열고 있는 thread는 마지막 thread가 close(닫기)할 때까지 계속 접근할 수 있습니다. 자세한 내용은 [Removing an Open File](7_FAQ.md)을 참조하세요.

* * *

모든 test program이 kernel image(커널 이미지)에 이미 들어 있던 project 1과 달리, project 2에서는 user space(사용자 공간)에서 실행되는 test program을 Pintos virtual machine(가상 머신)에 넣어야 합니다. 이를 쉽게 하기 위해 testing script(`make check`)가 이 작업을 자동으로 처리하므로, 대부분의 경우 이를 이해할 필요는 없습니다. 하지만 이를 알고 있으면 individual test case(개별 테스트 사례)를 실행하는 데 큰 도움이 됩니다.

Pintos virtual machine에 file을 넣으려면 먼저 file system partition(파일 시스템 파티션)이 있는 simulated disk(시뮬레이션된 디스크)를 만들 수 있어야 합니다. `pintos-mkdisk` program이 이 기능을 제공합니다. `userprog/build` directory에서 `pintos-mkdisk filesys.dsk 2`를 실행하세요. 이 command는 2 MB Pintos file system partition을 포함하는 `filesys.dsk`라는 simulated disk를 만듭니다. 그런 다음 `pintos` 뒤, `\--` 앞에 `\--fs-disk filesys.dsk`를 전달하여 disk를 지정합니다. 즉 `pintos --fs-disk filesys.disk -- KERNEL_COMMANDS...` 형태입니다. `\--`가 필요한 이유는 `\--fs-disk`가 simulated kernel이 아니라 pintos script를 위한 option이기 때문입니다. 이후 kernel command line(커널 명령줄)에 `-f -q`를 전달하여 file system partition을 format(포맷)합니다. 예: `pintos SCRIPT_COMMANDS -- -f -q`. `-f` option은 file system을 format하게 하고, `-q`는 format이 끝나자마자 Pintos가 exit(종료)하게 합니다.

simulated file system 안팎으로 file을 copy(복사)할 방법이 필요합니다. pintos의 `-p`("put")와 `-g`("get") option이 이를 수행합니다. `file`을 Pintos file system으로 copy하려면 `pintos -p file -- -q` command를 사용하세요. Pintos file system에 `newname`이라는 이름으로 copy하려면 원래 filename 뒤에 `:newname`을 붙입니다. `pintos -p file:newname -- -q`처럼 사용합니다. VM에서 file을 copy해 나오는 command도 비슷하지만 `-p` 대신 `-g`를 사용합니다.

덧붙이면, 이 command들은 kernel command line에 특수 command인 extract와 append를 전달하고, 특수 simulated "scratch"(임시 작업) partition과 file을 주고받는 방식으로 동작합니다. 아주 궁금하다면 pintos script와 `filesys/fsutil.c`를 살펴 구현 detail(세부 사항)을 볼 수 있습니다.

다음은 file system partition이 있는 disk를 만들고, file system을 format하고, 이 project의 두 번째 test case인 *args-single* program을 새 disk로 copy한 뒤, argument(인자) `onearg`를 전달해 실행하는 방법의 요약입니다. argument passing(인자 전달)은 여러분이 구현하기 전까지 동작하지 않습니다. test case를 이미 build했고 current directory(현재 디렉터리)가 `userprog/build`라고 가정합니다.
    
    
    pintos-mkdisk filesys.dsk 10
    pintos --fs-disk filesys.dsk -p tests/userprog/args-single:args-single -- -q -f run 'args-single onearg'
    

나중에 사용하거나 검사하기 위해 file system disk를 남겨 둘 필요가 없다면, 네 단계를 하나의 command로 합칠 수도 있습니다. `\--filesys-size=n` option은 pintos 실행 동안에만 사용할 약 n megabytes(메가바이트) 크기의 temporary file system partition(임시 파일 시스템 파티션)을 만듭니다. Pintos automatic test suite(자동 테스트 모음)는 이 syntax(문법)를 많이 사용합니다.
    
    
    pintos --fs-disk=10 -p tests/userprog/args-single:args-single -- -q -f run 'args-single onearg'
    

### How User Programs Work (User Program 동작 방식)

Pintos는 memory에 들어가고 여러분이 구현한 system call만 사용하는 한 normal C program(일반 C 프로그램)을 실행할 수 있습니다. 특히 이 project에 필요한 system call 중 memory allocation(메모리 할당)을 허용하는 것이 없으므로 `malloc()`은 구현할 수 없습니다. 또한 Pintos는 floating point operation(부동소수점 연산)을 사용하는 program도 실행할 수 없습니다. thread를 switch할 때 kernel이 processor의 floating-point unit(부동소수점 장치)을 save(저장)하고 restore(복원)하지 않기 때문입니다.

Pintos는 `userprog/process.c`에 제공된 loader(로더)로 ELF executable(ELF 실행 파일)을 load할 수 있습니다. ELF는 Linux, Solaris, 그리고 많은 다른 operating system에서 object file(목적 파일), shared library(공유 라이브러리), executable(실행 파일)에 사용하는 file format(파일 형식)입니다.

x86-64 ELF executable을 output(출력)하는 compiler(컴파일러)와 linker(링커)라면 무엇이든 사용하여 Pintos용 program을 만들 수 있습니다. 적절하게 동작할 compiler와 linker를 제공했습니다. test program을 simulated file system에 copy하기 전까지 Pintos는 유용한 일을 할 수 없다는 점을 바로 알아야 합니다. 여러 program을 file system에 copy하기 전에는 흥미로운 일을 할 수 없습니다. debugging(디버깅) 중 가끔 `filesys.dsk`를 유용하지 않은 상태로 망가뜨릴 수 있으므로, 깨끗한 reference file system disk(참고 파일 시스템 디스크)를 만들어 두고 필요할 때 copy하고 싶을 수 있습니다.

### Virtual Memory Layout (가상 메모리 배치)

Pintos의 virtual memory(가상 메모리)는 user virtual memory(사용자 가상 메모리)와 kernel virtual memory(커널 가상 메모리)라는 두 region(영역)으로 나뉩니다. user virtual memory는 virtual address(가상 주소) `0`부터 `KERN_BASE`까지입니다. `KERN_BASE`는 `include/threads/vaddr.h`에 정의되어 있으며 기본값은 `0x8004000000`입니다. kernel virtual memory는 virtual address space(가상 주소 공간)의 나머지를 차지합니다.

user virtual memory는 per-process(프로세스별)입니다. kernel이 한 process에서 다른 process로 switch할 때, processor의 page directory base register(페이지 디렉터리 기준 레지스터)를 변경하여 user virtual address space도 switch합니다(`thread/mmu.c`의 `pml4_activate()` 참조). `struct thread`는 process의 page table(페이지 테이블)을 가리키는 pointer(포인터)를 포함합니다.

kernel virtual memory는 global(전역)입니다. 어떤 user process나 kernel thread가 실행 중이든 항상 같은 방식으로 mapping(매핑)됩니다. Pintos에서 kernel virtual memory는 `KERN_BASE`부터 시작하여 physical memory(물리 메모리)에 one-to-one(일대일)로 mapping됩니다. 즉 virtual address `KERN_BASE`는 physical address `0`에 접근하고, virtual address `KERN_BASE + 0x1234`는 physical address `0x1234`에 접근하며, machine의 physical memory 크기까지 계속됩니다.

user program은 자신의 user virtual memory에만 접근할 수 있습니다. kernel virtual memory에 접근하려고 하면 `userprog/exception.c`의 `page_fault()`가 처리하는 page fault(페이지 폴트)가 발생하고 process는 terminate됩니다. kernel thread는 kernel virtual memory와, user process가 실행 중이라면 실행 중인 process의 user virtual memory 양쪽에 접근할 수 있습니다. 하지만 kernel 안에서도 unmapped user virtual address(매핑되지 않은 사용자 가상 주소)의 memory에 접근하려고 하면 page fault가 발생합니다.

### Typical Memory Layout (일반적인 메모리 배치)

개념적으로 각 process는 원하는 방식으로 자신의 user virtual memory를 배치할 수 있습니다. 실제로는 user virtual memory가 다음처럼 배치됩니다.
    
    
    USER_STACK +----------------------------------+
               |             user stack           |
               |                 |                |
               |                 |                |
               |                 V                |
               |           grows downward         |
               |                                  |
               |                                  |
               |                                  |
               |                                  |
               |           grows upward           |
               |                 ^                |
               |                 |                |
               |                 |                |
               +----------------------------------+
               | uninitialized data segment (BSS) |
               +----------------------------------+
               |     initialized data segment     |
               +----------------------------------+
               |            code segment          |
     0x400000  +----------------------------------+
               |                                  |
               |                                  |
               |                                  |
               |                                  |
               |                                  |
           0   +----------------------------------+
    

이 project에서는 user stack(사용자 스택)의 크기가 고정되어 있지만, project 3에서는 grow(성장)할 수 있게 됩니다. 전통적으로 uninitialized data segment(BSS, 초기화되지 않은 데이터 세그먼트)의 크기는 system call로 조정할 수 있지만, 여러분은 이를 구현할 필요가 없습니다.

Pintos의 code segment(코드 세그먼트)는 user virtual address 0x400000에서 시작합니다. 이는 address space(주소 공간)의 아래쪽에서 약 128 MB 떨어진 위치입니다. 이 값은 ubuntu에서 일반적이던 값이며 깊은 의미는 없습니다.

linker는 user program의 memory layout(메모리 배치)을 설정합니다. 이는 다양한 program segment(프로그램 세그먼트)의 이름과 위치를 알려 주는 "linker script(링커 스크립트)"의 지시에 따릅니다. linker manual의 "Scripts" 장을 읽으면 linker script에 대해 더 배울 수 있으며, `info ld`로 접근할 수 있습니다.

특정 executable의 layout을 보려면 objdump를 `-p` option과 함께 실행하세요.

### Accessing User Memory (사용자 메모리 접근)

system call의 일부로 kernel은 user program이 제공한 pointer(포인터)를 통해 memory에 접근해야 하는 경우가 많습니다. 이때 kernel은 매우 조심해야 합니다. user는 null pointer(널 포인터), unmapped virtual memory를 가리키는 pointer, 또는 kernel virtual address space(`KERN_BASE` 위)를 가리키는 pointer를 전달할 수 있기 때문입니다. 이런 모든 invalid pointer(잘못된 포인터)는 kernel이나 다른 실행 중인 process에 피해를 주지 않고 거부되어야 하며, 문제를 일으킨 process를 terminate하고 resource(자원)를 free(해제)해야 합니다.

이를 올바르게 수행하는 합리적인 방법은 적어도 두 가지가 있습니다. 첫 번째 방법은 user가 제공한 pointer의 validity(유효성)를 verify(검증)한 다음 dereference(역참조)하는 것입니다. 이 경로를 선택한다면 `thread/mmu.c`와 `include/threads/vaddr.h`의 function을 살펴보세요. 이것이 user memory access를 처리하는 가장 단순한 방법입니다.

두 번째 방법은 user pointer가 `KERN_BASE` 아래를 가리키는지만 check(확인)한 뒤 dereference하는 것입니다. invalid user pointer는 "page fault"를 일으키고, 이를 `userprog/exception.c`의 `page_fault()` code를 수정하여 처리할 수 있습니다. 이 technique(기법)은 processor의 MMU(memory management unit, 메모리 관리 장치)를 활용하므로 일반적으로 더 빠르며, 실제 kernel(Linux 포함)에서 사용되는 경향이 있습니다.

어느 경우든 resource를 "leak(누수)"하지 않도록 해야 합니다. 예를 들어 system call이 lock(잠금)을 acquire(획득)했거나 `malloc()`으로 memory를 allocate(할당)했다고 합시다. 그 뒤 invalid user pointer를 만나더라도 lock을 release(해제)하거나 memory page를 free해야 합니다. user pointer를 dereference하기 전에 verify하는 방식을 선택했다면 이는 비교적 straightforward(간단)합니다. invalid pointer가 page fault를 일으키는 방식을 선택하면 처리하기 더 어렵습니다. memory access에서 error code(오류 코드)를 return할 방법이 없기 때문입니다. 따라서 후자의 technique을 시도하려는 사람들을 위해 약간의 도움 되는 code를 제공합니다.
    
    
    /* Reads a byte at user virtual address UADDR.
     * UADDR must be below KERN_BASE.
     * Returns the byte value if successful, -1 if a segfault
     * occurred. */
    static int64_t
    get_user (const uint8_t *uaddr) {
        int64_t result;
        __asm __volatile (
        "movabsq $done_get, %0\n"
        "movzbq %1, %0\n"
        "done_get:\n"
        : "=&a" (result) : "m" (*uaddr));
        return result;
    }
    
    /* Writes BYTE to user address UDST.
     * UDST must be below KERN_BASE.
     * Returns true if successful, false if a segfault occurred. */
    static bool
    put_user (uint8_t *udst, uint8_t byte) {
        int64_t error_code;
        __asm __volatile (
        "movabsq $done_put, %0\n"
        "movb %b2, %1\n"
        "done_put:\n"
        : "=&a" (error_code), "=m" (*udst) : "q" (byte));
        return error_code != -1;
    }
    

각 function은 user address가 이미 `KERN_BASE` 아래임이 verify되었다고 가정합니다. 또한 kernel 안에서 page fault가 발생하면 `page_fault()`가 단순히 rax를 -1로 설정하고 이전 값을 `%rip`에 copy하도록 수정되어 있다고 가정합니다.
