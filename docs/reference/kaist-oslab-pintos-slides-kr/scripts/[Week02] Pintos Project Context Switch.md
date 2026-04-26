# [Week02] Pintos Project Context Switch

Source: https://youtu.be/mtX-bj1Fu6M?si=vCRv4Xu8zDLXLfbX

## Intro

이번 강의에서는 Pintos 운영체제의 context switch(문맥 전환) 세부 사항을 설명합니다.

강의의 개요는 다음과 같습니다. process structure, process state, process context, 그리고 두 thread를 scheduling하고 switching하는 과정을 다룹니다.

## Process Structure: `struct thread`

먼저 다룰 주제는 process를 나타내는 구조체인 `struct thread`입니다. 이는 운영체제 커널에서 가장 중요한 data structure 중 하나입니다.

이것이 `struct thread`입니다. thread를 표현하는 데 필요한 여러 attribute를 포함합니다. 첫 번째는 thread id이고, 그다음은 thread의 status, thread priority입니다. 이어서 kernel stack을 가리키는 pointer가 있는데, 이것은 뒤에서 설명합니다. 그리고 이 thread가 사용하는 page table을 가리키는 pointer도 포함합니다.

## Process State

Pintos 운영체제에는 네 가지 process state가 있습니다. running, ready, blocked가 기본 세 가지 상태입니다. 여기에 dying이라는 또 다른 상태가 있습니다. dying은 process가 exit를 호출한 뒤, 운영체제가 그 process에 할당된 모든 data structure를 완전히 제거하기 전까지 머무르는 상태입니다.

thread에서 가장 중요한 함수는 thread를 생성하는 함수입니다. 이것이 `thread_create()`의 prototype입니다. 네 가지 attribute를 받습니다. name, thread priority, thread가 호출될 때 실행할 function, 그리고 그 function에 전달해야 하는 parameter list입니다.

thread 생성의 세부 과정은 다음과 같습니다. `thread_create()` 안에서는 `init_thread()`라는 함수를 호출합니다. `memset()`에서는 `struct thread`를 위한 memory를 allocate하고 0으로 설정합니다. 구조체를 allocate한 뒤, 운영체제는 thread의 initial state를 blocked로 설정합니다. 그리고 방금 생성한 thread structure를 뒤에서 설명할 all list라는 list에 넣습니다.

## Process List

Pintos에는 process list 또는 thread list가 두 개 있습니다. 첫 번째는 ready list이고, 두 번째는 all list입니다.

Ready list는 실행 준비가 된 thread들의 집합입니다. All list는 시스템 안의 모든 thread들의 집합입니다. `thread_create()`에서는 thread가 먼저 all list에 삽입되고, CPU에서 실행될 준비가 된 thread들은 ready list에 삽입됩니다.

## Creating a Thread

thread 생성은 먼저 생성된 thread structure를 all list에 삽입하는 것으로 시작합니다. all list에 넣은 뒤, 운영체제는 thread structure의 여러 field를 초기화하고, thread state를 ready로 설정한 다음 ready list에 넣습니다.

현재 Pintos 운영체제에서는 운영체제가 thread structure를 ready list나 all list에 삽입할 때마다 `list_push_back()`을 호출합니다. 이는 운영체제가 방금 생성한 thread를 항상 list의 맨 뒤에 둔다는 뜻입니다.

이제 `schedule()` 함수를 설명하겠습니다. 이 함수는 매우 중요합니다. 운영체제가 새 thread를 schedule해야 할 때 호출하는 함수입니다.

새 thread를 scheduling한다는 것은 운영체제가 현재 실행 중인 thread를 CPU에서 내보내고, 다음에 실행할 thread를 찾아 CPU에 올린다는 뜻입니다. 그러면 누가 `schedule()`을 호출할까요? 이 질문은 process가 CPU를 어떻게 release하는가라는 질문과 같습니다.

이유는 두 가지입니다. 첫 번째는 voluntary release이고, 두 번째는 involuntary release입니다. voluntary release에서는 기존 process가 exit, block, 또는 yield를 호출합니다. involuntary switch에서는 현재 실행 중인 process가 자신의 의지와 상관없이 다른 process로 교체됩니다. 이는 더 높은 priority를 가진 process가 도착했거나, 현재 process가 사용하던 time quantum이 만료되었을 때 발생합니다.

