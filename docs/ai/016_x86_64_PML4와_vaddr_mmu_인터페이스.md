# x86-64 PML4와 vaddr.h, mmu.h 인터페이스

이 문서는 구현을 대신 작성하지 않고, `docs/reference`와 현재 저장소의 header/interface를 기준으로 정리한다. 핵심은 실제 Pintos 코드에서 사용하는 입장에서 `vaddr.h`와 `mmu.h`가 무엇을 제공하는지 이해하는 것이다. 내부 구현 이유를 설명하기 위해 `pte.h`와 `threads/mmu.c`의 구조도 함께 언급한다.

주요 근거:

- `docs/reference/pintos-kaist-kr/5_appendix/3_virtual_address.md`
- `docs/reference/pintos-kaist-kr/5_appendix/4_page_table.md`
- `docs/reference/pintos-kaist-kr/5_appendix/2_memory_allocation.md`
- `docs/reference/pintos-kaist-kr/3_project3/0_introduction.md`
- `pintos/include/threads/vaddr.h`
- `pintos/include/threads/mmu.h`
- `pintos/include/threads/pte.h`

## 1. Intel/x86-64이 제공하는 기본 구조

Pintos reference는 page table을 CPU가 virtual address를 physical address로 translate하는 자료구조라고 설명한다.

> ***page table***은 CPU가 virtual address를 physical address로 translate, 즉 page를 frame으로 translate하는 데 사용하는 data structure(자료구조)입니다. page table format(형식)은 x86-64 architecture가 정합니다.
>
> `docs/reference/pintos-kaist-kr/3_project3/0_introduction.md`

x86-64의 64-bit virtual address는 Pintos 문서에서 다음처럼 나뉜다.

```text
63          48 47            39 38            30 29            21 20         12 11         0
+-------------+----------------+----------------+----------------+-------------+------------+
| Sign Extend |    Page-Map    | Page-Directory | Page-directory |  Page-Table |  Physical  |
|             | Level-4 Offset |    Pointer     |     Offset     |   Offset    |   Offset   |
+-------------+----------------+----------------+----------------+-------------+------------+
              |                |                |                |             |            |
              +------- 9 ------+------- 9 ------+------- 9 ------+----- 9 -----+---- 12 ----+
```

의미는 다음과 같다.

- 하위 12 bit는 page offset이다. Pintos page size는 4,096 bytes이므로 한 page 안의 위치를 나타낸다.
- 그 위쪽 bit들은 9 bit씩 끊겨 page table의 각 level index로 사용된다.
- 4-level page table을 타고 내려가 마지막 PTE가 physical frame을 가리키고, 여기에 offset을 더해 최종 physical address가 된다.

Pintos header인 `pte.h`도 같은 분해를 코드 매크로로 제공한다.

- `PML4(la)`: Page-Map-Level-4 index
- `PDPE(la)`: Page-Directory-Pointer index
- `PDX(la)`: Page-directory index
- `PTX(la)`: Page-table index
- `PTE_ADDR(pte)`: PTE에서 address 부분만 추출

즉 Intel/x86-64이 제공하는 것은 “가상 주소를 여러 index와 offset으로 해석하고, hardware page table을 통해 physical frame으로 translate하는 규칙”이다. Pintos는 이 규칙을 감싼 추상 인터페이스를 `threads/mmu.c`와 `include/threads/mmu.h`로 제공한다.

## 2. PML4가 무엇인가

Pintos reference는 `pml4`를 다음처럼 설명한다.

> Pintos의 page table, 즉 project에서 사용할 page table은 table이 4 level(단계)을 가지기 때문에 Intel processor documentation(문서)에서 Page-Map-Level-4라고 부르는 `pml4`입니다.
>
> `docs/reference/pintos-kaist-kr/5_appendix/4_page_table.md`

사용자 입장에서 `pml4`는 “이 프로세스의 user virtual address가 어느 physical frame에 매핑되는지 알려주는 최상위 page table”이라고 보면 된다.

중요한 점:

- `pml4`는 `uint64_t *`로 표현된다.
- CPU는 현재 활성화된 page table을 사용해 memory reference를 translate한다.
- process마다 user virtual mapping은 다를 수 있다.
- kernel virtual mapping은 global하게 유지된다.

reference는 `pml4_activate()`에 대해 이렇게 설명한다.

