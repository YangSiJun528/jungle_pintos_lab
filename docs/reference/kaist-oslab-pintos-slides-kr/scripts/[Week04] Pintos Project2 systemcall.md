# [Week04] Pintos Project2 systemcall

Source: https://youtu.be/sBFJwVeAwEk?si=51sBUCoYdZZs9tQ9

## Intro

다음 주제는 Pintos에서 system call을 구현하는 방법입니다. 현재 Pintos의 system call handler는 비어 있습니다. 수정 후에는 system call handler를 채우고, user program이 사용할 수 있도록 system call을 구현해야 합니다.

구현해야 하는 system call에는 `halt`와 process, file 관련 함수들이 포함됩니다. 각 function의 세부 내용은 뒤에서 더 자세히 다룹니다.

system call은 operating system이 제공하는 service에 접근하기 위한 programming interface입니다. user program이 process 생성, file 접근, file 저장 같은 kernel feature를 사용할 수 있게 해 줍니다.

system call은 kernel mode에서 실행됩니다. KAIST Pintos에서 user program은 `syscall` instruction을 invoke하고, system call number와 argument는 register를 통해 전달됩니다. system call의 핵심은 trap을 발생시켜 execution mode의 privilege level을 높인다는 점입니다.

user program에서 process 생성이나 file 저장 같은 kernel service를 사용하고 싶다면 system call을 호출해서 operating system에 작업을 요청합니다. system call이 완료되면 control은 다시 user program으로 돌아갑니다. 중요한 점은 system call이 실행되는 동안 privilege level이 올라간다는 것입니다.

## Call Process of System Call in Pintos

Pintos operating system에서 system call이 호출되는 과정은 다음과 같습니다. user가 `write`를 호출하면 C file 안의 wrapper function이 실행됩니다. 이 wrapper에는 `write`의 body가 있고, 내부적으로 `syscall3` 같은 helper를 호출합니다.

`syscall3`는 parameter가 세 개인 system call이라는 뜻입니다. KAIST Pintos에서는 system call number를 `%rax`에 두고, argument들을 register에 배치한 다음 `syscall` instruction을 실행합니다.

kernel entry path는 `syscall_handler()`를 호출합니다. 이 function은 현재 비어 있습니다. system call을 구현하려면 이 부분을 채워야 합니다.

각 system call은 자기 number를 가집니다. system call number는 `include/lib/syscall-nr.h`에 정의되어 있습니다. 예를 들어 `SYS_HALT`는 system call number 0에 대응하고, `SYS_EXIT`는 system call number 1에 대응합니다.

## Requirement for System Call Handler

첫 번째 요구사항은 system call handler가 system call number를 사용해 올바른 system call을 호출해야 한다는 것입니다.

system call handler에서 가장 중요한 부분은 user program이 전달한 address의 validity를 확인하는 것입니다. system call이 pointer parameter를 받는다고 해 봅시다. kernel 안에서 operating system은 그 address에 접근하려 할 수 있습니다. address가 invalid하다면 어떤 일이 일어날지 알 수 없습니다.

실제 system call body를 호출하기 전에, operating system 또는 system call handler는 user program이 전달한 address가 올바른 address를 가리키는지 확인해야 합니다.

pointer는 kernel area가 아니라 user area를 가리켜야 합니다. virtual address space는 user space와 kernel space로 나뉩니다. user가 system call에 전달한 pointer는 kernel space가 아니라 user space 어딘가를 가리켜야 합니다.

이 pointer들이 valid address를 가리키지 않으면 page fault가 발생할 수 있습니다. address validation이 끝난 뒤, kernel은 필요할 때 user memory의 data를 kernel로 copy합니다. system call 실행 후 return value는 `struct intr_frame`의 `rax` member를 통해 `%rax` register에 저장됩니다.

중요한 rule은 user가 kernel에 valid user address를 전달해야 한다는 것입니다. kernel은 실행 중에 user address space에 직접 의존하는 것을 피해야 합니다. 대신 parameter를 kernel의 address space로 copy해서, user space에 반복해서 접근하지 않고 kernel 안에서 program을 실행할 수 있게 해야 합니다.

## Address Validation

user는 system call을 통해 invalid pointer를 전달할 수 있습니다. null pointer, unmapped virtual memory를 가리키는 pointer, kernel address space를 가리키는 pointer를 전달할 수 있습니다. 이런 경우 operating system은 program을 종료해야 합니다.

user가 system call을 호출할 때, kernel은 invalid pointer를 detect하고 kernel이나 다른 running process에 피해를 주지 않으면서 그 process를 terminate해야 합니다.

address validity를 detect하는 방법은 두 가지가 있습니다.

첫 번째 방법은 user-provided pointer의 validity를 명시적으로 확인하는 것입니다. page table을 확인하고 주어진 모든 address가 mapped되어 있는지 확인해야 합니다. KAIST Pintos에서는 `threads/mmu.c`의 `pml4_get_page()` 같은 함수와 `include/threads/vaddr.h`의 address helper를 사용합니다.

