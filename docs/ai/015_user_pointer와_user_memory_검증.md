# user pointer와 user memory 검증

구현/테스트 실행 없이 `docs/reference`만 기준으로 정리한다. 기준 자료는 `pintos-kaist-original`이며, 아래 인용은 같은 내용의 `pintos-kaist-kr` 번역문을 중심으로 적는다.

## 1. 유저 포인터의 값을 커널에서 작업하려면 복사해야 하는가?

문서가 요구하는 것은 "무조건 커널 버퍼로 복사"가 아니라, **user virtual address space의 데이터를 안전하게 읽고/쓸 방법을 제공하는 것**이다.

> syscall(시스템 콜)을 구현하려면 user virtual address space(사용자 가상 주소 공간)의 data(데이터)를 read(읽기)하고 write(쓰기)할 방법을 제공해야 합니다. argument(인자)를 가져올 때에는 이 능력이 필요하지 않습니다. 하지만 system call의 argument로 제공된 pointer(포인터)에서 data를 읽을 때에는 이 functionality(기능)를 통해 중계해야 합니다.
>
> `docs/reference/pintos-kaist-kr/2_project2/2_user_memory.md`

따라서 구분하면 다음과 같다.

- 시스템 콜 인자로 넘어온 **포인터 값 자체**, 즉 주소값을 레지스터에서 얻는 것은 user memory read/write 기능이 필요 없다고 문서가 말한다.
- 그 포인터가 가리키는 **실제 데이터**를 읽거나 쓸 때는 user memory access 기능을 거쳐야 한다.
- 문서는 두 방식을 제시한다. 하나는 "포인터 유효성 검증 후 역참조", 다른 하나는 "`KERN_BASE` 아래인지 확인 후 역참조하고 page fault 처리"이다. 즉 "복사"가 유일한 요구사항으로 적혀 있지는 않다.

관련 근거:

- `docs/reference/pintos-kaist-kr/2_project2/0_introduction.md`의 Accessing User Memory 절
- `docs/reference/pintos-kaist-kr/2_project2/2_user_memory.md`

## 2. 시스템 콜 포인터가 user memory가 아닌 경우를 어떻게 판단하는가?

문서 기준으로 invalid pointer는 최소 세 종류이다.

> user는 null pointer(널 포인터), unmapped virtual memory를 가리키는 pointer, 또는 kernel virtual address space(`KERN_BASE` 위)를 가리키는 pointer를 전달할 수 있기 때문입니다. 이런 모든 invalid pointer(잘못된 포인터)는 kernel이나 다른 실행 중인 process에 피해를 주지 않고 거부되어야 하며, 문제를 일으킨 process를 terminate하고 resource(자원)를 free(해제)해야 합니다.
>
> `docs/reference/pintos-kaist-kr/2_project2/0_introduction.md`

판단 기준은 두 층이다.

1. **주소 범위 판단**

   user virtual memory는 `0`부터 `KERN_BASE`까지이고, `KERN_BASE` 이상은 kernel virtual memory이다. `KERN_BASE` 기본값은 `0x8004000000`이다.

   > Pintos의 virtual memory(가상 메모리)는 user virtual memory(사용자 가상 메모리)와 kernel virtual memory(커널 가상 메모리)라는 두 region(영역)으로 나뉩니다. user virtual memory는 virtual address(가상 주소) `0`부터 `KERN_BASE`까지입니다. `KERN_BASE`는 `include/threads/vaddr.h`에 정의되어 있으며 기본값은 `0x8004000000`입니다.
   >
   > `docs/reference/pintos-kaist-kr/2_project2/0_introduction.md`

   Appendix도 같은 경계를 설명한다.

   > kernel virtual memory의 base address(기준 주소)입니다. 기본값은 0x8004000000입니다. user virtual memory는 virtual address 0부터 `KERN_BASE`까지이고, kernel virtual memory는 virtual address space(가상 주소 공간)의 나머지를 차지합니다.
   >
   > `docs/reference/pintos-kaist-kr/5_appendix/3_virtual_address.md`

2. **매핑 여부 판단**

   `KERN_BASE` 아래라고 항상 접근 가능한 것은 아니다. 문서는 kernel 안에서도 unmapped user virtual address에 접근하면 page fault가 난다고 말한다.

   > 하지만 kernel 안에서도 unmapped user virtual address(매핑되지 않은 사용자 가상 주소)의 memory에 접근하려고 하면 page fault가 발생합니다.
   >
   > `docs/reference/pintos-kaist-kr/2_project2/0_introduction.md`

   보조 함수 기준도 문서에 있다.

   > pml4에서 uaddr에 mapping된 frame을 look up(조회)합니다. uaddr이 mapping되어 있으면 그 frame의 kernel virtual address를 반환하고, 그렇지 않으면 null pointer를 반환합니다.
   >
   > `docs/reference/pintos-kaist-kr/5_appendix/4_page_table.md`

정리하면, `KERN_BASE` 이상이면 user memory가 아니고, `KERN_BASE` 미만이어도 null/unmapped이면 invalid이다. 또한 버퍼가 일부라도 invalid pointer, kernel memory, 또는 그런 region에 걸쳐 있으면 user process를 종료해야 한다.

