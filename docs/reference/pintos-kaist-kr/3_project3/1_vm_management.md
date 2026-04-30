## Memory Management (메모리 관리)

virtual memory system(가상 메모리 시스템)을 지원하려면 virtual page(가상 페이지)와 physical frame(물리 프레임)을 효과적으로 manage(관리)해야 합니다. 이는 어떤 virtual/physical memory region(메모리 영역)이 사용 중인지, 어떤 목적에 쓰이는지, 누가 쓰는지 등을 추적해야 함을 뜻합니다. 먼저 supplemental page table(보조 페이지 테이블)을 다룬 뒤 physical frame을 다룹니다. 이해를 돕기 위해 여기서는 virtual page에는 "page"라는 용어를, physical page에는 "frame"이라는 용어를 사용합니다.

### Page Structure and Operations (Page 구조와 연산)

#### struct page

`include/vm/vm.h`에 정의된 `page`는 virtual memory의 page를 나타내는 structure(구조체)입니다. page에 대해 알아야 하는 모든 necessary data(필요한 데이터)를 저장합니다. 현재 template(템플릿)에서 structure는 다음과 같습니다.
    
    
    struct page {
      const struct page_operations *operations;
      void *va;              /* Address in terms of user space */
      struct frame *frame;   /* Back reference for frame */
    
      union {
        struct uninit_page uninit;
        struct anon_page anon;
        struct file_page file;
    #ifdef EFILESYS
        struct page_cache page_cache;
    #endif
      };
    };
    

이 structure는 page operations(페이지 연산, 아래 참조), virtual address(가상 주소), physical frame을 가집니다. 추가로 union field(공용체 필드)를 가집니다. union은 하나의 memory region에 서로 다른 type(타입)의 data를 저장할 수 있게 하는 특수 data type(자료형)입니다. union에는 여러 member(멤버)가 있지만, 한 번에 하나의 member만 값을 포함할 수 있습니다. 이는 system의 page가 uninit_page, anon_page, file_page, page_cache 중 하나가 될 수 있음을 의미합니다. 예를 들어 page가 anonymous page(익명 페이지, [Anonymous Page](2_anon.md) 참조)라면 page struct는 member 중 하나로 `struct anon_page anon` field를 가집니다. `anon_page`는 anonymous page에 대해 유지해야 하는 필요한 모든 information(정보)을 포함합니다.

#### Page Operations (Page 연산)

