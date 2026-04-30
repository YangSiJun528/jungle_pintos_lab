# Memory Allocation (메모리 할당)

Pintos에는 두 memory allocator(메모리 할당자)가 있습니다. 하나는 page(페이지) 단위로 memory를 allocate(할당)하고, 다른 하나는 임의 크기의 block(블록)을 allocate할 수 있습니다.

## Page Allocator (페이지 할당자)

`include/threads/palloc.h`에 선언된 page allocator는 page 단위로 memory를 allocate합니다. 보통 한 번에 한 page씩 memory를 allocate하는 데 사용되지만, 여러 contiguous page(연속 페이지)를 한 번에 allocate할 수도 있습니다.

page allocator는 allocate하는 memory를 kernel pool(커널 풀)과 user pool(사용자 풀)이라는 두 pool(풀)로 나눕니다. 기본적으로 각 pool은 1MB 위의 system memory(시스템 메모리)를 절반씩 받지만, **ul** kernel command line option(커널 명령줄 옵션)으로 division(분할)을 바꿀 수 있습니다. allocation request(할당 요청)는 두 pool 중 하나에서 가져옵니다. 한 pool이 empty(비어 있음)가 되어도 다른 pool에는 free page(빈 페이지)가 남아 있을 수 있습니다. user pool은 user process(사용자 프로세스)용 memory allocation에, kernel pool은 그 밖의 모든 allocation에 사용해야 합니다. 이는 project 3부터 중요해집니다. 그 전까지는 모든 allocation을 kernel pool에서 해야 합니다.

각 pool의 usage(사용 상태)는 page당 1 bit(비트)를 사용하는 bitmap(비트맵)으로 추적됩니다. n page를 allocate하라는 request는 bitmap에서 false로 설정된 n개의 consecutive bit(연속 비트)를 scan(검색)합니다. 이는 해당 page들이 free임을 나타냅니다. 그런 다음 그 bit들을 true로 설정하여 used(사용 중)로 표시합니다. 이는 "first fit(최초 적합)" allocation strategy(할당 전략)입니다.

page allocator는 fragmentation(단편화)의 영향을 받습니다. 즉 n개 이상의 page가 free여도, free page들이 used page 사이에 흩어져 있으면 n개의 contiguous page를 allocate하지 못할 수 있습니다. 실제로 pathological case(병적인 경우)에서는 pool page의 절반이 free여도 2개의 contiguous page를 allocate하지 못할 수 있습니다. single-page request는 fragmentation 때문에 fail(실패)할 수 없으므로, 여러 contiguous page request는 가능한 한 제한해야 합니다.

page는 interrupt context(인터럽트 문맥)에서 allocate할 수 없지만 free할 수는 있습니다.

page가 free되면 debugging aid(디버깅 보조)로 모든 byte(바이트)가 **0xcc**로 clear(초기화)됩니다([Debugging Tips](2_memory_allocation.md) 참조).

page allocator type(타입)과 function(함수)은 아래에 설명되어 있습니다.

* * *
    
    
    void *palloc_get_page (enum palloc_flags flags)
    void *palloc_get_multiple (enum palloc_flags flags, size_t page_cnt)
    

> 각각 한 page 또는 *page_cnt*개의 contiguous page를 obtain(획득)하여 반환합니다. page를 allocate할 수 없으면 null pointer(널 포인터)를 반환합니다.
> 
> *flags* argument(인자)는 다음 flag(플래그)의 조합일 수 있습니다.
> 
>   - `PAL_ASSERT`
> 
> page를 allocate할 수 없으면 kernel panic(커널 패닉)을 일으킵니다. 이는 kernel initialization(커널 초기화) 중에만 적절합니다. user process가 kernel panic을 일으키도록 허용해서는 안 됩니다.
> 
>   - `PAL_ZERO`
> 
> allocate된 page의 모든 byte를 반환하기 전에 zero(0)로 만듭니다. 설정하지 않으면 새로 allocate된 page의 contents(내용)는 예측할 수 없습니다.
> 
>   - `PAL_USER`
> 
> user pool에서 page를 obtain합니다. 설정하지 않으면 kernel pool에서 page가 allocate됩니다.