> user가 invalid pointer(잘못된 포인터), kernel memory(커널 메모리)를 가리키는 pointer, 또는 일부가 그런 region(영역)에 걸쳐 있는 block(블록)을 제공하면 어떻게 될까요? 이런 경우에는 user process(사용자 프로세스)를 terminate(종료)하여 처리해야 합니다.
>
> `docs/reference/pintos-kaist-kr/2_project2/2_user_memory.md`

## 3. 사용자가 가능한 영역에는 어떤 종류가 있는가? VM 매핑을 고려해야 하는가?

기본 범위는 **각 프로세스 자신의 user virtual memory**이다.

> user program은 자신의 user virtual memory에만 접근할 수 있습니다.
>
> `docs/reference/pintos-kaist-kr/2_project2/0_introduction.md`

일반적인 배치는 문서상 다음이다.

- code segment: `0x400000`부터 시작
- initialized data segment
- uninitialized data segment, BSS
- user stack
- Project 2에서는 stack 크기 고정, Project 3에서는 grow 가능

관련 근거:

- `docs/reference/pintos-kaist-kr/2_project2/0_introduction.md`의 Typical Memory Layout 절

VM까지 포함하면 반드시 매핑 상태를 고려해야 한다. Project 3에서는 page fault가 항상 버그가 아니고, lazy-loaded page, swapped-out page, write-protected page일 수 있다.

> project 2에서 page fault는 항상 kernel이나 user program의 bug를 나타냈습니다. project 3에서는 더 이상 그렇지 않습니다. 이제 page fault는 단지 page를 file이나 swap slot에서 가져와야 함을 의미할 수 있습니다.
>
> `docs/reference/pintos-kaist-kr/3_project3/0_introduction.md`

Project 3의 page fault 처리 기준은 supplemental page table을 확인하는 것이다.

> supplemental page table은 최소 두 가지 목적으로 사용됩니다. 가장 중요하게는 page fault가 발생했을 때, kernel이 fault(실패)를 일으킨 virtual page를 supplemental page table에서 lookup(조회)하여 그 page에 어떤 data가 있어야 하는지 알아냅니다.
>
> `docs/reference/pintos-kaist-kr/3_project3/0_introduction.md`

VM에서 invalid access가 되는 경우도 문서가 직접 나열한다.

> supplemental page table이 user process가 접근하려던 address에 어떤 data도 기대해서는 안 된다고 나타내거나, page가 kernel virtual memory 안에 있거나, read-only page(읽기 전용 페이지)에 write하려는 access라면 그 access는 invalid입니다. invalid access는 process를 terminate하고 모든 resource를 free합니다.
>
> `docs/reference/pintos-kaist-kr/3_project3/0_introduction.md`

또한 VM에서는 executable이나 memory-mapped file을 포함하는 region을 supplemental page table로 추적해야 한다.

> supplemental page table은 원하는 방식으로 organize(구성)할 수 있습니다. 구성에는 적어도 두 가지 기본 approach가 있습니다. segment(세그먼트) 단위 또는 page 단위입니다. 여기서 segment는 연속된 page group(페이지 그룹), 즉 executable(실행 파일)이나 memory-mapped file(메모리 매핑 파일)을 포함하는 memory region을 의미합니다.
>
> `docs/reference/pintos-kaist-kr/3_project3/0_introduction.md`

memory mapped file도 사용자 가상 주소 공간의 매핑된 영역으로 다뤄진다.

> file system은 주로 `read`와 `write` system call(시스템 콜)을 통해 접근합니다. secondary interface(보조 인터페이스)는 `mmap` system call을 사용하여 file을 virtual page에 "map(매핑)"하는 것입니다. 그러면 program은 file data에 대해 memory instruction(메모리 명령)을 직접 사용할 수 있습니다.
>
> `docs/reference/pintos-kaist-kr/3_project3/0_introduction.md`

## 결론

Project 2 syscall 포인터 검증만 보면 `KERN_BASE` 범위와 실제 매핑 여부/page fault 처리가 핵심이다. Project 3 VM까지 포함하면 supplemental page table, lazy loading, swap, mmap region까지 고려해야 한다.

따라서 문서 기반 결론은 다음과 같다.

- 포인터 값 자체를 시스템 콜 인자로 가져오는 것과, 그 포인터가 가리키는 user memory를 읽고 쓰는 것은 구분해야 한다.
- user memory 접근은 검증 후 역참조하거나, `KERN_BASE` 아래 여부를 확인한 뒤 page fault로 처리하는 방식 중 하나를 택할 수 있다고 문서가 설명한다.
- user address range 안에 있어도 매핑되지 않았거나, VM상 해당 주소에 기대되는 데이터가 없거나, read-only page에 write하려는 경우 invalid access이다.
- VM 프로젝트에서는 단순 주소 범위만으로는 부족하고 supplemental page table과 mmap/lazy/swap 상태까지 고려해야 한다.
