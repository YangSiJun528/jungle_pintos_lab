# FAQ

### Do we need a working Project 2 to implement Project 3? (Project 3을 구현하려면 동작하는 Project 2가 필요한가요?)

네.

### How do we resume a process after we have handled a page fault? (page fault를 처리한 뒤 process는 어떻게 재개하나요?)

`page_fault()`에서 return(반환)하면 현재 user process(사용자 프로세스)가 resume(재개)됩니다. 그러면 instruction pointer(명령어 포인터)가 가리키는 instruction(명령어)을 다시 시도합니다. user process가 왜 때때로 stack pointer(스택 포인터)보다 높은 주소에서 fault(폴트)를 일으키는지 궁금할 수 있습니다. stack growth test(스택 증가 테스트)에서 user program이 현재 stack pointer보다 높은 address(주소)에서 fault를 일으키는 것을 볼 수 있습니다.

### Does the virtual memory system need to support data segment growth? (virtual memory system이 data segment 증가를 지원해야 하나요?)

아니요. data segment(데이터 세그먼트)의 크기는 linker(링커)가 결정합니다. Pintos에는 여전히 dynamic allocation(동적 할당)이 없습니다. memory-mapped file(메모리 매핑 파일)을 사용해 user level(사용자 수준)에서 "fake(흉내)"낼 수는 있습니다. 잘 설계된 system이라면 data segment growth(데이터 세그먼트 증가)를 지원하는 데 큰 additional complexity(추가 복잡도)가 들지 않아야 합니다.

### Why should I use `PAL_USER` for allocating page frames? (page frame 할당에 왜 `PAL_USER`를 사용해야 하나요?)

`palloc_get_page()`에 `PAL_USER`를 전달하면 main kernel pool(주 커널 풀) 대신 user pool(사용자 풀)에서 memory(메모리)를 allocate(할당)합니다. user pool의 page가 부족해지면 user program이 page를 발생시키는 정도로 끝나지만, kernel pool의 page가 부족해지면 많은 kernel function(커널 함수)이 memory를 얻어야 하므로 여러 failure(실패)가 발생합니다. 원한다면 `palloc_get_page()` 위에 다른 allocator(할당자)를 layer(계층)로 둘 수 있지만, underlying mechanism(기반 메커니즘)은 이것이어야 합니다. 또한 `-ul` kernel command-line option(커널 명령줄 옵션)을 사용하여 user pool 크기를 제한할 수 있으며, 이는 다양한 user memory size(사용자 메모리 크기)에서 VM implementation(가상 메모리 구현)을 test하기 쉽게 합니다.