> pml4를 activate합니다. active page table(활성 페이지 테이블)은 CPU가 memory reference(메모리 참조)를 translate(변환)하는 데 사용하는 page table입니다.
>
> `docs/reference/pintos-kaist-kr/5_appendix/4_page_table.md`

현재 코드의 `threads/mmu.c`에서는 `pml4_activate()`가 결국 CPU의 page directory base register 역할을 하는 `cr3`에 해당 page table의 physical address를 적재한다. 즉 `pml4_activate()`는 “이제 이 주소 공간으로 주소 변환을 하라”고 CPU에 알려주는 인터페이스이다.

## 3. vaddr.h: virtual address를 다루는 사용 인터페이스

`vaddr.h`는 page table을 직접 조작하기 전 단계에서 virtual address 자체를 다루는 helper를 제공한다. 실제 Pintos 구현에서 user pointer 검증, page 정렬, offset 추출, kernel/user 영역 판단에 자주 쓰인다.

### page 크기와 offset

`vaddr.h`의 핵심 매크로:

- `PGSHIFT`: page offset의 첫 bit index
- `PGBITS`: page offset bit 수, 12
- `PGSIZE`: page 크기, `1 << PGBITS`, 즉 4096
- `PGMASK`: page offset bit mask
- `pg_ofs(va)`: virtual address 안의 page offset 추출
- `pg_no(va)`: virtual page number 추출
- `pg_round_down(va)`: 해당 주소가 속한 page의 시작 주소
- `pg_round_up(va)`: 가장 가까운 page boundary로 올림

reference도 같은 내용을 설명한다.

> bytes(바이트) 단위의 page size(페이지 크기, 4,096)입니다.
>
> `docs/reference/pintos-kaist-kr/5_appendix/3_virtual_address.md`

> va가 가리키는 virtual page(가상 페이지)의 시작 주소, 즉 page offset이 0으로 설정된 va를 반환합니다.
>
> `docs/reference/pintos-kaist-kr/5_appendix/3_virtual_address.md`

실사용 관점:

- page 단위 매핑을 만들 때는 보통 주소가 page-aligned인지 확인해야 한다.
- `pml4_set_page()`는 user page와 kernel page 모두 offset이 0인 page boundary 주소를 기대한다.
- 임의의 user pointer가 어느 page에 속하는지 확인하려면 `pg_round_down()`이 필요하다.
- page 내부 byte 위치가 필요하면 `pg_ofs()`를 쓴다.

### user/kernel virtual address 구분

`vaddr.h`의 핵심 매크로:

- `KERN_BASE`: kernel virtual memory 시작 주소
- `is_user_vaddr(vaddr)`: user virtual address이면 true
- `is_kernel_vaddr(vaddr)`: kernel virtual address이면 true
- `USER_STACK`: user stack 시작 지점

reference는 경계를 다음처럼 설명한다.

> kernel virtual memory의 base address(기준 주소)입니다. 기본값은 0x8004000000입니다. user virtual memory는 virtual address 0부터 `KERN_BASE`까지이고, kernel virtual memory는 virtual address space(가상 주소 공간)의 나머지를 차지합니다.
>
> `docs/reference/pintos-kaist-kr/5_appendix/3_virtual_address.md`

실사용 관점:

- system call로 넘어온 pointer가 kernel 영역인지 빠르게 거를 때 `is_user_vaddr()` 또는 `is_kernel_vaddr()`를 본다.
- 단, `is_user_vaddr()`가 true라는 말은 “주소 범위가 user 쪽”이라는 뜻이지 “실제로 매핑되어 접근 가능하다”는 뜻은 아니다.
- 실제 매핑 여부는 `pml4_get_page()` 같은 page table 조회나 page fault 처리와 연결된다.

### physical address와 kernel virtual address 변환

`vaddr.h`의 변환 매크로:

- `ptov(paddr)`: physical address에 대응하는 kernel virtual address 반환
- `vtop(vaddr)`: kernel virtual address에 대응하는 physical address 반환

reference는 x86-64이 physical address로 memory를 직접 접근하는 방법을 제공하지 않기 때문에 Pintos가 kernel virtual memory를 physical memory에 직접 mapping한다고 설명한다.

> x86-64는 physical address(물리 주소)가 주어졌을 때 memory에 직접 접근하는 방법을 제공하지 않습니다. ... Pintos는 kernel virtual memory를 physical memory에 one-to-one(일대일)으로 mapping(매핑)하여 이를 우회합니다.
>
> `docs/reference/pintos-kaist-kr/5_appendix/3_virtual_address.md`