두 번째 방법은 user pointer가 `KERN_BASE` 아래인지 확인하는 것입니다. 이 경우 operating system은 그 address가 mapped address를 참조하는지 직접 확인하지 않습니다. 주어진 user pointer가 `KERN_BASE`보다 낮은지만 확인합니다. user-supplied pointer가 invalid하면 page fault가 발생하고, `page_fault()`를 수정해 이를 처리합니다.

두 번째 approach는 kernel이 모든 것을 명시적으로 확인할 필요가 없으므로 더 빠릅니다. user address의 validity를 확인하는 데 hardware memory protection을 활용합니다. Linux 같은 실제 operating system에서도 이런 approach가 널리 사용됩니다.

## Accessing User Memory

process가 lock을 잡았거나 `malloc()`으로 memory를 allocate한 뒤 작업 중 page fault가 발생하는 예를 생각해 봅시다. process는 죽지만 global lock이 여전히 잡힌 채로 남거나, allocate된 memory가 free되지 않을 수 있습니다. 따라서 page fault는 resource leakage를 일으킬 수 있습니다.

process를 terminate하기 전에 operating system은 resource leakage를 피하기 위해 lock을 release하거나 allocated memory를 반환해야 합니다.

이런 leakage를 처리하는 방식은 두 validation method에서 다릅니다. 첫 번째 method에서는 operating system이 lock을 잡거나 resource를 allocate하기 전에 모든 것을 확인하므로 비교적 straightforward합니다. system call 안에서는 pointer validity를 확인한 뒤에만 lock을 잡거나 memory를 allocate합니다.

두 번째 method는 invalid memory access에서 일반적인 error code가 return되지 않기 때문에 더 어렵습니다. Pintos는 이런 case를 처리하기 위해 helper function을 제공합니다.

- `get_user()`는 지정된 user address에서 byte를 읽습니다.
- `put_user()`는 지정된 user address에 byte 하나를 씁니다.

## Add System Calls: Process Related System Calls

다음은 구현해야 할 process-related system call입니다.

`halt`는 operating system을 shutdown합니다. Pintos는 `halt`가 호출된 경우를 제외하고는 shutdown되어서는 안 됩니다.

`exit`는 process를 exit합니다. process가 exit할 때는 process name과 exit status를 print해야 합니다.

`fork`는 child process를 생성합니다. `exec`는 현재 process를 command line에 주어진 executable로 변경하고 argument를 전달합니다. KAIST Pintos에서 `exec` 자체는 child process를 생성하지 않습니다.

`wait`는 process ID가 `pid`인 child process의 termination을 기다립니다.

## Process Hierarchy

이 project에서는 process hierarchy라는 개념을 도입해야 합니다. parent process, 그 parent가 생성한 child process들, 그리고 sibling을 가리키는 pointer를 지정해야 합니다.

구현해야 할 중요한 concept은 두 가지입니다. 첫째, thread structure에 parent process를 가리키는 pointer를 추가합니다. 둘째, child process들을 doubly linked list로 유지할 수 있도록 sibling pointer를 추가합니다.

children을 가리키는 pointer를 구현하는 방법은 여러 가지입니다. 하지만 parent 안에 모든 child에 대한 pointer를 직접 저장하는 것은 합리적이지 않습니다. 보통 child 또는 sibling list의 head와 tail을 유지할 수 있습니다. sibling list는 생성된 순서, 즉 oldest에서 youngest 순서로 유지할 수 있습니다.

## `wait`

`wait` system call은 child process가 exit할 때까지 기다리고 child의 exit status를 받습니다. `pid`로 식별되는 process가 아직 살아 있다면, `wait`는 그 process가 terminate될 때까지 block합니다. 그런 다음 child가 `exit`에 전달한 status를 return합니다.

`pid`로 식별되는 process가 `exit`를 호출하지 않고 kernel에 의해 terminate되었다면, `wait`는 `-1`을 return합니다.

parent process가 이미 terminate된 child process에 대해 `wait`를 호출할 수도 있습니다. 이 경우 terminated process의 exit status를 return합니다. 중요한 점은 child가 terminate된 뒤 parent가 그 process descriptor를 deallocate해야 한다는 것입니다.

`wait`는 process ID가 calling process의 direct child를 가리키지 않으면 fail합니다. 또한 calling process가 같은 `pid`에 대해 이미 `wait`를 호출한 적이 있어도 fail합니다.

현재 `process_wait()`의 형태는 구현되어야 합니다. 구현이 쉽지는 않으므로 초기 단계에서는 여기에 infinite loop를 넣을 수 있지만, 이것이 올바른 구현은 아닙니다.

## Correct Implementation of `process_wait()`

`process_wait()`의 올바른 구현은 parameter를 사용해 child process의 descriptor를 찾습니다. caller는 child process가 exit할 때까지 block합니다. child가 exit하면 parent는 child process descriptor를 deallocate하고 exit status를 return합니다.