`schedule()`을 호출하기 전에는 interrupt를 disable해야 하고, running thread의 state를 running에서 다른 상태로 바꿔야 합니다. 여기서 다른 상태란, `schedule()`이 block에서 호출되는 경우 status가 blocked가 된다는 뜻입니다.

`schedule()`이 호출되는 경우는 세 가지입니다. process가 exit할 때, process가 event를 기다리거나 block할 때, 그리고 process가 yield를 호출할 때입니다.

현재 Pintos 운영체제에서는 기존 process가 원하지 않는 한 운영체제가 기존 process를 switch out할 방법이 없습니다. 현재 Pintos 운영체제에는 preemption이 없습니다. 이것이 non-preemption입니다. 첫 번째 project를 진행하면서 이 운영체제를 preemptive operating system으로 바꾸게 됩니다.

Preemptible Pintos에서는 새로 들어온 process의 priority가 더 높으면 기존 process를 CPU에서 내보내는 scheduling module을 구현하게 됩니다.

이것은 Pintos의 실제 코드입니다. `thread_block()`입니다. 이 함수는 thread가 I/O completion을 요청할 때 호출되고, 그 안에서 `thread_block()`을 호출합니다. `thread_block()` 안에서는 process의 state를 `THREAD_BLOCKED`로 두고 `schedule()`을 호출합니다. scheduling 세부 사항은 뒤에서 다룰 것이므로 지금은 잠시 넘어가겠습니다.

두 번째 주제는 `thread_yield()`입니다. running process가 CPU를 다른 process에 넘기고 싶으면 yield를 호출합니다. yield에서는 현재 실행 중인 process가 자기 자신을 ready queue의 맨 뒤에 넣고 기존 CPU를 다른 process에게 넘깁니다. 이 경우 기존 process의 state는 `THREAD_READY`가 됩니다.

세 번째 경우는 process가 exit할 때입니다. process exit 이후 state를 `THREAD_DYING`으로 바꾼 다음 `schedule()`을 호출합니다.

이 세 가지가 `schedule()`이 호출되는 상황입니다. 이 모든 상태에서 process가 자발적으로 CPU를 relinquish한다는 점을 기억하세요.

## `schedule(void)`

이제 `schedule()` 함수의 세부 사항을 살펴봅시다.

이 함수는 current thread의 pointer를 얻은 다음, 다음에 실행할 thread의 pointer를 얻습니다. 이 지점에서 scheduling discipline이 개입합니다. 예를 들어 first-come, first-served, shortest job first, shortest time to completion algorithm, 또는 round robin을 사용할 수 있습니다.

이 모든 scheduling algorithm에서 운영체제는 다음에 실행할 thread를 선택합니다. `next_thread_to_run()`에서 운영체제는 다음에 실행할 thread를 선택하고, 방금 선택한 thread structure의 pointer를 반환합니다.

현재 running thread와 다음에 실행할 thread의 pointer를 얻은 뒤, `switch_threads()`라는 함수를 호출합니다. `switch_threads()`는 현재 context를 저장하고, 다음에 실행할 thread의 next context를 CPU register에 복구하는 역할을 합니다. 그리고 `prev`라는 변수에 thread structure pointer를 반환합니다.

`prev`로 return된 뒤, 운영체제는 `thread_schedule_tail()`을 호출합니다. 이 함수는 `prev`가 가리키는 thread structure를 ready list의 맨 뒤에 넣습니다.

## Stack

이제 thread switch의 세부 사항을 설명하겠습니다. 첫 단계로 stack이 무엇인지 설명합니다.

Stack은 매우 기본적인 data structure입니다. push와 pop operation을 가집니다. item을 stack에 push하면 stack top pointer가 증가합니다. data를 stack에서 pop하면 pointer는 반대쪽으로 이동합니다.

중요한 register가 하나 있습니다. 우리는 이것을 `esp`, 즉 stack pointer라고 부르며, stack의 top을 가리킵니다. 보통 stack은 낮은 address space 방향으로 자랍니다.

