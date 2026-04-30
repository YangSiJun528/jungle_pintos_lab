# FAQ

### How much code will I need to write? (얼마나 많은 코드를 작성해야 하나요?)

다음은 `git diff --stat`으로 생성한 reference solution(참고 구현)의 요약입니다. 마지막 행은 insert(추가)와 delete(삭제)된 총 line(줄) 수를 보여 줍니다. 변경된 line은 insertion과 deletion 양쪽으로 계산됩니다. reference solution은 가능한 solution 중 하나일 뿐입니다. 다른 solution도 많이 가능하며, 그중 많은 solution은 reference solution과 크게 다를 수 있습니다. 뛰어난 solution 중에는 reference solution이 수정한 file(파일)을 모두 수정하지 않는 것도 있고, reference solution이 수정하지 않은 file을 수정하는 것도 있을 수 있습니다. 또한 여기에는 extra requirement(추가 요구사항) 구현도 포함되어 있습니다. 참고로 약 150 line은 extra와 관련되어 있습니다.
    
    
    src/include/threads/thread.h   |  23 ++
    src/include/userprog/syscall.h |   3 +
    src/threads/thread.c           |   5 +
    src/userprog/exception.c       |   4 +
    src/userprog/process.c         | 355 +++++++++++++++++++++++++++++++++++++++++-
    src/userprog/syscall.c         | 429 ++++++++++++++++++++++++++++++++++++++++++-
    6 files changed, 782 insertions(+), 37 deletions(-)
    

### The kernel always panics when I run `pintos -p file -- -q`. (`pintos -p file -- -q`를 실행하면 kernel이 항상 panic합니다.)

file system(파일 시스템)을 `pintos -f`로 format(포맷)했나요?

file name(파일 이름)이 너무 길지는 않나요? file system은 file name을 14 character(문자)로 제한합니다.

`pintos -p ../../examples/echo -- -q` 같은 command(명령)는 이 제한을 넘습니다.

대신 `pintos -p ../../examples/echo:echo -- -q`를 사용하여 file을 `echo`라는 이름으로 넣으세요.

file system이 full(가득 참) 상태인가요?

file system에 이미 16개의 file이 있나요? base Pintos file system은 16-file limit(파일 16개 제한)이 있습니다.

file system이 너무 fragmented(단편화)되어 file을 위한 contiguous space(연속 공간)가 부족할 수도 있습니다.

### When I run `pintos -p ../file --`, 'file' isn't copied. (`pintos -p ../file --`를 실행해도 'file'이 복사되지 않습니다.)

기본적으로 file은 여러분이 참조한 이름으로 기록됩니다. 따라서 이 경우 copy된 file의 이름은 '../file'이 됩니다. 아마 대신 `pintos -p ../file:file --`를 실행하고 싶을 것입니다.

### All my user programs die with page faults. (모든 user program이 page fault로 죽습니다.)

argument passing(인자 전달)을 구현하지 않았거나 올바르게 구현하지 않았다면 이런 일이 발생합니다. user program용 basic C library(기본 C 라이브러리)는 stack(스택)에서 argv를 읽으려고 합니다. stack이나 register(레지스터)가 제대로 설정되어 있지 않으면 page fault(페이지 폴트)가 발생합니다.

### All my user programs die with `system call!` (모든 user program이 `system call!`로 죽습니다.)

다른 것을 보기 전에 system call(시스템 콜)을 구현해야 합니다. 합리적인 모든 program은 최소 하나의 system call(`exit()`)을 만들려고 하고, 대부분의 program은 그보다 더 많이 만듭니다. 특히 `printf()`는 write system call을 invoke(호출)합니다. default system call handler(기본 시스템 콜 처리기)는 단순히 `system call!`을 print(출력)하고 program을 terminate(종료)합니다. 그 전까지는 `hex_dump()`를 사용하여 argument passing이 올바르게 구현되었다고 스스로 확인할 수 있습니다.

### How can I disassemble user programs? (user program은 어떻게 disassemble하나요?)

objdump utility(유틸리티)는 전체 user program이나 object file(목적 파일)을 disassemble(역어셈블)할 수 있습니다. objdump -d file 형태로 invoke하세요. 개별 function(함수)을 disassemble하려면 GDB의 disassemble command를 사용할 수 있습니다.