`process_wait()`의 caller와 child thread의 execution을 어떻게 synchronize할 수 있을까요? thread structure에 wait semaphore를 추가합니다. 이 semaphore는 thread가 처음 생성될 때 0으로 initialize됩니다.

`wait` system call에서 caller는 child의 wait semaphore에 대해 `sema_down()`을 호출합니다. child process의 exit path에서는 child가 `sema_up()`을 호출합니다. `sema_up()`은 semaphore value를 1 증가시키고, `sema_down()`은 semaphore value를 1 감소시킵니다.

thread가 생성될 때 semaphore value가 0으로 initialize되므로, caller가 `wait`를 호출하면 block됩니다. child process가 `exit`를 호출하면 semaphore value를 1 증가시키고, waiting process가 wake up되어 계속 진행할 수 있습니다.

`exit`에서는 process가 자신의 exit status도 return해야 합니다. 따라서 `struct thread`에 thread의 exit status를 저장하는 field를 추가해야 합니다.

## Flow of Parent Calling `wait` and Child

control flow는 semaphore를 다음과 같이 사용합니다. `process_wait()`에서 parent가 `sema_down()`을 호출합니다. `exit`에서 child가 `sema_up()`을 호출합니다.

`sema_down()`을 호출하면 caller는 child process가 finish될 때까지 block됩니다. user process가 `exit`를 호출하면 `sema_up()`을 호출하므로, semaphore value가 증가하기를 기다리던 process가 unblock됩니다.

## `exec()` System Call

KAIST Pintos에서 `exec`는 current process를 새 executable로 변경하고 그 binary를 실행합니다.

`exec` system call은 command-line argument를 받고 요청된 program을 실행합니다. 실행할 program에 argument를 전달합니다. 성공하면 return하지 않으며, load에 실패하면 process는 exit status `-1`로 terminate됩니다.

`fork`의 경우 parent는 child가 resource duplication에 성공했는지 알기 전까지 return해서는 안 됩니다. `exec`의 경우 current process가 executable을 load하고 그 program으로 switch합니다.

## Kernel Function for `exec()`: `process_exec()`

`process_exec()`는 `exec`를 위한 kernel function입니다. 이 함수는 current process를 위한 binary file을 load하고, loaded program으로 execution을 switch합니다.

`fork`를 통한 child creation에서는 caller와 child process를 synchronize하기 위해 semaphore를 사용할 수 있습니다. child setup이 끝날 때까지 기다려야 한다면 thread structure에 semaphore를 추가할 수 있습니다.

child setup에서 parent는 successful setup을 기다리기 위해 `sema_down()`을 호출할 수 있고, child는 setup이 끝났을 때 `sema_up()`을 호출할 수 있습니다.

또한 load status를 나타내는 다른 variable이 필요합니다. 이 field는 file이 성공적으로 load되었는지 기록합니다.

## Current Flow of Parent Calling `exec` and Child

child creation의 경우 parent process가 creation path를 호출하고 새 thread가 실행되기 시작합니다. child path 안에서 child는 필요한 resource를 load하거나 duplicate합니다. parent process는 setup이 완료되었는지 확실해질 때까지 기다려야 합니다. 이후 parent와 child가 병렬로 계속 실행될 수 있습니다.

control flow는 다음과 같습니다. parent process가 계속 실행되다가 새 thread를 생성합니다. 새 thread는 execution을 계속하고 setup을 수행합니다. setup이 끝나면 `sema_up()`을 호출합니다. parent의 `sema_down()`이 return하면 두 process는 병렬로 계속 실행될 수 있습니다.

parent-child synchronization을 올바르게 구현하는 방식은 parent가 semaphore에서 기다리고, child setup이 끝난 뒤 child가 `sema_up()`을 호출하는 것입니다.

중요한 design issue는 `sema_down()`을 creation call 바로 옆에 둘지, creation helper 안에 둘지입니다.

## `exit`

`exit`에서는 주어진 process의 exit status를 return해야 합니다. process가 `exit`를 호출하면 operating system은 current user program을 terminate하고 status를 kernel에 return해야 합니다. parent process가 이를 wait하면 parent는 해당 thread의 exit status를 읽게 됩니다.

기존 `exit` body에는 exit status를 process descriptor 또는 `struct thread`에 저장하는 code를 추가해야 합니다. 또한 exit status와 thread name을 print한 뒤 `thread_exit()`을 호출해야 합니다.

## Kernel Function for `exit()`: `thread_exit()`

`thread_exit()`은 exit를 위한 common kernel function입니다. 여기서는 process status field에 status를 저장하고, 이 thread가 finish되기를 기다리던 process가 계속 진행할 수 있도록 semaphore를 signal해야 합니다.

`thread_exit()`에서는 interrupt가 disabled됩니다. thread list를 manipulate할 때는 언제나 interrupt를 disable해야 합니다. 그런 다음 Pintos는 thread list에서 node를 제거하고, thread status를 `THREAD_DYING`으로 바꾸고, CPU를 다른 process에 넘깁니다. 이 function은 다시 도달하지 않습니다.
