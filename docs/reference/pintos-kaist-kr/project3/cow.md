## Copy-on-write (Extra) (쓰기 시 복사)

**Pintos에서 copy-on-write mechanism(쓰기 시 복사 메커니즘)을 구현하세요.**

Copy-on-write는 같은 physical page(물리 페이지)의 instance(인스턴스)를 사용하여 duplication operation(복제 작업)을 더 빠르게 만드는 resource management technique(자원 관리 기법)입니다. 어떤 resource(자원)를 여러 process(프로세스)가 사용 중이라면, 보통 conflict(충돌)가 발생하지 않도록 각 process가 그 resource의 own copy(자기 복사본)를 가져야 합니다. 하지만 resource가 modify(수정)되지 않고 read(읽기)만 된다면 physical memory(물리 메모리)에 여러 copy를 둘 필요가 없습니다.

예를 들어 `fork`를 통해 새 process가 생성된다고 합시다. child(자식)는 parent(부모)의 resource를 inherit(상속)해야 하므로 data(데이터)를 자신의 virtual address space(가상 주소 공간)에 duplicate(복제)합니다. 보통 virtual memory(가상 메모리)에 content(내용)를 추가하려면 physical page를 allocate(할당)하고, data를 frame(프레임)에 write(쓰기)하고, page table(페이지 테이블)에 virtual->physical mapping(가상-물리 매핑)을 추가해야 합니다. 이 단계들은 꽤 time-consuming(시간이 많이 듦)할 수 있습니다.

하지만 copy-on-write technique(기법)을 사용하면 resource의 새 copy를 위해 새 physical page를 allocate하지 않습니다. 기술적으로 content가 이미 physical memory에 존재하기 때문입니다. 따라서 child process의 page table에 virtual->physical mapping만 추가합니다. 이때 virtual address는 이제 child의 memory space 안에 있습니다. 그러면 parent와 child는 같은 physical page의 같은 data에 접근합니다. 하지만 여전히 separate virtual address space(분리된 가상 주소 공간)를 통해 isolate(격리)되어 있고, 이들이 같은 frame을 refer(참조)한다는 것은 OS만 알고 있습니다. process 중 하나가 shared resource(공유 자원)의 content를 modify하려고 할 때에만, 자신을 위해 새 physical page에 별도 copy를 만듭니다. 따라서 실제 copy operation은 첫 write까지 deferred(지연)됩니다.

즉 OS는 copy-on-write page에 대한 write attempt(쓰기 시도)를 detect(감지)할 수 있어야 합니다. 이를 위해 OS는 "write-protect(쓰기 보호)" mechanism을 사용합니다. 아이디어는 단순합니다. write access(쓰기 접근)에서 page fault(페이지 폴트)를 만들면 됩니다. memory management system(메모리 관리 시스템)의 지원으로, write-protected page를 아예 unwritable(쓰기 불가)로 mark(표시)하면 쉽게 구현할 수 있습니다.

**`fork`에 대해서만 copy-on-write를 구현하면 됩니다.** child process가 parent process의 resource를 inherit할 때, child가 이를 modify하려고 하기 전까지 resource는 같은 physical data를 reference(참조)할 수 있습니다. **모든 write-protected page는 eviction(축출) 후보입니다.**

copy-on-write에 대해서는 basic test case(기본 테스트 사례)만 제공합니다. 가능한 모든 case를 고려해야 합니다. 작은 hint(힌트): file-backed page(파일 기반 페이지)의 sharing(공유)을 구현해야 합니다. 이 extra project의 grading(채점)은 hidden test case(숨겨진 테스트 사례)로도 수행됩니다.
