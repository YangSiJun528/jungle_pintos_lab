# Project3: Virtual Memory (가상 메모리)

이제 Pintos의 내부 동작에 어느 정도 익숙해졌을 것입니다. 여러분의 OS(운영체제)는 적절한 synchronization(동기화)으로 여러 thread of execution(실행 흐름)을 처리할 수 있고, 여러 user program(사용자 프로그램)을 동시에 load(적재)할 수 있습니다. 하지만 실행할 수 있는 program의 수와 크기는 machine(컴퓨터)의 main memory(주 메모리) 크기에 제한됩니다. 이 assignment(과제)에서는 infinite memory(무한한 메모리)의 illusion(착각)을 만들어 그 제한을 제거합니다.

이 assignment는 이전 project 위에 build(빌드)합니다. project 2의 test program도 project 3에서 동작해야 합니다. project 3 작업을 시작하기 전에 project 2 제출물의 bug(결함)를 고치도록 주의하세요. 그 bug들은 project 3에서도 같은 문제를 일으킬 가능성이 높습니다.

project 3에서는 여러분의 편의를 위해 step-by-step direction(단계별 지시)을 제공합니다.

## Background (배경)

### Source Files (소스 파일)

이 project에서는 `vm` directory(디렉터리)에서 작업합니다. `Makefile`은 `-DVM` setting(설정)을 켜도록 update(갱신)되어 있습니다. 많은 양의 template code(템플릿 코드)를 제공합니다. ***반드시 제공된 template을 따라야 합니다. 즉, 제공된 template을 기반으로 하지 않은 code를 제출하면 0점을 받습니다.*** 또한 "DO NOT CHANGE"라고 표시된 template은 절대 변경해서는 안 됩니다. 여기서는 수정하게 될 각 template file(파일)에 대한 detail(세부 사항)을 제공합니다.

  - `include/vm/vm.h`, `vm/vm.c`

virtual memory(가상 메모리)에 대한 general interface(일반 인터페이스)를 제공합니다. header file(헤더 파일)에서 virtual memory system(가상 메모리 시스템)이 지원해야 하는 여러 `vm_type` -- VM_UNINIT, VM_ANON, VM_FILE, VM_PAGE_CACHE -- 의 definition(정의)과 explanation(설명)을 볼 수 있습니다. 지금은 VM_PAGE_CACHE는 무시하세요. 이는 project 4용입니다. 또한 여기에서 supplemental page table(보조 페이지 테이블)을 구현합니다(아래 참조).

  - `include/vm/uninit.h`, `vm/uninit.c`

uninitialized page(초기화되지 않은 페이지, vm_type = `VM_UNINIT`)에 대한 operation(연산)을 제공합니다. 현재 design(설계)에서는 모든 page가 처음에는 uninitialized page로 설정되고, 이후 anonymous page(익명 페이지)나 file-backed page(파일 기반 페이지)로 변환됩니다.

  - `include/vm/anon.h`, `vm/anon.c`

anonymous page(vm_type = `VM_ANON`)에 대한 operation을 제공합니다.

  - `include/vm/file.h`, `vm/file.c`

file-backed page(vm_type = `VM_FILE`)에 대한 operation을 제공합니다.

  - `include/vm/inspect.h`, `vm/inspect.c`

grading(채점)을 위한 memory inspection operation(메모리 검사 연산)을 포함합니다. 이 file들은 변경하지 마세요.

이 project에서 작성하는 code의 대부분은 `vm` directory의 file과 이전 project에서 소개된 file에 들어갑니다. 처음 마주칠 가능성이 있는 file은 몇 개뿐입니다.

  - `include/devices/block.h`, `devices/block.c`

block device(블록 장치)에 대한 sector-based read/write access(섹터 단위 읽기/쓰기 접근)를 제공합니다. 이 interface를 사용하여 swap partition(스왑 파티션)에 block device로 접근합니다.

### Memory Terminology (메모리 용어)