* * *
    
    
    void palloc_free_page (void *page)
    void palloc_free_multiple (void *pages, size_t page_cnt)
    

> 각각 한 page 또는 *pages*에서 시작하는 page cnt개의 contiguous page를 free(해제)합니다. 모든 page는 `palloc_get_page()` 또는 `palloc_get_multiple()`로 얻은 것이어야 합니다.

## Block Allocator (블록 할당자)

`threads/malloc.h`에 선언된 block allocator는 임의 크기의 block을 allocate할 수 있습니다. 이는 이전 section에서 설명한 page allocator 위에 layer(계층)로 구현되어 있습니다. block allocator가 반환하는 block은 kernel pool에서 얻습니다.

block allocator는 memory allocate에 두 가지 strategy를 사용합니다. 첫 번째 strategy는 1 kB 이하의 block, 즉 page size의 1/4 이하 block에 적용됩니다. 이 allocation은 가장 가까운 power of 2(2의 거듭제곱) 또는 16 bytes 중 더 큰 값으로 round up(올림)됩니다. 그런 다음 해당 크기의 allocation에만 사용되는 page로 group(그룹화)됩니다.

두 번째 strategy는 1 kB보다 큰 block에 적용됩니다. 이런 allocation은 작은 overhead(부가 비용)를 더한 뒤 가장 가까운 page size로 round up되고, block allocator가 page allocator에 그 수만큼 contiguous page를 request합니다.

어느 경우든 request된 allocation size(할당 크기)와 실제 block size의 차이는 wasted(낭비)됩니다. 실제 operating system(운영체제)은 이 낭비를 최소화하도록 allocator를 정교하게 tune(조정)하지만, Pintos 같은 instructional system(교육용 시스템)에서는 중요하지 않습니다.

page allocator에서 page를 얻을 수 있는 한 small allocation(작은 할당)은 항상 성공합니다. 대부분의 small allocation은 이미 allocate된 page의 일부를 사용하여 satisfy(충족)되므로 page allocator에서 새 page를 전혀 요구하지 않습니다. 하지만 large allocation(큰 할당)은 항상 page allocator 호출을 필요로 하며, 하나보다 많은 contiguous page가 필요한 allocation은 앞 section에서 논의한 것처럼 fragmentation 때문에 fail할 수 있습니다. 따라서 code에서 large allocation 수를 최소화해야 하며, 특히 각각 약 4 kB를 넘는 allocation은 더 그렇습니다.

block이 free되면 debugging aid로 모든 byte가 **0xcc**로 clear됩니다([Debugging Tips](2_memory_allocation.md) 참조).

block allocator는 interrupt context에서 호출할 수 없습니다.

block allocator function은 아래에 설명되어 있습니다. interface(인터페이스)는 같은 이름의 standard C library(표준 C 라이브러리) function과 같습니다.

* * *
    
    
    void *malloc (size_t size)
    

> kernel pool에서 최소 *size* bytes 길이의 새 block을 obtain하여 반환합니다. size가 zero(0)이거나 memory가 available(사용 가능)하지 않으면 null pointer를 반환합니다.

* * *
    
    
    void *calloc (size_t a, size_t b)
    

> kernel pool에서 최소 a * b bytes 길이의 새 block을 obtain하여 반환합니다. block contents는 zero로 clear됩니다. a 또는 b가 zero이거나 memory가 부족하면 null pointer를 반환합니다.

* * *
    
    
    void *realloc (void *block, size_t new_size)
    

> *block*을 *new_size* bytes로 resize(크기 변경)하려고 시도하며, 과정에서 이동될 수 있습니다. 성공하면 새 block을 반환하며, 이 경우 old block(이전 블록)에 더 이상 접근해서는 안 됩니다. 실패하면 null pointer를 반환하고 old block은 계속 valid(유효)합니다. block이 null인 call은 `malloc()`과 equivalent(동등)합니다. new size가 zero인 call은 `free()`와 equivalent합니다.

* * *
    
    
    void free (void *block)
    

> *block*을 free합니다. block은 이전에 `malloc()`, `calloc()`, 또는 `realloc()`이 반환했고 아직 free되지 않은 것이어야 합니다.
