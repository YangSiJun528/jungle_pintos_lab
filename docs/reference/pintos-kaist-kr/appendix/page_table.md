# Page Table (페이지 테이블)

`threads/mmu.c`의 code(코드)는 x86_64 hardware page table(하드웨어 페이지 테이블)에 대한 abstract interface(추상 인터페이스)입니다. Pintos의 page table, 즉 project에서 사용할 page table은 table이 4 level(단계)을 가지기 때문에 Intel processor documentation(문서)에서 Page-Map-Level-4라고 부르는 `pml4`입니다. page table interface는 internal structure(내부 구조)에 접근하기 편하도록 page table을 나타내는 데 `uint64_t *`를 사용합니다. 아래 section(절)은 page table interface와 internals(내부)를 설명합니다.

## Creation, Destruction, and Activation (생성, 파괴, 활성화)

이 function(함수)들은 page table을 create(생성), destroy(파괴), activate(활성화)합니다. base Pintos code는 필요한 곳에서 이미 이 function들을 호출하므로, 직접 호출할 필요는 없어야 합니다.

* * *
    
    
    uint64_t * pml4_create (void);
    

> 새 page table을 create하고 return(반환)합니다. 새 page table은 Pintos의 normal kernel virtual page mapping(일반 커널 가상 페이지 매핑)을 포함하지만 user virtual mapping(사용자 가상 매핑)은 포함하지 않습니다. memory(메모리)를 obtain(획득)할 수 없으면 null pointer(널 포인터)를 반환합니다.

* * *
    
    
    void pml4_destroy (uint64_t *pml4);
    

> page table 자체와 그것이 mapping하는 frame(프레임)을 포함하여 pml4가 보유한 모든 resource(자원)를 free(해제)합니다. page table의 모든 level에서 resource를 free하기 위해 `pdpe_destroy`, `pgdir_destory`, `pt_destroy`를 *recursively(재귀적으로)* 호출합니다.

* * *
    
    
    void pml4_activate (uint64 t *pml4)
    

> pml4를 activate합니다. active page table(활성 페이지 테이블)은 CPU가 memory reference(메모리 참조)를 translate(변환)하는 데 사용하는 page table입니다.

## Inspection and Updates (검사와 갱신)

이 function들은 page table이 encapsulate(감싸고)하는 page에서 frame으로의 mapping을 examine(검사)하거나 update(갱신)합니다. active page table과 inactive page table, 즉 running process(실행 중인 프로세스)와 suspended process(중단된 프로세스) 모두에서 동작하며, 필요한 경우 TLB(translation lookaside buffer, 주소 변환 캐시)를 flush(비움)합니다.

* * *
    
    
    bool pml4_set_page (uint64_t *pml4, void *upage, void *kpage, bool rw);
    

> user page upage에서 kernel virtual address(커널 가상 주소) kpage로 identify(식별)되는 frame으로의 mapping을 pd에 추가합니다. rw가 true이면 page는 read/write(읽기/쓰기)로 mapping되고, 그렇지 않으면 read-only(읽기 전용)로 mapping됩니다. user page upage는 pml4에 이미 mapping되어 있어서는 안 됩니다. kernel page kpage는 `palloc_get_page(PAL_USER)`로 user pool(사용자 풀)에서 얻은 kernel virtual address여야 합니다. 성공하면 true, 실패하면 false를 반환합니다. failure(실패)는 page table에 필요한 추가 memory를 obtain할 수 없을 때 발생합니다.

* * *
    
    
    void * pml4_get_page (uint64_t *pml4, const void *uaddr);
    

> pml4에서 uaddr에 mapping된 frame을 look up(조회)합니다. uaddr이 mapping되어 있으면 그 frame의 kernel virtual address를 반환하고, 그렇지 않으면 null pointer를 반환합니다.

* * *
    
    
    void pml4_clear_page (uint64_t *pml4, void *upage);
    

> pml4에서 page를 "not present(존재하지 않음)"로 mark(표시)합니다. 이후 해당 page에 접근하면 fault(폴트)가 발생합니다. page table에서 page에 대한 다른 bit(비트)는 preserved(보존)되므로 accessed bit와 dirty bit(다음 section 참조)를 check(확인)할 수 있습니다. page가 mapping되어 있지 않으면 이 function은 아무 효과가 없습니다.

## Accessed and Dirty Bits (accessed/dirty bit)

x86_64 hardware는 각 page의 page table entry(PTE, 페이지 테이블 항목)에 있는 bit 쌍을 통해 page replacement algorithm(페이지 교체 알고리즘) 구현을 일부 지원합니다. page에 대한 read(읽기)나 write(쓰기)가 발생하면 CPU는 page의 PTE에서 accessed bit(접근 비트)를 1로 설정하고, write가 발생하면 dirty bit(더티 비트)를 1로 설정합니다. CPU는 이 bit들을 절대 0으로 reset(초기화)하지 않지만, OS(운영체제)는 그렇게 할 수 있습니다. 이 bit들을 올바르게 interpret(해석)하려면 *aliases*(별칭)를 이해해야 합니다. 즉 같은 frame을 refer(참조)하는 두 개 이상의 page입니다. aliased frame에 접근하면 accessed/dirty bit는 access에 사용된 page의 page table entry 하나에서만 update됩니다. 다른 alias의 accessed/dirty bit는 update되지 않습니다.

* * *
    
    
    bool pml4_is_dirty (uint64_t *pml4, const void *vpage);
    bool pml4_is_accessed (uint64_t *pml4, const void *vpage);
    

> pml4가 vpage에 대해 dirty 또는 accessed로 mark된 page table entry를 포함하면 true를 반환합니다. 그렇지 않으면 false를 반환합니다.

* * *
    
    
    void pml4_set_dirty (uint64_t *pml4, const void *vpage, bool dirty);
    void pml4_set_accessed (uint64_t *pml4, const void *vpage, bool accessed);
    

> pml4가 page에 대한 page table entry를 가지고 있으면, 그 dirty 또는 accessed bit를 주어진 value(값)로 설정합니다.