memory와 storage(저장소)에 대한 몇 가지 terminology(용어)를 먼저 제시합니다. 일부 용어는 project 2에서 익숙할 것입니다([Virtual Memory Layout](../2_project2/0_introduction.md) 참조). 하지만 많은 부분은 새롭습니다.

#### Pages (페이지)

page(페이지), 때로 virtual page(가상 페이지)라고 부르는 것은 크기가 4,096 bytes(바이트), 즉 ***page size(페이지 크기)***인 연속적인 virtual memory region(가상 메모리 영역)입니다. page는 반드시 ***page-aligned(페이지 정렬)***되어야 합니다. 즉 page size로 나누어떨어지는 virtual address(가상 주소)에서 시작해야 합니다. 따라서 64-bit virtual address의 마지막 12 bit(비트)는 ***page offset(페이지 오프셋)***, 또는 단순히 ***offset***입니다. 위쪽 bit들은 page table(페이지 테이블)의 index(인덱스)를 나타내는 데 사용되며, 곧 소개합니다. 64-bit system에서는 4-level page table(4단계 페이지 테이블)을 사용하므로 virtual address는 다음처럼 보입니다.
    
    
    63          48 47            39 38            30 29            21 20         12 11         0
    +-------------+----------------+----------------+----------------+-------------+------------+
    | Sign Extend |    Page-Map    | Page-Directory | Page-directory |  Page-Table |    Page    |
    |             | Level-4 Offset |    Pointer     |     Offset     |   Offset    |   Offset   |
    +-------------+----------------+----------------+----------------+-------------+------------+
                  |                |                |                |             |            |
                  +------- 9 ------+------- 9 ------+------- 9 ------+----- 9 -----+---- 12 ----+
                                              Virtual Address
    

각 process(프로세스)는 독립적인 user virtual page(사용자 가상 페이지) 집합을 가지며, 이는 virtual address `KERN_BASE`(0x8004000000) 아래의 page들입니다. 반면 ***kernel virtual page(커널 가상 페이지)*** 집합은 global(전역)이며, 어떤 thread나 process가 실행 중인지와 관계없이 항상 같은 위치에 남아 있습니다. kernel(운영체제 핵심부)은 user page와 kernel page 모두에 접근할 수 있지만, user process는 자신의 user page에만 접근할 수 있습니다. 자세한 내용은 [Virtual Memory Layout](../2_project2/0_introduction.md)을 참조하세요.

Pintos는 virtual address를 다루기 위한 여러 유용한 function(함수)을 제공합니다. 자세한 내용은 [Virtual Addresses](../5_appendix/3_virtual_address.md) section을 참조하세요.

#### Frames (프레임)

***frame(프레임)***, 때로 ***physical frame(물리 프레임)*** 또는 ***page frame(페이지 프레임)***이라고 부르는 것은 physical memory(물리 메모리)의 연속적인 region입니다. page와 마찬가지로 frame도 page-size이고 page-aligned여야 합니다. 따라서 64-bit physical address(물리 주소)는 다음처럼 ***frame number(프레임 번호)***와 frame ***offset***으로 나눌 수 있습니다.
    
    
                              12 11         0
        +-----------------------+-----------+
        |      Frame Number     |   Offset  |
        +-----------------------+-----------+
                  Physical Address
    

x86-64는 physical address의 memory에 직접 접근할 방법을 제공하지 않습니다. Pintos는 kernel virtual memory(커널 가상 메모리)를 physical memory에 직접 mapping(매핑)하여 이를 우회합니다. kernel virtual memory의 첫 page는 physical memory의 첫 frame에, 두 번째 page는 두 번째 frame에 mapping되는 식입니다. 따라서 frame은 kernel virtual memory를 통해 접근할 수 있습니다.

Pintos는 physical address와 kernel virtual address(커널 가상 주소) 사이를 translate(변환)하는 function을 제공합니다. 자세한 내용은 [Virtual Addresses](../5_appendix/3_virtual_address.md)를 참조하세요.

#### Page Tables (페이지 테이블)

