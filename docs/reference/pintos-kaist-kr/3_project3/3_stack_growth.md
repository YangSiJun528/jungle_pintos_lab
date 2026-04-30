### Stack Growth (스택 증가)

project 2에서 stack(스택)은 `USER_STACK`에서 시작하는 single page(단일 페이지)였고, program execution(프로그램 실행)은 이 크기로 제한되었습니다. 이제 stack이 현재 크기를 넘어 grow(증가)하면 필요에 따라 additional page(추가 페이지)를 allocate(할당)합니다.

추가 page는 stack access(스택 접근)처럼 "보이는" 경우에만 allocate하세요. stack access와 다른 access를 구분하려고 시도하는 heuristic(휴리스틱, 경험적 판단 규칙)을 고안하세요.

user program(사용자 프로그램)이 stack pointer(스택 포인터) 아래의 stack에 write(쓰기)하면 bug(결함)가 있는 것입니다. 일반적인 실제 OS(운영체제)는 process(프로세스)를 언제든 interrupt(인터럽트)하여 stack의 data(데이터)를 수정하는 "signal(시그널)"을 전달할 수 있기 때문입니다. 하지만 x86-64 PUSH instruction(명령어)은 stack pointer를 조정하기 전에 access permission(접근 권한)을 check(확인)하므로, stack pointer보다 8 bytes 아래에서 page fault(페이지 폴트)를 일으킬 수 있습니다.

user program의 stack pointer 현재 값을 얻을 수 있어야 합니다. user program이 생성한 system call(시스템 콜)이나 page fault 안에서는 각각 `syscall_handler()` 또는 `page_fault()`에 전달된 `struct intr_frame`의 `rsp` member(멤버)에서 가져올 수 있습니다. invalid memory access(잘못된 메모리 접근)를 감지하기 위해 page fault에 의존한다면, kernel(운영체제 핵심부)에서 page fault가 발생하는 또 다른 case(경우)를 처리해야 합니다. processor(프로세서)는 exception(예외)이 user mode(사용자 모드)에서 kernel mode(커널 모드)로 switch(전환)를 일으킬 때에만 stack pointer를 save(저장)하므로, `page_fault()`에 전달된 `struct intr_frame`에서 `rsp`를 읽으면 user stack pointer가 아니라 undefined value(정의되지 않은 값)가 나옵니다. user에서 kernel mode로 처음 transition(전이)할 때 `struct thread`에 rsp를 저장하는 등의 다른 방법을 마련해야 합니다.

**stack growth functionality(스택 증가 기능)를 구현하세요.** 이를 구현하려면 먼저 `vm/vm.c`의 `vm_try_handle_fault`를 수정하여 stack growth를 identify(식별)합니다. stack growth를 identify한 뒤에는 `vm/vm.c`의 `vm_stack_growth`를 호출하여 stack을 grow해야 합니다. `vm_stack_growth`를 구현하세요.

* * *
    
    
    bool vm_try_handle_fault (struct intr_frame *f, void *addr,
        bool user, bool write, bool not_present);
    

> 이 function은 page fault exception(페이지 폴트 예외)을 처리하는 동안 `userprog/exception.c`의 `page_fault`에서 호출됩니다. 이 function 안에서 page fault가 stack growth에 valid case(유효한 경우)인지 아닌지 check해야 합니다. fault를 stack growth로 handle(처리)할 수 있다고 확인했다면 faulted address(폴트가 난 주소)로 `vm_stack_growth`를 호출하세요.

* * *
    
    
    void vm_stack_growth (void *addr);
    

> `addr`이 더 이상 faulted address가 아니도록 하나 이상의 anonymous page(익명 페이지)를 allocate하여 stack size(스택 크기)를 늘립니다. allocation을 처리할 때 `addr`을 PGSIZE로 round down(내림 정렬)해야 합니다.

대부분의 OS는 stack size에 절대적인 limit(제한)을 둡니다. 일부 OS에서는 많은 Unix system의 `ulimit` command(명령)처럼 user-adjustable(사용자가 조정 가능)한 limit을 둡니다. 많은 GNU/Linux system에서 기본 limit은 8 MB입니다. 이 project에서는 stack size를 최대 1MB로 제한해야 합니다.

이제 모든 stack-growth test case가 통과해야 합니다.
