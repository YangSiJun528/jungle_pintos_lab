## Anonymous Page (익명 페이지)

이 project의 이 부분에서는 *anonymous page*(익명 페이지)라고 부르는 non-disk based image(디스크 기반이 아닌 이미지)를 구현합니다.

anonymous mapping(익명 매핑)은 backing file(뒷받침 파일)이나 device(장치)가 없습니다. file-backed page(파일 기반 페이지)와 달리 이름이 있는 file source(파일 원본)가 없기 때문에 anonymous입니다. anonymous page는 stack(스택)과 heap(힙)처럼 executable(실행 파일)에서 사용됩니다.

anonymous page를 설명하는 struct(구조체)로 `include/vm/anon.h`의 `anon_page`가 있습니다. 현재는 비어 있지만, 구현하면서 anonymous page의 necessary information(필요한 정보)이나 state(상태)를 저장하기 위해 member(멤버)를 추가할 수 있습니다. 또한 page의 generic information(일반 정보)을 포함하는 `include/vm/page.h`의 `struct page`를 참조하세요. anonymous page의 경우 page structure 안에 `struct anon_page anon`이 포함됩니다.

### Page Initialization with Lazy Loading (Lazy Loading을 통한 Page 초기화)

Lazy loading(지연 로딩)은 memory(메모리) load(적재)를 실제로 필요한 시점까지 미루는 design(설계)입니다. page가 allocate(할당)되어 그에 대응하는 page struct는 있지만 dedicated physical frame(전용 물리 프레임)은 없고, page의 실제 content(내용)도 아직 load되지 않았습니다. content는 page fault(페이지 폴트)가 signal(신호)하는, 정말 필요해지는 시점에만 load됩니다.

page type(페이지 타입)이 세 가지이므로 initialization routine(초기화 루틴)도 각 page마다 다릅니다. 아래 section에서 다시 설명하겠지만, 여기서는 page initialization flow(초기화 흐름)의 high-level view(상위 수준 개요)를 제공합니다. 먼저 kernel(운영체제 핵심부)이 새 page request(요청)를 받으면 `vm_alloc_page_with_initializer`가 invoke(호출)됩니다. initializer(초기화 함수)는 page structure를 allocate하고 page type에 따라 적절한 initializer를 설정하여 새 page를 initialize한 뒤 control(제어)을 user program(사용자 프로그램)에 돌려줍니다. user program이 실행되다가 어느 시점에 자신이 소유한다고 생각하지만 아직 content가 없는 page에 접근하려 하므로 page fault가 발생합니다. fault handling procedure(폴트 처리 절차) 중 `uninit_initialize`가 invoke되고, 이전에 설정한 initializer를 호출합니다. anonymous page에는 `anon_initializer`, file-backed page에는 `file_backed_initializer`가 initializer가 됩니다.

page는 initialize -> (page_fault -> lazy-load -> swap-in -> swap-out -> ...) -> destroy라는 life cycle(수명 주기)을 가질 수 있습니다. life cycle의 각 transition(전이)마다 필요한 procedure(절차)는 page type 또는 `VM_TYPE`에 따라 다르며, 이전 paragraph(문단)는 initialization에 대한 예였습니다. 이 project에서는 각 page type에 대해 이러한 transition process를 구현합니다.

### Lazy Loading for Executable (실행 파일 지연 로딩)

lazy loading에서는 process(프로세스)가 execution(실행)을 시작할 때 즉시 필요한 memory 부분만 main memory(주 메모리)에 load합니다. 이는 모든 binary image(바이너리 이미지)를 한 번에 memory에 load하는 eager loading(즉시 로딩)에 비해 overhead(부담)를 줄일 수 있습니다.

lazy loading을 지원하기 위해 `include/vm/vm.h`에 `VM_UNINIT`이라는 page type을 도입합니다. 모든 page는 처음에 `VM_UNINIT` page로 생성됩니다. 또한 uninitialized page를 위한 page structure인 `include/vm/uninit.h`의 `struct uninit_page`를 제공합니다. uninitialized page를 create(생성), initialize(초기화), destroy(파괴)하는 function은 `include/vm/uninit.c`에서 찾을 수 있습니다. 이 function들을 나중에 완성해야 합니다.

page fault가 발생하면 page fault handler(`userprog/exception.c`의 `page_fault`)가 control을 `vm/vm.c`의 `vm_try_handle_fault`로 넘깁니다. 이 function은 먼저 valid page fault(유효한 페이지 폴트)인지 확인합니다. 여기서 *valid*란 invalid access(잘못된 접근)를 뜻합니다. *bogus* fault(처리 가능한 폴트)라면 page에 content를 load하고 user program으로 control을 돌려줍니다.