***page table***은 CPU가 virtual address를 physical address로 translate, 즉 page를 frame으로 translate하는 데 사용하는 data structure(자료구조)입니다. page table format(형식)은 x86-64 architecture가 정합니다. Pintos는 `threads/mmu.c`에 page table management code(페이지 테이블 관리 코드)를 제공합니다.

아래 diagram(그림)은 page와 frame의 관계를 보여 줍니다. 왼쪽의 virtual address는 page number(페이지 번호)와 offset으로 구성됩니다. page table은 page number를 frame number로 translate하고, 변경되지 않은 offset과 결합하여 오른쪽의 physical address를 얻습니다.
    
    
                              +----------+
             .--------------->|Page Table|-----------.
            /                 +----------+            |
            |   12 11 0                               V  12 11 0
        +---------+----+                         +---------+----+
        | Page Nr | Ofs|                         |Frame Nr | Ofs|
        +---------+----+                         +---------+----+
         Virt Addr   |                            Phys Addr    ^
                      \_______________________________________/
    

#### Swap Slots (스왑 슬롯)

***swap slot(스왑 슬롯)***은 swap partition 안의 page-size disk space(디스크 공간)입니다. slot의 배치를 제한하는 hardware limitation(하드웨어 제한)은 frame보다 더 유연하지만, swap slot을 page-aligned로 두는 데 단점이 없으므로 page-aligned여야 합니다.

### Resource Management Overview (자원 관리 개요)

다음 data structure를 design/implement(설계/구현)해야 합니다.

***Supplemental page table***

> page table을 보완하여 page fault handling(페이지 폴트 처리)을 가능하게 합니다. 아래 Managing the Supplemental Page Table을 참조하세요.

***Frame table***

> physical frame(물리 프레임)의 eviction policy(축출 정책)를 효율적으로 구현할 수 있게 합니다. 아래 Managing the Frame Table을 참조하세요.

***Swap table***

> swap slot 사용량을 추적합니다. 아래 Managing the Swap Table을 참조하세요.

세 data structure를 반드시 완전히 독립적으로 구현할 필요는 없습니다. 관련 resource를 하나의 unified data structure(통합 자료구조)로 전체 또는 일부 merge(병합)하는 것이 편리할 수 있습니다.

각 data structure에 대해 각 element(원소)가 어떤 information(정보)을 포함해야 하는지 결정해야 합니다. 또한 data structure의 scope(범위)가 local(per-process, 프로세스별)인지 global(system 전체에 적용)인지, 그리고 그 scope 안에서 instance(인스턴스)가 몇 개 필요한지도 결정해야 합니다.

design을 단순화하기 위해 이 data structure들을 non-pageable memory(페이지 아웃되지 않는 메모리)에 저장해도 됩니다. 예를 들어 `calloc`이나 `malloc`으로 allocate(할당)한 memory입니다. 이는 그들 사이의 pointer(포인터)가 계속 valid(유효)하게 남는다고 확신할 수 있다는 뜻입니다.

#### Choices of implementation (Performance perspective) (구현 선택지: 성능 관점)

가능한 implementation choice(구현 선택지)에는 array(배열), list(리스트), bitmap(비트맵), hash table(해시 테이블)이 있습니다. array는 종종 가장 단순한 approach(접근)지만, sparse(드문드문 채워진) array는 memory를 낭비합니다. list도 단순하지만, 특정 position(위치)을 찾기 위해 긴 list를 traverse(순회)하는 것은 time(시간)을 낭비합니다. array와 list는 모두 resize(크기 조정)될 수 있지만, list는 중간 insertion/deletion(삽입/삭제)을 더 효율적으로 지원합니다.

Pintos는 `lib/kernel/bitmap.c`와 `include/lib/kernel/bitmap.h`에 bitmap data structure를 포함합니다. bitmap은 bit의 array이며 각 bit는 true 또는 false가 될 수 있습니다. bitmap은 보통 동일한 resource 집합의 usage(사용 상태)를 추적하는 데 사용됩니다. resource n이 사용 중이면 bitmap의 bit ***n***이 true입니다. Pintos bitmap은 fixed size(고정 크기)이지만, resizing을 지원하도록 구현을 확장할 수 있습니다.

