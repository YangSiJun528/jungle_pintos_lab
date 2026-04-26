### User Memory Access (사용자 메모리 접근)

**user memory access(사용자 메모리 접근)를 구현하세요.**

syscall(시스템 콜)을 구현하려면 user virtual address space(사용자 가상 주소 공간)의 data(데이터)를 read(읽기)하고 write(쓰기)할 방법을 제공해야 합니다. argument(인자)를 가져올 때에는 이 능력이 필요하지 않습니다. 하지만 system call의 argument로 제공된 pointer(포인터)에서 data를 읽을 때에는 이 functionality(기능)를 통해 중계해야 합니다. 이 부분은 조금 까다로울 수 있습니다. user가 invalid pointer(잘못된 포인터), kernel memory(커널 메모리)를 가리키는 pointer, 또는 일부가 그런 region(영역)에 걸쳐 있는 block(블록)을 제공하면 어떻게 될까요? 이런 경우에는 user process(사용자 프로세스)를 terminate(종료)하여 처리해야 합니다.