실사용 관점:

- page table entry에는 physical frame address가 들어간다.
- kernel C code는 physical address를 직접 dereference하지 않고, `ptov()`로 kernel virtual address로 바꿔 접근한다.
- 반대로 page table에 넣을 주소는 `vtop()`으로 physical address 형태로 바꾼다.

## 4. mmu.h: page table을 다루는 사용 인터페이스

`mmu.h`는 x86-64 page table을 직접 만지는 세부 구현을 감추고, Pintos 코드가 page mapping을 조회/추가/삭제/상태 확인할 수 있게 하는 interface이다.

### 생성, 파괴, 활성화

사용 인터페이스:

- `uint64_t *pml4_create(void);`
- `void pml4_destroy(uint64_t *pml4);`
- `void pml4_activate(uint64_t *pml4);`

reference의 설명:

> 새 page table을 create하고 return(반환)합니다. 새 page table은 Pintos의 normal kernel virtual page mapping(일반 커널 가상 페이지 매핑)을 포함하지만 user virtual mapping(사용자 가상 매핑)은 포함하지 않습니다.
>
> `docs/reference/pintos-kaist-kr/5_appendix/4_page_table.md`

> page table 자체와 그것이 mapping하는 frame(프레임)을 포함하여 pml4가 보유한 모든 resource(자원)를 free(해제)합니다.
>
> `docs/reference/pintos-kaist-kr/5_appendix/4_page_table.md`

실사용 관점:

- process 주소 공간을 만들 때 `pml4_create()`로 새 page table을 만든다.
- 이 새 page table에는 kernel mapping은 있지만 user mapping은 없다.
- process가 끝날 때는 `pml4_destroy()`로 page table과 관련 자원을 정리한다.
- context switch나 process 실행 시점에는 `pml4_activate()`로 CPU가 사용할 page table을 바꾼다.

### 매핑 추가

사용 인터페이스:

- `bool pml4_set_page(uint64_t *pml4, void *upage, void *kpage, bool rw);`

reference의 설명:

> user page upage에서 kernel virtual address(커널 가상 주소) kpage로 identify(식별)되는 frame으로의 mapping을 ... 추가합니다. rw가 true이면 page는 read/write(읽기/쓰기)로 mapping되고, 그렇지 않으면 read-only(읽기 전용)로 mapping됩니다.
>
> `docs/reference/pintos-kaist-kr/5_appendix/4_page_table.md`

실사용 관점:

- `upage`는 user virtual page 주소이다.
- `kpage`는 실제 physical frame을 kernel이 접근할 수 있는 kernel virtual address이다.
- `rw`는 user page의 쓰기 가능 여부이다.
- `upage`와 `kpage`는 page-aligned여야 한다.
- `upage`는 이미 해당 `pml4`에 매핑되어 있으면 안 된다.
- Project 3에서 user page용 frame은 `palloc_get_page(PAL_USER)`로 user pool에서 얻는 것이 기준이다.

여기서 “왜 kpage인가?”가 헷갈릴 수 있다. page table entry에는 결국 physical frame address가 들어가야 하지만, kernel code가 받은 frame pointer는 kernel virtual address이다. 내부 구현은 `vtop(kpage)`로 physical address 부분을 page table entry에 넣는다. 이 때문에 API는 `kpage`를 받지만, 의미는 “이 kernel virtual address가 가리키는 frame에 user page를 매핑한다”이다.

### 매핑 조회

사용 인터페이스:

- `void *pml4_get_page(uint64_t *pml4, const void *uaddr);`

reference의 설명:

> pml4에서 uaddr에 mapping된 frame을 look up(조회)합니다. uaddr이 mapping되어 있으면 그 frame의 kernel virtual address를 반환하고, 그렇지 않으면 null pointer를 반환합니다.
>
> `docs/reference/pintos-kaist-kr/5_appendix/4_page_table.md`

실사용 관점:

- user virtual address가 실제로 매핑되어 있는지 확인할 때 쓴다.
- 반환값은 physical address가 아니라 kernel virtual address이다.
- 반환값이 `NULL`이면 해당 user address는 현재 page table에 present mapping이 없다.
- `uaddr`는 page 시작 주소일 필요는 없다. 내부적으로 offset을 보존해 대응되는 kernel virtual address를 돌려준다.