Pintos는 hash table data structure도 포함합니다([Hash Table](../5_appendix/7_hash_table.md) 참조). Pintos hash table은 넓은 table size 범위에서 insertion과 deletion을 효율적으로 지원합니다.

더 complex(복잡한) data structure가 더 나은 performance나 다른 benefit(이점)을 줄 수도 있지만, implementation을 불필요하게 복잡하게 만들 수도 있습니다. 따라서 design의 일부로 advanced data structure(예: balanced binary tree, 균형 이진 트리)를 구현하는 것은 권장하지 않습니다.

### Managing the Supplemental Page Table (보조 페이지 테이블 관리)

***supplemental*** page table은 각 page에 대한 additional data(추가 데이터)로 page table을 보완합니다. page table format이 impose(부과)하는 limitation 때문에 필요합니다. 이런 data structure도 종종 "page table"이라고 부릅니다. 혼동을 줄이기 위해 "supplemental"이라는 단어를 붙입니다.

supplemental page table은 최소 두 가지 목적으로 사용됩니다. 가장 중요하게는 page fault가 발생했을 때, kernel이 fault(실패)를 일으킨 virtual page를 supplemental page table에서 lookup(조회)하여 그 page에 어떤 data가 있어야 하는지 알아냅니다. 둘째, process가 terminate될 때 kernel은 어떤 resource를 free할지 결정하기 위해 supplemental page table을 consult(참조)합니다.

#### Organization of Supplemental Page Table (보조 페이지 테이블 구성)

supplemental page table은 원하는 방식으로 organize(구성)할 수 있습니다. 구성에는 적어도 두 가지 기본 approach가 있습니다. segment(세그먼트) 단위 또는 page 단위입니다. 여기서 segment는 연속된 page group(페이지 그룹), 즉 executable(실행 파일)이나 memory-mapped file(메모리 매핑 파일)을 포함하는 memory region을 의미합니다.

선택적으로 page table 자체를 사용하여 supplemental page table의 member(구성원)를 추적할 수 있습니다. 이렇게 하려면 `threads/mmu.c`의 Pintos page table implementation을 수정해야 합니다. 이 접근은 advanced student(고급 학생)에게만 권장합니다.

#### Handling page fault (page fault 처리)

supplemental page table의 가장 중요한 user(사용자)는 page fault handler(페이지 폴트 처리기)입니다. project 2에서 page fault는 항상 kernel이나 user program의 bug를 나타냈습니다. project 3에서는 더 이상 그렇지 않습니다. 이제 page fault는 단지 page를 file이나 swap slot에서 가져와야 함을 의미할 수 있습니다. 이런 경우를 처리하려면 더 sophisticated(정교한) page fault handler를 구현해야 합니다. `userprog/exception.c`의 `page_fault()`인 page fault handler는 `vm/vm.c`의 여러분의 page fault handler `vm_try_handle_fault()`를 호출합니다. 여러분의 page fault handler는 대략 다음을 수행해야 합니다.

  1. supplemental page table에서 faulted page(폴트가 난 페이지)를 찾습니다. memory reference(메모리 참조)가 valid하면 supplemental page table entry(항목)를 사용하여 page에 들어갈 data의 위치를 찾습니다. data는 file system(파일 시스템), swap slot에 있거나, 단순히 all-zero page(모두 0인 페이지)일 수 있습니다. sharing(공유), 즉 Copy-on-Write(쓰기 시 복사)를 구현했다면 page의 data가 page table에는 없지만 이미 page frame에 있을 수도 있습니다. supplemental page table이 user process가 접근하려던 address에 어떤 data도 기대해서는 안 된다고 나타내거나, page가 kernel virtual memory 안에 있거나, read-only page(읽기 전용 페이지)에 write하려는 access라면 그 access는 invalid입니다. invalid access는 process를 terminate하고 모든 resource를 free합니다.

  2. page를 저장할 frame을 얻습니다. sharing을 구현했다면 필요한 data가 이미 frame에 있을 수 있으며, 이 경우 그 frame을 locate(찾기)할 수 있어야 합니다.

  3. file system이나 swap에서 읽거나 zeroing(0으로 채움)하는 등으로 data를 frame으로 fetch(가져오기)합니다. sharing을 구현했다면 필요한 page가 이미 frame에 있을 수 있으며, 이 경우 이 단계에서 아무 작업도 필요 없습니다.

  4. faulting virtual address(폴트를 일으킨 가상 주소)의 page table entry가 physical page(물리 페이지)를 가리키게 합니다. `threads/mmu.c`의 function들을 사용할 수 있습니다.