`push abc`는 `abc`가 가리키는 address의 값을 stack top에 저장하고 stack pointer를 4 증가시킵니다. 하지만 stack pointer를 4 증가시킨다는 것은 `esp` 값이 4 byte 감소하는 것에 해당합니다.

다음 operation은 `pop abc`입니다. `esp` pointer가 가리키는 값을 가져와 `abc`가 가리키는 위치에 저장합니다. 그런 다음 stack pointer를 4 byte 감소시킵니다. 이것이 pop입니다. Push와 pop은 stack의 두 가지 기본 operation입니다.

process의 virtual address space는 user space와 kernel space라는 두 configured region으로 나뉩니다. Pintos에서 virtual address space 0부터 3GB까지는 user space이고, 3GB를 넘어서는 virtual space는 kernel address space입니다.

kernel address space에 접근하려면 process가 execution mode를 kernel mode로 바꿔야 합니다. 이는 privilege level을 높여서 이루어집니다.

process가 user mode에서 실행될 때는 function call과 local variable 정의에 user stack을 사용합니다. kernel로 switch되어 kernel에서 실행될 때는 function call에 kernel stack을 사용합니다. 즉 kernel stack과 user stack이 있습니다.

## `switch_threads(struct thread *cur, struct thread *next)`

이제 `switch_threads()`의 세부 단계를 설명하겠습니다.

`switch_threads()` 함수는 `cur`와 `next`가 가리키는 두 thread를 switch합니다. `cur`는 현재 실행 중인 thread를 나타내고, `next`는 실행할 thread를 나타냅니다.

기본적으로 네 단계로 구성됩니다. 먼저 register를 kernel stack에 저장합니다. 그다음 current thread의 stack attribute에 현재 stack top의 위치를 저장합니다. 그런 다음 새 thread의 stack top을 CPU stack pointer로 복구합니다. 방금 새로 설정한 stack pointer를 사용해 stack에서 register를 CPU로 복구합니다.

## Call `switch_threads()`

이 그림은 `switch_threads()`를 호출한 직후의 current stack과 current stack top register 값을 나타냅니다. `cur`와 `next`라는 두 pointer가 있습니다. 이들은 현재 실행 중인 thread와 다음에 실행될 thread의 thread structure를 가리킵니다.

그리고 두 stack이 있습니다. 이것은 current thread의 kernel stack이고, 다른 하나는 next thread의 kernel stack입니다. 둘 다 kernel stack region에 있습니다. stack pointer `esp`는 current stack의 stack top을 가리킵니다.

이것은 `switch_threads()`가 호출되는 바로 그 시점의 memory layout 상태입니다. 실행은 곧바로 `switch_threads()` 함수로 jump합니다.

`switch_threads()`로 jump한 뒤에는 current thread의 kernel stack에 CPU register 네 개를 저장합니다. `ebx`, `ebp`, `esi`, `edi` register를 push한 뒤, stack pointer는 새 stack top으로 증가합니다. 네 register를 push하기 위해 `switch_threads()` 함수는 `ebx`, `ebp`, `esi`, `edi`에 대한 네 instruction을 실행합니다.

이 네 instruction을 실행하면 현재 실행 중인 thread의 네 register가 stack top, 즉 current kernel stack에 push되고, kernel stack은 새로 설정된 stack top을 가리키도록 바뀝니다.

네 register를 kernel stack에 저장한 뒤에는 현재 실행 중인 thread의 thread structure에 kernel stack-top address를 저장할 차례입니다.

이 그림에서 stack attribute는 kernel stack의 stack top을 가리키도록 설정됩니다. 이 attribute는 `esp` register의 stack top address를 담습니다.

먼저 해야 할 일은 stack attribute의 offset을 `edx` register에 load하는 것입니다. Stack attribute는 thread structure의 시작점에서 24 byte 떨어져 있으므로, 먼저 `struct thread`의 시작점에서 얼마만큼 jump해야 하는지 offset을 알아야 합니다. 이 값은 이 statement로 지정되어 있고, macro `thread_stack_ofs`는 이 thread structure 시작점에서 stack attribute까지의 offset을 나타냅니다. 이 값은 24입니다. 이 statement를 실행한 뒤 `edx`에는 24라는 값이 들어갑니다.

