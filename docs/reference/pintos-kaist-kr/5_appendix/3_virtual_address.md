# Virtual Addresses (가상 주소)

64-bit virtual address(가상 주소)는 다음과 같이 구조화됩니다.
    
    
    63          48 47            39 38            30 29            21 20         12 11         0
    +-------------+----------------+----------------+----------------+-------------+------------+
    | Sign Extend |    Page-Map    | Page-Directory | Page-directory |  Page-Table |  Physical  |
    |             | Level-4 Offset |    Pointer     |     Offset     |   Offset    |   Offset   |
    +-------------+----------------+----------------+----------------+-------------+------------+
                  |                |                |                |             |            |
                  +------- 9 ------+------- 9 ------+------- 9 ------+----- 9 -----+---- 12 ----+
                                              Virtual Address
    

Header(헤더) `include/threads/vaddr.h`와 `include/threads/mmu.h`는 virtual address를 다루기 위한 다음 function(함수)과 macro(매크로)를 정의합니다.

* * *
    
    
    #define PGSHIFT { /* Omit details */ }
    #define PGBITS { /* Omit details */ }
    

> virtual address의 offset(오프셋) 부분에 대한 bit index(비트 인덱스, 0)와 bit 수(12)를 각각 나타냅니다.

* * *
    
    
    #define PGMASK { /* Omit details */ }
    

> page offset(페이지 오프셋)의 bit는 1로, 나머지는 0으로 설정된 bit mask(비트 마스크, 0xfff)입니다.

* * *
    
    
    #define PGSIZE { /* Omit details */ }
    

> bytes(바이트) 단위의 page size(페이지 크기, 4,096)입니다.

* * *
    
    
    #define pg_ofs(va) { /* Omit details */ }
    

> virtual address va에서 page offset을 extract(추출)하여 반환합니다.

* * *
    
    
    #define pg_no(va) { /* Omit details */ }
    

> virtual address va에서 page number(페이지 번호)를 extract하여 반환합니다.

* * *
    
    
    #define pg_round_down(va) { /* Omit details */ }
    

> va가 가리키는 virtual page(가상 페이지)의 시작 주소, 즉 page offset이 0으로 설정된 va를 반환합니다.

* * *
    
    
    #define pg_round_up(va) { /* Omit details */ }
    

> va를 가장 가까운 page boundary(페이지 경계)로 round up(올림)하여 반환합니다.

Pintos의 virtual memory(가상 메모리)는 user virtual memory(사용자 가상 메모리)와 kernel virtual memory(커널 가상 메모리)라는 두 region(영역)으로 나뉩니다([Virtual Memory Layout](https://casys-kaist.github.io/pintos-kaist/project2/introduction) 참조).

둘 사이의 boundary(경계)는 `KERN_BASE`입니다.

* * *
    
    
    #define KERN_BASE { /* Omit details */ }
    

> kernel virtual memory의 base address(기준 주소)입니다. 기본값은 0x8004000000입니다. user virtual memory는 virtual address 0부터 `KERN_BASE`까지이고, kernel virtual memory는 virtual address space(가상 주소 공간)의 나머지를 차지합니다.

* * *
    
    
    #define is_user_vaddr(vaddr) { /* Omit details */ }
    #define is_kernel_vaddr(vaddr) { /* Omit details */ }
    

> va가 각각 user 또는 kernel virtual address이면 true를, 그렇지 않으면 false를 반환합니다.

* * *

x86-64는 physical address(물리 주소)가 주어졌을 때 memory에 직접 접근하는 방법을 제공하지 않습니다. operating system kernel(운영체제 핵심부)에서는 이 능력이 자주 필요하므로, Pintos는 kernel virtual memory를 physical memory(물리 메모리)에 one-to-one(일대일)으로 mapping(매핑)하여 이를 우회합니다. 즉 virtual address > `KERN_BASE`는 physical address 0에 접근하고, virtual address `KERN_BASE + 0x1234`는 physical address 0x1234에 접근하며, machine(컴퓨터)의 physical memory 크기까지 계속됩니다. 따라서 physical address에 `KERN_BASE`를 더하면 그 address에 접근하는 kernel virtual address를 얻고, 반대로 kernel virtual address에서 `KERN_BASE`를 빼면 대응하는 physical address를 얻습니다.

Header `include/threads/vaddr.h`는 이러한 translation(변환)을 수행하는 function 쌍을 제공합니다.

* * *
    
    
    #define ptov(paddr) { /* Omit details */ }
    

> physical address pa에 대응하는 kernel virtual address를 반환합니다. pa는 0부터 physical memory byte 수 사이여야 합니다.

* * *
    
    
    #define vtop(vaddr) { /* Omit details */ }
    

> va에 대응하는 physical address를 반환합니다. va는 kernel virtual address여야 합니다.

Header `include/threads/mmu.h`는 page table(페이지 테이블)에 대한 operation(연산)을 제공합니다.

* * *
    
    
    #define is_user_pte(pte) { /* Omit details */ }
    #define is_kern_pte(pte) { /* Omit details */ }
    

> page table entry(PTE, 페이지 테이블 항목)가 각각 user 또는 kernel 소유인지 query(질의)합니다.

* * *
    
    
    #define is_writable(pte) { /* Omit details */ }
    

> page table entry(PTE)가 가리키는 virtual address가 writable(쓰기 가능)인지 query합니다.

* * *
    
    
    typedef bool pte_for_each_func (uint64_t *pte, void *va, void *aux);
    bool pml4_for_each (uint64_t *pml4, pte_for_each_func *func, void *aux);
    

> PML4 아래의 각 valid entry(유효 항목)에 대해 auxiliary value(보조 값) AUX와 함께 FUNC를 apply(적용)합니다. VA는 entry의 virtual address를 나타냅니다. pte_for_each_func가 false를 반환하면 iteration(반복)을 멈추고 false를 반환합니다.

아래는 `pml4_for_each`에 전달할 수 있는 `func` example(예시)입니다.
    
    
    static bool
    stat_page (uint64_t *pte, void *va,  void *aux) {
            if (is_user_vaddr (va))
                    printf ("user page: %llx\n", va);
            if (is_writable (va))
                    printf ("writable page: %llx\n", va);
            return true;
    }