### Managing the Frame Table (프레임 테이블 관리)

frame table(프레임 테이블)은 각 frame마다 하나의 entry를 포함합니다. frame table의 각 entry는 현재 그 frame을 차지하는 page가 있다면 그 page를 가리키는 pointer와, 여러분이 선택한 다른 data를 포함합니다. frame table은 free frame(빈 프레임)이 없을 때 evict(축출)할 page를 선택하여 Pintos가 eviction policy를 효율적으로 구현할 수 있게 합니다.

user page에 사용하는 frame은 `palloc_get_page(PAL_USER)`를 호출하여 "user pool(사용자 풀)"에서 얻어야 합니다. `PAL_USER`를 사용해야 "kernel pool(커널 풀)"에서 allocate하는 것을 피할 수 있으며, 그렇지 않으면 일부 test case가 예기치 않게 fail할 수 있습니다. frame table implementation의 일부로 `palloc.c`를 수정한다면 두 pool의 구분을 유지해야 합니다.

frame table에서 가장 중요한 operation은 unused frame(사용되지 않는 프레임)을 얻는 것입니다. free frame이 있으면 쉽습니다. free frame이 없으면 어떤 page를 그 frame에서 evict하여 frame을 free해야 합니다.

swap slot을 allocate하지 않고 evict할 수 있는 frame이 없고 swap이 full(가득 참)이라면 kernel panic(커널 패닉)을 일으키세요. 실제 OS는 이런 상황에서 recover(복구)하거나 prevent(예방)하기 위한 다양한 policy를 적용하지만, 이 project의 scope(범위)를 벗어납니다.

eviction process(축출 과정)는 대략 다음 단계로 구성됩니다.

  1. page replacement algorithm(페이지 교체 알고리즘)을 사용하여 evict할 frame을 선택합니다. 아래 설명하는 page table의 "accessed" bit와 "dirty" bit가 유용할 것입니다.

  2. 그 frame을 refer(참조)하는 모든 page table에서 frame에 대한 reference(참조)를 제거합니다. sharing을 구현하지 않았다면 어떤 시점에도 하나의 page만 frame을 refer해야 합니다.

  3. 필요한 경우 page를 file system이나 swap에 write(쓰기)합니다. 그러면 evicted frame(축출된 프레임)은 다른 page를 저장하는 데 사용할 수 있습니다.

#### Accessed and Dirty Bits (accessed/dirty bit)

x86-64 hardware는 각 page의 page table entry(PTE, 페이지 테이블 항목)에 있는 bit 쌍을 통해 page replacement algorithm 구현을 일부 지원합니다. page에 대한 read(읽기)나 write가 발생하면 CPU는 그 page의 PTE에서 accessed bit(접근 비트)를 1로 설정하고, write가 발생하면 ***dirty bit(더티 비트)***를 1로 설정합니다. CPU는 이 bit들을 절대 0으로 reset(초기화)하지 않지만, OS는 그렇게 할 수 있습니다.

`aliases`를 알아야 합니다. 이는 같은 frame을 refer하는 두 개 이상의 page를 의미합니다. aliased frame(별칭이 있는 프레임)이 access되면, accessed bit와 dirty bit는 하나의 page table entry, 즉 access에 사용된 page의 entry에서만 update됩니다. 다른 alias의 accessed/dirty bit는 update되지 않습니다.