즉 user pointer 검증에서 `is_user_vaddr()`가 범위 검사라면, `pml4_get_page()`는 현재 page table 기준의 매핑 검사이다.

### 매핑 제거 또는 not-present 표시

사용 인터페이스:

- `void pml4_clear_page(uint64_t *pml4, void *upage);`

reference의 설명:

> pml4에서 page를 "not present(존재하지 않음)"로 mark(표시)합니다. 이후 해당 page에 접근하면 fault(폴트)가 발생합니다.
>
> `docs/reference/pintos-kaist-kr/5_appendix/4_page_table.md`

실사용 관점:

- user page를 더 이상 present하지 않게 만든다.
- 이후 그 user page 접근은 page fault를 일으킨다.
- accessed/dirty bit 같은 다른 bit는 보존될 수 있으므로, eviction이나 VM 처리와 연결된다.

### accessed/dirty bit

사용 인터페이스:

- `bool pml4_is_dirty(uint64_t *pml4, const void *upage);`
- `void pml4_set_dirty(uint64_t *pml4, const void *upage, bool dirty);`
- `bool pml4_is_accessed(uint64_t *pml4, const void *upage);`
- `void pml4_set_accessed(uint64_t *pml4, const void *upage, bool accessed);`

reference의 설명:

> x86_64 hardware는 각 page의 page table entry(PTE, 페이지 테이블 항목)에 있는 bit 쌍을 통해 page replacement algorithm(페이지 교체 알고리즘) 구현을 일부 지원합니다. page에 대한 read(읽기)나 write(쓰기)가 발생하면 CPU는 page의 PTE에서 accessed bit(접근 비트)를 1로 설정하고, write가 발생하면 dirty bit(더티 비트)를 1로 설정합니다.
>
> `docs/reference/pintos-kaist-kr/5_appendix/4_page_table.md`

실사용 관점:

- `accessed`: 최근 읽기/쓰기 접근이 있었는지 확인하는 데 사용한다.
- `dirty`: page 내용이 수정되었는지 확인하는 데 사용한다.
- Project 3의 page replacement, swap, mmap write-back 같은 곳에서 중요하다.

주의할 점은 alias이다.

> Pintos에서 모든 user virtual page는 그 kernel virtual page와 alias됩니다. 이 alias를 어떤 방식으로든 manage(관리)해야 합니다.
>
> `docs/reference/pintos-kaist-kr/3_project3/0_introduction.md`

즉 같은 frame을 user virtual address와 kernel virtual address 양쪽으로 접근할 수 있으므로, 어떤 주소로 접근했느냐에 따라 accessed/dirty bit 관찰이 달라질 수 있다.

### PTE flag helper

`mmu.h`에는 PTE 상태를 읽는 macro도 있다.

- `is_writable(pte)`: PTE가 writable인지 확인
- `is_user_pte(pte)`: user PTE인지 확인
- `is_kern_pte(pte)`: kernel PTE인지 확인
- `pte_get_paddr(pte)`: PTE에서 physical address 부분 추출

`pte.h`에는 flag bit가 정의되어 있다.

- `PTE_P`: present
- `PTE_W`: writable
- `PTE_U`: user/kernel
- `PTE_A`: accessed
- `PTE_D`: dirty

실사용 관점에서는 보통 `pml4_get_page()`, `pml4_set_page()`, `pml4_clear_page()`를 먼저 쓰고, PTE flag helper는 page table을 순회하거나 accessed/dirty 같은 상태를 직접 해석해야 할 때 연결된다.

## 5. Memory Allocation과 page table의 연결

reference는 Pintos에 두 allocator가 있다고 설명한다.

> Pintos에는 두 memory allocator(메모리 할당자)가 있습니다. 하나는 page(페이지) 단위로 memory를 allocate(할당)하고, 다른 하나는 임의 크기의 block(블록)을 allocate할 수 있습니다.
>
> `docs/reference/pintos-kaist-kr/5_appendix/2_memory_allocation.md`

page table과 VM에서 중요한 것은 page allocator이다.

> page allocator는 allocate하는 memory를 kernel pool(커널 풀)과 user pool(사용자 풀)이라는 두 pool(풀)로 나눕니다.
>
> `docs/reference/pintos-kaist-kr/5_appendix/2_memory_allocation.md`

> user pool은 user process(사용자 프로세스)용 memory allocation에, kernel pool은 그 밖의 모든 allocation에 사용해야 합니다. 이는 project 3부터 중요해집니다.
>
> `docs/reference/pintos-kaist-kr/5_appendix/2_memory_allocation.md`