### Why do many C include files not work in Pintos programs? (왜 많은 C include file이 Pintos program에서 동작하지 않나요?)

### Can I use libfoo in my Pintos programs? (Pintos program에서 libfoo를 사용할 수 있나요?)

제공되는 C library(라이브러리)는 매우 제한적입니다. 실제 operating system(운영체제)의 C library에 기대되는 많은 feature(기능)를 포함하지 않습니다. C library는 I/O(입출력)와 memory allocation(메모리 할당)을 위해 system call을 만들어야 하므로, 해당 operating system과 architecture(아키텍처)에 맞게 특별히 build(빌드)되어야 합니다. 물론 모든 function이 system call을 만드는 것은 아니지만, 보통 library는 하나의 unit(단위)으로 compile(컴파일)됩니다.

여러분이 원하는 library는 Pintos가 구현하지 않은 C library 부분을 사용할 가능성이 높습니다. Pintos 아래에서 동작하게 하려면 어느 정도 porting effort(이식 작업)가 필요할 것입니다. 특히 Pintos user program C library에는 `malloc()` implementation(구현)이 없습니다.

### How do I compile new user programs? (새 user program은 어떻게 compile하나요?)

`src/examples/Makefile`을 수정한 뒤 `make`를 실행하세요.

### Can I run user programs under a debugger? (debugger에서 user program을 실행할 수 있나요?)

네, 몇 가지 limitation(제한)이 있습니다. [GDB](7_FAQ.md)를 참조하세요.

### What's the difference between tid_t and pid_t? (tid_t와 pid_t의 차이는 무엇인가요?)

tid_t는 kernel thread(커널 스레드)를 identify(식별)합니다. 그 안에서 user process(사용자 프로세스)가 실행될 수도 있고(`process_fork()`로 생성된 경우), 그렇지 않을 수도 있습니다(`thread_create()`로 생성된 경우). 이는 kernel에서만 사용하는 data type(자료형)입니다. pid_t는 user process를 identify합니다. user process와 kernel이 exec 및 wait system call에서 사용합니다. tid_t와 pid_t에 어떤 suitable type(적절한 타입)을 선택해도 됩니다. 기본적으로 둘 다 int입니다. 둘이 같은 값을 사용해 같은 process를 식별하는 one-to-one mapping(일대일 매핑)으로 만들 수도 있고, 더 complex mapping(복잡한 매핑)을 사용할 수도 있습니다. 선택은 여러분에게 달려 있습니다.

### Can I just cast a `struct file *` to get a file descriptor? (`struct file *`를 cast해서 file descriptor를 얻어도 되나요?)

### Can I just cast a `struct thread *` to a `pid_t`? (`struct thread *`를 `pid_t`로 cast해도 되나요?)

아니요. `pid_t`와 `file descriptor`는 pointer type(포인터 타입)보다 작기 때문에 그대로 cast할 수 없습니다.

### Can I set a maximum number of open files per process? (process별 open file 최대 개수를 정해도 되나요?)

arbitrary limit(임의 제한)을 두지 않는 편이 좋습니다. 필요하다면 process당 128개의 open file 제한을 둘 수 있습니다. 하지만 extra requirement를 구현하려면 제한이 없어야 합니다.

### What happens when an open file is removed? (open file이 remove되면 어떻게 되나요?)

file에 대해 standard Unix semantics(표준 유닉스 의미)를 구현해야 합니다. 즉 file이 remove되면 그 file에 대한 file descriptor를 가진 어떤 process든 해당 descriptor를 계속 사용할 수 있습니다. 이는 그 file에서 read하고 write할 수 있음을 의미합니다. file은 name을 잃고 다른 process는 더 이상 open할 수 없지만, 그 file을 참조하는 모든 file descriptor가 close되거나 machine이 shut down(종료)될 때까지 계속 존재합니다.

### How can I run user programs that need more than 4 kB stack space? (4 kB보다 큰 stack space가 필요한 user program은 어떻게 실행하나요?)

각 process에 대해 하나보다 많은 page(페이지)의 stack space를 allocate(할당)하도록 stack setup code(스택 설정 코드)를 수정할 수 있습니다. 다음 project에서 더 나은 solution을 구현하게 됩니다.
