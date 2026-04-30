### Buffer Cache (Extra Credit) (버퍼 캐시)

file system(파일 시스템)을 수정하여 file block(파일 블록)의 cache(캐시)를 유지하세요. block을 read(읽기)하거나 write(쓰기)하라는 request(요청)가 들어오면, 그 block이 cache에 있는지 check(확인)하고, 있다면 disk(디스크)에 가지 않고 cached data(캐시된 데이터)를 사용하세요. 없으면 block을 disk에서 cache로 fetch(가져오기)하고, 필요하다면 오래된 entry(항목)를 evict(축출)합니다. cache는 최대 64 sectors(섹터) 크기로 제한됩니다.

"clock" algorithm(시계 알고리즘)만큼은 좋은 cache replacement algorithm(캐시 교체 알고리즘)을 구현해야 합니다. data(데이터)에 비해 metadata(메타데이터)의 가치가 일반적으로 더 크다는 점을 고려하는 것을 권장합니다. disk access(디스크 접근) 횟수로 측정한 performance(성능)가 가장 좋은 조합이 무엇인지 알아보기 위해 accessed(접근됨), dirty(수정됨), 기타 information(정보)을 실험해 보세요.

원한다면 free map(빈 공간 지도)의 cached copy(캐시된 복사본)를 memory(메모리)에 영구적으로 유지할 수 있습니다. 이는 cache size(캐시 크기)에 포함될 필요가 없습니다.

제공된 inode code는 disk의 sector-by-sector interface(섹터 단위 인터페이스)를 system call interface(시스템 콜 인터페이스)의 byte-by-byte interface(바이트 단위 인터페이스)로 translate(변환)하기 위해 `malloc()`으로 allocate(할당)한 "bounce buffer"(중간 버퍼)를 사용합니다. 이 bounce buffer를 제거해야 합니다. 대신 **buffer cache의 sector 안팎으로 data를 직접 copy(복사)하세요.**

cache는 write-behind(지연 쓰기) 방식이어야 합니다. 즉 modified data(수정된 데이터)를 즉시 disk에 write하지 말고 dirty block(더티 블록)을 cache에 유지합니다. dirty block이 evict될 때 disk에 write하세요. write-behind는 crash(충돌)에 대해 file system을 더 취약하게 만들기 때문에, 추가로 모든 dirty cached block을 주기적으로 disk에 write back(되쓰기)해야 합니다. Pintos를 halt(중지)할 때 cache가 flush(밀어내기)되도록 `filesys_done()`에서도 cache를 disk에 write back해야 합니다.

첫 번째 project의 `timer_sleep()`이 동작한다면 write-behind는 훌륭한 application(응용)입니다. 그렇지 않다면 덜 general(범용적이지 않은) facility(기능)를 구현할 수 있지만, busy-waiting(바쁜 대기)을 보이지 않도록 해야 합니다.

read-ahead(미리 읽기)도 구현해야 합니다. 즉 file의 한 block이 read될 때 그 다음 block이 곧 read될 수 있으므로 자동으로 cache에 fetch합니다. read-ahead는 asynchronous(비동기)로 수행될 때에만 정말 유용합니다. 이는 process(프로세스)가 file에서 disk block 1을 request하면 disk block 1이 read될 때까지 block(대기)해야 하지만, 그 read가 완료되면 control(제어)이 즉시 process로 돌아가야 한다는 뜻입니다. disk block 2에 대한 read-ahead request는 background(백그라운드)에서 asynchronously 처리되어야 합니다. 이를 위해 I/O(입출력)용 dedicated thread(전용 스레드)를 만들어야 할 수 있습니다. disk I/O가 필요하면 thread가 request를 dedicated thread로 전달합니다.

#### Hint for implementation (구현 힌트)

Linux처럼 virtual memory subsystem(가상 메모리 하위 시스템)을 통해 이 functionality(기능)를 구현할 수 있습니다. 새로운 vm type(예: "pagecache")을 도입하고 `swap_in`, `swap_out`, `destroy` interface(인터페이스)를 통해 buffer cache를 manage(관리)할 수 있습니다. 이 design(설계)을 선택하면 SECTOR_SIZE granularity(섹터 크기 단위)가 아니라 PAGE_SIZE granularity(페이지 크기 단위)로 cache를 manage할 수 있습니다. file backed page(파일 기반 페이지)의 synchronization issue(동기화 문제)에 대해서는 mmaped area(매핑된 영역)를 cache로 사용할 수 있습니다.

**cache를 design 초기에 integrate(통합)하는 것을 권장합니다.** 과거에는 많은 group(그룹)이 design process(설계 과정) 후반에 cache를 덧붙이려고 했습니다. 이는 매우 어렵습니다. 그런 group들은 대부분 또는 모든 test에 fail하는 project를 제출하는 경우가 많았습니다.