bogus page fault에는 세 가지 case(경우)가 있습니다. lazy-loaded page(지연 로드 페이지), swaped-out page(스왑 아웃된 페이지), write-protected page(쓰기 보호 페이지, [Copy-on-Write (Extra)](6_cow.md) 참조)입니다. 지금은 첫 번째 case인 lazy-loaded page만 생각하세요. lazy loading을 위한 page fault라면 kernel은 `vm_alloc_page_with_initializer`에서 이전에 설정한 initializer 중 하나를 호출하여 segment(세그먼트)를 lazy load합니다. `userprog/process.c`의 `lazy_load_segment`를 구현해야 합니다.

**`vm_alloc_page_with_initializer()`를 구현하세요.** 전달된 vm_type에 따라 적절한 initializer를 fetch(가져오기)하고, 그것으로 `uninit_new`를 호출해야 합니다.

* * *
    
    
    bool vm_alloc_page_with_initializer (enum vm_type type, void *va,
            bool writable, vm_initializer *init, void *aux);
    

> 주어진 type으로 uninitialized page를 create합니다. uninit page의 swap_in handler(핸들러)는 type에 따라 page를 자동으로 initialize하고, 주어진 AUX로 INIT을 호출합니다. page struct를 얻으면 process의 supplementary page table에 page를 insert(삽입)합니다. `vm.h`에 정의된 `VM_TYPE` macro(매크로)를 사용하면 편리할 수 있습니다.

page fault handler는 call chain(호출 체인)을 따라가며, swap_in을 호출할 때 마침내 `uninit_intialize`에 도달합니다. 이 function의 complete implementation(완전한 구현)을 제공합니다. 다만 design에 따라 `uninit_initialize`를 수정해야 할 수도 있습니다.

* * *
    
    
    static bool uninit_initialize (struct page *page, void *kva);
    

> 첫 fault에서 page를 initialize합니다. template code는 먼저 `vm_initializer`와 `aux`를 fetch하고, function pointer(함수 포인터)를 통해 대응되는 page_initializer를 호출합니다. design에 따라 function을 수정해야 할 수 있습니다.

필요에 따라 `vm/anon.c`의 `vm_anon_init`과 `anon_initializer`를 수정할 수 있습니다.

* * *
    
    
    void vm_anon_init (void);
    

> anonymous page subsystem(하위 시스템)을 initialize합니다. 이 function 안에서 anonymous page와 관련된 어떤 것이든 setup(설정)할 수 있습니다.

* * *
    
    
    bool anon_initializer (struct page *page,enum vm_type type, void *kva);
    

> 이 function은 먼저 `page->operations`에 anonymous page용 handler를 setup합니다. 현재 empty struct(빈 구조체)인 `anon_page`의 일부 information을 update(갱신)해야 할 수 있습니다. 이 function은 anonymous page, 즉 `VM_ANON`의 initializer로 사용됩니다.

**`userprog/process.c`에서 `load_segment`와 `lazy_load_segment`를 구현하세요.** executable에서 segment loading(세그먼트 로딩)을 구현합니다. 이 page들은 모두 lazily load되어야 합니다. 즉 kernel이 해당 page fault를 intercept(가로챔)할 때에만 load됩니다.

program loader(프로그램 로더)의 core(핵심)인 `userprog/process.c`의 `load_segment` loop(반복문)를 수정해야 합니다. loop의 각 iteration(반복)마다 `vm_alloc_page_with_initializer`를 호출하여 pending page object(대기 중인 페이지 객체)를 만듭니다. page fault가 발생할 때가 실제로 file에서 segment가 load되는 시점입니다.

* * *
    
    
    static bool load_segment (struct file *file, off_t ofs, uint8_t *upage,
            uint32_t read_bytes, uint32_t zero_bytes, bool writable);
    

> 현재 code는 main loop 안에서 file에서 읽을 byte 수와 zero(0)로 채울 byte 수를 계산합니다. 그런 다음 `vm_alloc_page_with_initializer`를 호출하여 pending object를 만듭니다. `vm_alloc_page_with_initializer`에 제공할 `aux` argument(인자)로 auxiliary value(보조 값)를 setup해야 합니다. binary loading(바이너리 로딩)에 필요한 information을 포함하는 structure를 만들고 싶을 수 있습니다.

