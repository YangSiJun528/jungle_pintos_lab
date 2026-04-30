## Swap In/Out (스왑 입출력)

memory swapping(메모리 스와핑)은 physical memory(물리 메모리) 사용을 극대화하기 위한 memory reclamation technique(메모리 회수 기법)입니다. main memory(주 메모리)의 frame(프레임)이 모두 allocate(할당)되면 system(시스템)은 user program(사용자 프로그램)의 추가 memory allocation request(메모리 할당 요청)를 처리할 수 없습니다. 한 가지 solution(해결책)은 현재 사용 중이지 않은 memory frame을 disk(디스크)로 swap out(내보내기)하는 것입니다. 이렇게 하면 일부 memory resource(자원)가 free(해제)되어 다른 application(애플리케이션)이 사용할 수 있습니다.

swapping은 operating system(운영체제)이 수행합니다. system이 memory가 부족해졌음을 감지했지만 memory allocation request를 받으면, swap disk(스왑 디스크)로 evict(축출)할 page(페이지)를 선택합니다. 그런 다음 memory frame의 exact state(정확한 상태)를 disk에 copy(복사)합니다. process(프로세스)가 swapped out page(스왑 아웃된 페이지)에 접근하려 하면 OS는 정확한 content(내용)를 다시 memory로 가져와 page를 recover(복구)합니다.

eviction 대상으로 선택되는 page는 anonymous page(익명 페이지)일 수도 있고 file-backed page(파일 기반 페이지)일 수도 있습니다. 이 section(절)에서는 각 경우를 처리합니다.

모든 swapping operation(스와핑 연산)은 명시적으로 호출되지 않고 function pointer(함수 포인터)로 호출됩니다. 이들은 각 page initializer(초기화 함수)의 operation으로 register(등록)될 `struct page_operations file_ops`의 member(멤버)입니다.

### Anonymous Page (익명 페이지)

**`vm/anon.c`에서 `vm_anon_init`과 `anon_initializer`를 수정하세요.** anonymous page는 backing storage(뒷받침 저장소)가 없습니다. anonymous page swapping을 지원하기 위해 swap disk라는 temporary backing storage(임시 뒷받침 저장소)를 제공합니다. swap disk를 활용하여 anonymous page의 swap을 구현합니다.

* * *
    
    
    void vm_anon_init (void);
    

> 이 function에서는 swap disk를 setup(설정)해야 합니다. swap disk의 free/used area(빈/사용 중 영역)를 manage(관리)할 data structure(자료구조)도 필요합니다. swap area(스왑 영역)도 PGSIZE(4096 bytes) granularity(단위)로 manage됩니다.

* * *
    
    
    bool anon_initializer (struct page *page, enum vm_type type, void *kva);
    

> anonymous page의 initializer입니다. swapping을 지원하기 위해 `anon_page`에 일부 information(정보)을 추가해야 합니다.

이제 anonymous page swapping을 지원하기 위해 `vm/anon.c`에 `anon_swap_in`과 `anon_swap_out`을 구현하세요. page는 swap in(들여오기)되려면 먼저 swap out되어야 하므로, `anon_swap_in`보다 `anon_swap_out`을 먼저 구현하고 싶을 수 있습니다. data content를 swap disk로 move(이동)하고 안전하게 memory로 다시 가져와야 합니다.

* * *
    
    
    static bool anon_swap_in (struct page *page, void *kva);
    

> disk에서 memory로 data content를 read(읽기)하여 swap disk에서 anonymous page를 swap in합니다. data location(위치)은 page가 swap out될 때 page struct에 저장되어 있어야 합니다. swap table(스왑 테이블)을 update(갱신)해야 한다는 점을 기억하세요([Managing the Swap Table](0_introduction.md) 참조).

* * *
    
    
    static bool anon_swap_out (struct page *page);
    

> memory에서 disk로 content를 copy하여 anonymous page를 swap disk로 swap out합니다. 먼저 swap table을 사용하여 disk에서 free swap slot(빈 스왑 슬롯)을 찾은 뒤, data page를 그 slot으로 copy합니다. data location은 page struct에 저장되어야 합니다. disk에 더 이상 free slot이 없으면 kernel panic(커널 패닉)을 일으켜도 됩니다.

### File-Mapped Page (파일 매핑 페이지)

file-backed page의 content는 file(파일)에서 오므로, mmaped file이 backing store(뒷받침 저장소)로 사용되어야 합니다. 즉 file-backed page를 evict하면 그 page가 mapping된 원래 file에 write back(되쓰기)됩니다. `vm/file.c`에서 `file_backed_swap_in`, `file_backed_swap_out`을 구현하세요. design(설계)에 따라 `file_backed_init`과 `file_initializer`를 수정할 수 있습니다.

* * *
    
    
    static bool file_backed_swap_in (struct page *page, void *kva);
    

> file에서 content를 read하여 `kva` 위치의 page를 swap in합니다. file system(파일 시스템)과 synchronize(동기화)해야 합니다.

* * *
    
    
    static bool file_backed_swap_out (struct page *page);
    

> content를 file에 write back하여 page를 swap out합니다. 먼저 page가 dirty(수정됨)인지 check하고 싶을 수 있습니다. dirty가 아니라면 file의 content를 modify(수정)할 필요가 없습니다. page를 swap out한 뒤에는 page의 dirty bit(더티 비트)를 끄는 것을 기억하세요.