Pintos에서 모든 user virtual page는 그 kernel virtual page와 alias됩니다. 이 alias를 어떤 방식으로든 manage(관리)해야 합니다. 예를 들어 code가 두 address 모두에 대해 accessed/dirty bit를 check하고 update할 수 있습니다. 또는 kernel이 user data에 user virtual address를 통해서만 접근하여 문제를 피할 수도 있습니다.

다른 alias는 sharing을 구현했거나 code에 bug가 있을 때만 발생해야 합니다.

accessed/dirty bit를 다루는 function의 자세한 내용은 [Page Table Accessed and Dirty Bits](../5_appendix/4_page_table.md) section을 참조하세요.

### Managing the Swap Table (스왑 테이블 관리)

swap table(스왑 테이블)은 사용 중인 swap slot과 free swap slot을 추적합니다. frame에서 swap partition으로 page를 evict할 때 unused swap slot(사용되지 않는 스왑 슬롯)을 선택할 수 있어야 합니다. page가 다시 읽혀 오거나 그 page가 swap된 process가 terminate될 때 swap slot을 free할 수 있어야 합니다.

`vm/build` directory에서 `pintos-mkdisk swap.dsk --swap-size=n` command를 사용하여 n-MB swap partition을 포함하는 `swap.dsk`라는 disk를 만드세요. 이후 pintos를 실행하면 `swap.dsk`가 자동으로 extra disk(추가 디스크)로 attach(연결)됩니다. 또는 `\--swap-size=n`으로 한 번의 실행에만 사용할 temporary n-MB swap disk를 pintos에 지정할 수 있습니다.

swap slot은 lazily(게으르게), 즉 eviction으로 실제로 필요할 때만 allocate되어야 합니다. process startup(프로세스 시작) 시 executable에서 data page를 읽고 곧바로 swap에 write하는 것은 lazy가 아닙니다. 특정 page를 저장하기 위해 swap slot을 reserve(예약)해서는 안 됩니다.

swap slot의 contents(내용)가 frame으로 다시 읽혀 오면 그 swap slot을 free하세요.

### Managing Memory Mapped Files (메모리 매핑 파일 관리)

file system은 주로 `read`와 `write` system call(시스템 콜)을 통해 접근합니다. secondary interface(보조 인터페이스)는 `mmap` system call을 사용하여 file을 virtual page에 "map(매핑)"하는 것입니다. 그러면 program은 file data에 대해 memory instruction(메모리 명령)을 직접 사용할 수 있습니다. file `foo`가 길이 `0x1000` bytes(4 kB, 즉 한 page)라고 합시다. `foo`가 address `0x5000`에서 시작하도록 memory에 mapped되면, `0x5000...0x5fff` 위치에 대한 memory access는 `foo`의 대응되는 byte에 접근합니다.

다음은 `mmap`을 사용해 file을 console(콘솔)에 print(출력)하는 program입니다. command line(명령줄)에 지정된 file을 open하고, virtual address `0x10000000`에 map한 뒤, mapped data를 console(fd 1)에 write하고, file을 unmap(매핑 해제)합니다.
    
    
    #include <stdio.h>
    #include <syscall.h>
    int main (int argc UNUSED, char *argv[])
    {
      void *data = (void *) 0x10000000;                 /* Address at which to map. */
      int fd = open (argv[1]);                          /* Open file. */
      void *map = mmap (data, filesize (fd), 0, fd, 0); /* Map file. */
      write (1, data, filesize (fd));                   /* Write file to console. */
      munmap (map);                                     /* Unmap file (optional). */
      return 0;
    }
    

제출물은 memory mapped file이 사용하는 memory를 추적할 수 있어야 합니다. 이는 mapped region(매핑된 영역)의 page fault를 올바르게 처리하고, mapped file이 process 안의 다른 segment와 overlap(겹침)되지 않도록 보장하는 데 필요합니다.