* * *
    
    
    static bool lazy_load_segment (struct page *page, void *aux);
    

> `load_segment`에서 `vm_alloc_page_with_initializer`의 네 번째 argument로 `lazy_load_segment`가 제공된다는 것을 보았을 것입니다. 이 function은 executable page의 initializer이며 page fault 시 invoke됩니다. page struct와 `aux`를 argument로 받습니다. `aux`는 `load_segment`에서 setup한 information입니다. 이 information을 사용하여 segment를 읽을 file을 찾고, 결국 segment를 memory에 읽어 넣어야 합니다.

**`userprog/process.c`의 `setup_stack`을 새 memory management system에 stack allocation(스택 할당)이 맞도록 조정해야 합니다.** 첫 stack page는 lazily allocate할 필요가 없습니다. load time(로드 시점)에 command line argument(명령줄 인자)와 함께 allocate하고 initialize할 수 있으며, fault될 때까지 기다릴 필요가 없습니다. stack을 identify(식별)할 방법을 제공해야 할 수 있습니다. page를 mark(표시)하기 위해 `vm/vm.h`의 `vm_type`에 있는 auxillary marker(보조 마커), 예를 들어 `VM_MARKER_0`을 사용할 수 있습니다.

마지막으로 faulted address(폴트가 난 주소)에 대응하는 page struct를 supplemental page table에서 `spt_find_page`로 찾아 해결하도록 `vm_try_handle_fault` function을 수정하세요.

모든 요구사항을 구현한 뒤에는 fork를 제외한 project 2의 모든 test가 통과해야 합니다.

### Supplemental Page Table - Revisit (보조 페이지 테이블 다시 보기)

이제 copy(복사)와 cleanup(정리) operation을 지원하기 위해 supplemental page table interface를 다시 살펴봅니다. 이 operation들은 process를 create(더 구체적으로 child process를 생성)하거나 destroy(파괴)할 때 필요합니다. 자세한 내용은 아래에 설명합니다. 이 시점에서 supplemental page table을 다시 보는 이유는 위에서 구현한 initializing function 중 일부를 사용하고 싶을 수 있기 때문입니다.

**`vm/vm.c`에서 `supplemental_page_table_copy`와 `supplemental_page_table_kill`을 구현하세요.**

* * *
    
    
    bool supplemental_page_table_copy (struct supplemental_page_table *dst,
        struct supplemental_page_table *src);
    

> supplemental page table을 src에서 dst로 copy합니다. child가 parent(부모)의 execution context(실행 문맥)를 inherit(상속)해야 할 때, 즉 `fork()`에서 사용됩니다. src의 supplemental page table에 있는 각 page를 iterate(순회)하고, entry(항목)의 exact copy(정확한 복사본)를 dst의 supplemental page table에 만듭니다. uninit page를 allocate하고 즉시 claim해야 합니다.

* * *
    
    
    void supplemental_page_table_kill (struct supplemental_page_table *spt);
    

> supplemental page table이 보유하던 모든 resource(자원)를 free(해제)합니다. 이 function은 process가 exit할 때(`userprog/process.c`의 `process_exit()`) 호출됩니다. page entry들을 iterate하고 table 안의 page에 대해 `destroy(page)`를 호출해야 합니다. 이 function에서는 actual page table(`pml4`)과 physical memory(`palloc`된 memory)에 대해 걱정할 필요가 없습니다. caller(호출자)가 supplemental page table cleanup 이후 그것들을 정리합니다.

### Page Cleanup (페이지 정리)

**`vm/uninit.c`의 `uninit_destroy`와 `vm/anon.c`의 `anon_destroy`를 구현하세요.** 이는 uninitialized page에 대한 `destroy` operation의 handler입니다. uninitialized page가 다른 page object로 transmute(변환)되더라도 process가 exit할 때 아직 uninit page가 남아 있을 수 있습니다.

* * *
    
    
    static void uninit_destroy (struct page *page);
    

> page struct가 보유하던 resource를 free합니다. page의 vm type을 check하고 그에 맞게 처리하고 싶을 수 있습니다.

지금은 anonymous page만 처리해도 됩니다. 이후 file-backed page를 cleanup하기 위해 이 function을 다시 보게 됩니다.

* * *
    
    
    static void anon_destroy (struct page *page);
    

> anonymous page가 보유하던 resource를 free합니다. page struct 자체를 명시적으로 free할 필요는 없습니다. caller가 해야 합니다.

이제 project 2의 모든 test가 통과해야 합니다.