위에서 설명하고 `include/vm/vm.h`에 정의된 것처럼 page는 `VM_UNINIT`, `VM_ANON`, `VM_FILE`일 수 있습니다. page에는 swapping in(스왑 인), swapping out(스왑 아웃), page destroy(파괴) 같은 여러 action(동작)이 필요합니다. 각 page type(페이지 타입)에 대해 이러한 action에 필요한 step(단계)과 task(작업)가 다릅니다. 즉 `VM_ANON` page와 `VM_FILE` page에는 서로 다른 `destroy` function(함수)이 invoke(호출)되어야 합니다. 한 가지 방법은 각 function에서 switch-case syntax(문법)를 사용하여 case별로 처리하는 것입니다. 여기서는 이를 다루기 위해 Object-Oriented-Programming(OOP, 객체 지향 프로그래밍)의 "class inheritance(클래스 상속)" concept(개념)을 도입합니다. 실제로 C programming language(프로그래밍 언어)에는 "class"나 "inheritance"가 없지만, Linux 같은 실제 operating system code(운영체제 코드)와 비슷한 방식으로 [function pointers](https://www.geeksforgeeks.org/function-pointer-in-c/)를 사용하여 그 concept을 구현합니다.

function pointer(함수 포인터)는 지금까지 배운 다른 pointer(포인터)와 마찬가지로 memory 안의 function, 즉 executable code(실행 가능한 코드)를 가리키는 pointer입니다. function pointer는 run-time value(실행 시점 값)에 따라 별도의 check 없이 특정 function을 호출해 실행할 수 있는 단순한 방법을 제공하므로 유용합니다. 우리 경우 code-level(코드 수준)에서는 단순히 `destroy (page)`를 호출하는 것만으로 충분하며, compiler(컴파일러)는 올바른 function pointer를 호출하여 page type에 맞는 `destroy` routine(루틴)을 선택합니다.

page operation을 위한 structure `struct page_operations`는 `include/vm/vm.h`에 정의되어 있습니다. 이 structure를 세 function pointer를 담은 function table(함수 테이블)로 생각하세요.
    
    
    struct page_operations {
      bool (*swap_in) (struct page *, void *);
      bool (*swap_out) (struct page *);
      void (*destroy) (struct page *);
      enum vm_type type;
    };
    

이제 page_operation structure를 어디에서 찾을 수 있는지 봅시다. `include/vm/vm.h`의 page structure `struct page`를 보면 `operations`라는 field가 있습니다. 이제 `vm/file.c`로 가 보면 function prototype(원형) 앞에 선언된 page_operations structure `file_ops`를 볼 수 있습니다. 이것은 file-backed page(파일 기반 페이지)를 위한 function pointer table입니다. `.destroy` field는 `file_backed_destroy` 값을 가지며, 이는 같은 file에 정의된 page destroy function입니다.

function pointer interface(인터페이스)로 `file_backed_destroy`가 어떻게 invoke되는지 이해해 봅시다. `vm/vm.c`의 `vm_dealloc_page (page)`가 호출되고, 이 page가 file-backed page(`VM_FILE`)라고 합시다. function 안에서는 `destroy (page)`를 invoke합니다. `destroy (page)`는 `include/vm/vm.h`에서 다음과 같은 macro(매크로)로 정의되어 있습니다.
    
    
    #define destroy(page) if ((page)->operations->destroy) (page)->operations->destroy (page)
    

이는 `destroy` function을 호출하면 실제로 `(page)->operations->destroy (page)`가 invoke된다는 뜻입니다. 이는 page structure에서 가져온 `destroy` function입니다. page가 `VM_FILE` page이므로 `.destroy` field는 `file_backed_destory`를 가리킵니다. 결과적으로 file-backed page용 destroy routine이 수행됩니다.

### Implement Supplemental Page Table (보조 페이지 테이블 구현)

이 시점에서 여러분의 Pintos에는 memory의 virtual/physical mapping(가상/물리 매핑)을 manage하기 위한 page table(`pml4`)이 있습니다. 하지만 이것만으로는 충분하지 않습니다. 이전 section에서 논의했듯이 page fault(페이지 폴트)와 resource management(자원 관리)를 처리하려면 각 page에 대한 additional information(추가 정보)을 담는 supplementary page table(보조 페이지 테이블)도 필요합니다. 따라서 project 3의 첫 task(작업)로 supplemental page table의 basic functionality(기본 기능)를 구현하는 것을 제안합니다.

**`vm/vm.c`에 supplemental page table management function을 구현하세요.**

먼저 Pintos에서 supplemental page table을 어떻게 design(설계)할지 결정해야 합니다. 자신만의 supplemental page table을 design한 뒤, 그 design에 맞춰 아래 세 function을 구현하세요.

* * *
    
    
    void supplemental_page_table_init (struct supplemental_page_table *spt);
    

> supplemental page table을 initialize(초기화)합니다. supplemental page table에 사용할 data structure(자료구조)는 직접 선택할 수 있습니다. 이 function은 새 process(프로세스)가 시작될 때(`userprog/process.c`의 `initd`)와 process가 fork(복제)될 때(`userprog/process.c`의 `__do_fork`) 호출됩니다.

* * *
    
    
    struct page *spt_find_page (struct supplemental_page_table *spt, void *va);
    

> 주어진 supplemental page table에서 va에 대응하는 `struct page`를 찾습니다. 실패하면 NULL을 return(반환)합니다.

* * *
    
    
    bool spt_insert_page (struct supplemental_page_table *spt, struct page *page);
    

> 주어진 supplemental page table에 `struct page`를 insert(삽입)합니다. 이 function은 virtual address가 주어진 supplemental page table에 이미 존재하지 않는지 check(확인)해야 합니다.

### Frame Management (프레임 관리)

이제부터 모든 page는 만들어졌을 때의 memory meta-data(메타데이터)만 들고 있는 것이 아닙니다. 따라서 physical memory를 manage하기 위한 다른 scheme(방식)이 필요합니다. `include/vm/vm.h`에는 physical memory를 나타내는 `struct frame`이 있습니다. 현재 structure는 다음과 같습니다.
    
    
    /* The representation of "frame" */
    struct frame {
      void *kva;
      struct page *page;
    };
    

field는 두 개뿐입니다. `kva`는 kernel virtual address(커널 가상 주소)이고, `page`는 page structure입니다. frame management interface를 구현하면서 member를 더 추가해도 됩니다.

**`vm/vm.c`에서 `vm_get_frame`, `vm_claim_page`, `vm_do_claim_page`를 구현하세요.**

* * *
    
    
    static struct frame *vm_get_frame (void);
    

> `palloc_get_page`를 호출하여 user pool(사용자 풀)에서 새 physical page(물리 페이지)를 얻습니다. user pool에서 page를 성공적으로 얻으면 frame도 allocate(할당)하고, member를 initialize한 뒤 반환합니다. `vm_get_frame`을 구현한 뒤에는 모든 user space page(PALLOC_USER)를 이 function을 통해 allocate해야 합니다. 지금은 page allocation failure(페이지 할당 실패) 시 swap out(스왑 아웃)을 처리할 필요가 없습니다. 일단 그런 case(경우)를 `PANIC ("todo")`로 표시하세요.

* * *
    
    
    bool vm_do_claim_page (struct page *page);
    

> page를 claim(점유)합니다. 즉 physical frame을 allocate합니다. 먼저 `vm_get_frame`을 호출하여 frame을 얻습니다. 이 부분은 template에서 이미 되어 있습니다. 그런 다음 MMU(memory management unit, 메모리 관리 장치)를 설정해야 합니다. 다시 말해 page table에 virtual address에서 physical address로의 mapping을 추가합니다. return value는 operation이 성공했는지 나타내야 합니다.

* * *
    
    
    bool vm_claim_page (void *va);
    

> `va`에 allocate할 page를 claim합니다. 먼저 page를 얻은 뒤 그 page로 `vm_do_claim_page`를 호출해야 합니다.