사용 인터페이스:

- `palloc_get_page(flags)`
- `palloc_get_multiple(flags, page_cnt)`
- `palloc_free_page(page)`
- `palloc_free_multiple(pages, page_cnt)`

중요 flag:

- `PAL_ZERO`: 할당된 page를 0으로 초기화
- `PAL_USER`: user pool에서 page 획득
- `PAL_ASSERT`: 실패 시 kernel panic

Project 3의 frame table 설명은 user page에 사용할 frame을 user pool에서 얻어야 한다고 말한다.

> user page에 사용하는 frame은 `palloc_get_page(PAL_USER)`를 호출하여 "user pool(사용자 풀)"에서 얻어야 합니다.
>
> `docs/reference/pintos-kaist-kr/3_project3/0_introduction.md`

따라서 관계는 다음처럼 보면 된다.

1. user virtual page가 필요하다.
2. physical frame을 하나 얻어야 한다.
3. Pintos kernel은 physical frame을 직접 만지지 않고, frame에 대응되는 kernel virtual address를 pointer로 가진다.
4. 그 kernel virtual address를 `pml4_set_page(pml4, upage, kpage, rw)`에 넘긴다.
5. page table entry에는 내부적으로 physical address와 PTE flags가 기록된다.

## 6. Supplemental Page Table과의 구분

`mmu.h`의 PML4 page table은 CPU가 실제 주소 변환에 사용하는 hardware page table 인터페이스이다. 반면 Project 3의 supplemental page table은 page fault와 resource management를 위해 Pintos가 추가로 관리하는 software 자료구조이다.

reference는 supplemental page table을 다음처럼 설명한다.

> supplemental page table은 최소 두 가지 목적으로 사용됩니다. 가장 중요하게는 page fault가 발생했을 때, kernel이 fault(실패)를 일으킨 virtual page를 supplemental page table에서 lookup(조회)하여 그 page에 어떤 data가 있어야 하는지 알아냅니다.
>
> `docs/reference/pintos-kaist-kr/3_project3/0_introduction.md`

구분하면 다음과 같다.

- PML4/page table: CPU가 현재 접근 가능한 virtual page를 physical frame으로 translate하는 실제 매핑
- supplemental page table: 아직 frame에 올라오지 않았거나, file/swap/zero page에서 가져와야 하는 page의 의미와 상태를 기록하는 Pintos의 보조 구조

Project 2에서는 page fault가 대체로 invalid access를 뜻하지만, Project 3에서는 lazy-loaded page나 swapped-out page처럼 처리 가능한 fault일 수 있다. 그래서 VM 이후에는 `pml4_get_page()`만으로 “이 주소가 영원히 invalid”라고 단정할 수 없고, supplemental page table을 함께 봐야 한다.

## 7. 사용 입장에서의 요약

가상 주소 자체를 다룰 때는 `vaddr.h`를 먼저 본다.

- page 크기: `PGSIZE`
- page offset: `pg_ofs()`
- page boundary: `pg_round_down()`, `pg_round_up()`
- user/kernel 범위: `is_user_vaddr()`, `is_kernel_vaddr()`
- physical과 kernel virtual 변환: `ptov()`, `vtop()`

실제 page table mapping을 다룰 때는 `mmu.h`를 본다.

- 새 주소 공간 생성: `pml4_create()`
- 주소 공간 활성화: `pml4_activate()`
- user address의 현재 매핑 조회: `pml4_get_page()`
- user page를 frame에 매핑: `pml4_set_page()`
- 매핑을 not-present로 표시: `pml4_clear_page()`
- page replacement/write-back 판단: `pml4_is_accessed()`, `pml4_is_dirty()`

내부 이유를 이해하려면 `pte.h`를 본다.

- x86-64 virtual address는 PML4, PDPE, PDX, PTX, offset으로 나뉜다.
- PTE에는 physical frame address와 present/writable/user/accessed/dirty 같은 flag가 들어간다.
- `threads/mmu.c`는 이 4-level 구조를 걸어 내려가며 필요한 table을 만들거나 PTE를 찾는다.

결론적으로, Pintos에서 `vaddr.h`는 “주소 계산과 범위 판정”, `mmu.h`는 “주소 공간과 page mapping 조작”, `pte.h`는 “x86-64 하드웨어 page table 형식”을 담당한다고 보면 된다.