다음 단계는 current `struct thread`의 시작 address를 `eax` register에 load하는 것입니다. 운영체제가 다음으로 하는 일은 current `struct thread`의 위치를 `eax` register에 저장하는 것입니다. 그런데 current `struct thread`의 현재 위치를 어떻게 알 수 있을까요?

`cur`의 stack을 보면 current thread structure의 위치를 저장하는 `cur` field가 있고, 이것은 `esp` structure에서 20 byte 떨어져 있습니다. `switch_threads()` call 기준으로 이 값은 20을 나타냅니다. 이 지점에서 20 byte 위로 올라가면 그 위치에 current `struct thread` data structure의 address가 들어 있습니다. 이 statement를 실행하면 `eax` register에는 current thread structure의 address가 들어갑니다.

다음 단계로 current thread의 stack top address를 thread의 stack field에 저장합니다. stack pointer인 `esp`의 값을 이 address로 옮깁니다. 여기서 `eax`는 이 `struct thread` data structure의 시작점을 가리키고, `edx`는 thread structure의 시작점과 stack 위치 사이의 offset을 나타냅니다. 그곳에 `esp` 값을 저장합니다.

그 결과 pointer, 즉 stack field는 current stack top의 address를 담게 됩니다. 모양은 다음과 같습니다.

## Switch Kernel Stack

다음 단계는 stack pointer를 next thread structure의 stack top을 가리키도록 switch하는 것입니다.

stack pointer를 current thread의 stack top에서 next thread의 stack top으로 switch하는 과정은 두 단계로 이루어집니다. 먼저 next thread의 thread structure 위치를 식별해야 합니다. 이를 위해 이 지점에서 24 byte 위로 올라가 next thread structure의 위치를 찾습니다.

next thread structure의 위치를 식별한 뒤, thread structure의 시작 address와 stack이 위치한 곳 사이의 offset을 식별합니다. 이 값은 24 byte입니다. 그다음 thread structure에서 stack pointer의 위치를 가져올 수 있습니다. 그러면 stack top pointer가 current thread에서 new thread로 성공적으로 바뀐 것입니다.

data structure를 보면, next thread structure의 위치를 식별하는 첫 instruction은 stack pointer에서 24 byte 위로 올라갑니다. 이곳이 next thread structure의 위치이며, `switch_threads()`의 caller가 제공한 next thread의 thread structure address를 담고 있습니다.

next thread structure의 위치를 `ecx`에 load한 뒤, stack field의 offset을 이 base location에 더하고, 그 address에 있는 address value를 `esp`에 저장합니다. 그러면 `esp`는 새 stack top, 즉 next thread의 stack top을 가리킵니다.

## Restore the New Context

stack pointer를 새 stack top으로 switch한 뒤, 네 register 값을 pop해서 `edi`, `esi`, `ebp`, `ebx` 네 register로 복구합니다. 네 register 값을 pop한 결과 stack pointer는 이 위치로 update됩니다.

네 pop instruction을 수행하면 next thread의 kernel stack이 update됩니다. 이것이 두 thread를 switch하는 방식입니다.

current thread와 next thread를 switch한 뒤, 운영체제는 새로 선택된 thread의 state를 running으로 update합니다. previous thread가 dying state였다면, 운영체제는 그 thread에 allocate된 모든 page를 clean up해야 합니다.

## Change the State of New Current

이것은 `thread_schedule_tail()` 코드의 세부 내용입니다. 이 함수는 새로 running 상태가 된 thread를 `THREAD_RUNNING`으로 update합니다. `switch_threads()`의 caller가 dying state였다면, dying state process에 allocate된 모든 page를 deallocate하고 free해야 합니다.

## Summary

이 영상에서는 Pintos 운영체제의 context switch 세부 사항을 설명했습니다.

`schedule()` 함수는 exit, yield, block에서 호출되며, 새 process를 CPU에 올립니다.

context switch에는 네 가지 중요한 단계가 있습니다. 현재 실행 중인 thread의 context를 stack에 저장하고, 현재 stack top을 현재 실행 중인 `struct thread`에 저장합니다. 그런 다음 stack top register를 next thread의 stack top을 가리키도록 switch하고, stack에서 context를 CPU로 복구합니다.

두 thread를 switch한 뒤에는 다음 running process의 state를 update하고, dying process의 memory를 free합니다.
