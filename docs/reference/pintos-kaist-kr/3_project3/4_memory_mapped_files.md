### Memory Mapped Files (메모리 매핑 파일)

이 section(절)에서는 memory-mapped page(메모리 매핑 페이지)를 구현합니다. anonymous page(익명 페이지)와 달리 memory-mapped page는 file-backed mapping(파일 기반 매핑)입니다. page 안의 content(내용)는 기존 file(파일)의 data(데이터)를 mirror(반영)합니다. page fault(페이지 폴트)가 발생하면 physical frame(물리 프레임)이 즉시 allocate(할당)되고 file에서 memory(메모리)로 content가 copy(복사)됩니다. memory-mapped page가 unmapped(매핑 해제)되거나 swapped out(스왑 아웃)될 때 content의 변경 사항은 file에 반영됩니다.

#### `mmap` and `munmap` System Call (`mmap`과 `munmap` 시스템 콜)

memory mapped file(메모리 매핑 파일)을 위한 두 system call(시스템 콜)인 **`mmap`과 `munmap`을 구현하세요**. 여러분의 VM system(가상 메모리 시스템)은 mmap region(매핑 영역)의 page를 lazily(지연 방식으로) load해야 하며, mmaped file 자체를 mapping의 backing store(뒷받침 저장소)로 사용해야 합니다. 이 두 system call을 구현하려면 `vm/file.c`에 정의된 `do_mmap`과 `do_munmap`을 구현하고 사용해야 합니다.

* * *
    
    
    void *mmap (void *addr, size_t length, int writable, int fd, off_t offset);
    

> `fd`로 open(열린)된 file에서 `offset` byte(바이트)부터 시작하는 `length` bytes를 process(프로세스)의 virtual address space(가상 주소 공간) 안의 `addr`에 map(매핑)합니다. file 전체는 `addr`에서 시작하는 연속적인 virtual page(가상 페이지)에 mapping됩니다. file 길이가 PGSIZE의 배수가 아니면, 마지막 mapped page의 일부 byte가 file 끝을 넘어 "stick out(튀어나옴)"합니다. page가 fault되어 들어올 때 이 byte들을 zero(0)로 설정하고, page가 disk(디스크)에 write back(되쓰기)될 때 버리세요. 성공하면 이 function(함수)은 file이 mapping된 virtual address(가상 주소)를 반환합니다. 실패하면 file을 map하기에 valid address(유효한 주소)가 아닌 NULL을 반환해야 합니다.

fd로 open된 file의 길이가 0 bytes라면 `mmap` call(호출)은 fail(실패)할 수 있습니다. addr이 page-aligned(페이지 정렬)되어 있지 않거나, mapped page range(매핑된 페이지 범위)가 stack(스택)이나 executable load time(실행 파일 로드 시점)에 mapping된 page를 포함하여 기존 mapped page set(집합)과 overlap(겹침)하면 반드시 fail해야 합니다. Linux에서 `addr`이 NULL이면 kernel(운영체제 핵심부)이 mapping을 만들 적절한 address를 찾습니다. 단순화를 위해 주어진 `addr`에 mmap을 시도하면 됩니다. 따라서 `addr`이 0이면 fail해야 합니다. 일부 Pintos code(코드)가 virtual page 0은 mapping되지 않았다고 가정하기 때문입니다. `length`가 zero(0)일 때도 mmap은 fail해야 합니다. 마지막으로 console input/output(콘솔 입출력)을 나타내는 file descriptor(파일 디스크립터)는 mappable(매핑 가능)하지 않습니다.

memory-mapped page도 anonymous page처럼 lazy manner(지연 방식)로 allocate되어야 합니다. page object(페이지 객체)를 만들기 위해 `vm_alloc_page_with_initializer`나 `vm_alloc_page`를 사용할 수 있습니다.

* * *
    
    
    void munmap (void *addr);
    

> 지정된 address range(주소 범위) `addr`에 대한 mapping을 unmap합니다. `addr`은 같은 process가 이전 mmap call에서 반환받았고 아직 unmapped되지 않은 virtual address여야 합니다.

process가 `exit`을 통해서든 다른 방식으로든 exit할 때 모든 mapping은 implicitly(암묵적으로) unmapped됩니다. mapping이 implicitly든 explicitly(명시적으로)든 unmapped될 때, process가 write(쓰기)한 모든 page는 file에 write back되어야 하고, write하지 않은 page는 그러면 안 됩니다. 그 다음 page들은 process의 virtual page list(가상 페이지 리스트)에서 제거됩니다.

file을 close(닫기)하거나 remove(삭제)해도 그 file의 mapping은 unmap되지 않습니다. Unix convention(유닉스 관례)에 따라, mapping은 생성된 뒤 `munmap`이 호출되거나 process가 exit할 때까지 valid(유효)합니다. 자세한 내용은 [Removing an Open File](../2_project2/7_FAQ.md)을 참조하세요. 각 mapping마다 file에 대한 separate and independent reference(별도 독립 참조)를 얻기 위해 `file_reopen` function을 사용해야 합니다.

둘 이상의 process가 같은 file을 map하더라도 consistent data(일관된 데이터)를 볼 필요는 없습니다. Unix는 두 mapping이 같은 physical page(물리 페이지)를 share(공유)하게 하여 이를 처리하며, mmap system call에는 client(클라이언트)가 page를 shared(공유)로 할지 private(비공개, 즉 copy-on-write)으로 할지 지정하는 argument(인자)도 있습니다.

필요에 따라 `vm/vm.c`의 `vm_file_init`과 `vm_file_initializer`를 수정할 수 있습니다.

* * *
    
    
    void vm_file_init (void);
    

> file-backed page subsystem(파일 기반 페이지 하위 시스템)을 initialize(초기화)합니다. 이 function 안에서 file backed page와 관련된 어떤 것이든 setup(설정)할 수 있습니다.

* * *
    
    
    bool file_backed_initializer (struct page *page, enum vm_type type, void *kva);
    

> file-backed page를 initialize합니다. 이 function은 먼저 `page->operations`에 file-backed page용 handler(핸들러)를 setup합니다. memory를 backing하는 file 같은 page struct(구조체)의 information(정보)을 update(갱신)하고 싶을 수 있습니다.

* * *
    
    
    static void file_backed_destroy (struct page *page);
    

> associated file(연관된 파일)을 close하여 file backed page를 destroy(파괴)합니다. content가 dirty(수정됨)라면 변경 사항을 file에 write back해야 합니다. 이 function에서 page struct를 free(해제)할 필요는 없습니다. `file_backed_destroy`의 caller(호출자)가 처리해야 합니다.
