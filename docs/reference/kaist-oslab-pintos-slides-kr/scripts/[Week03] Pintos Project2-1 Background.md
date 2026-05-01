# [Week03] Pintos Project2-1 Background

Source: https://youtu.be/RbsE0EQ9_dY?si=JcKIZByXsQfrbEmk

## Intro

두 번째 project의 두 번째 부분에서는 Pintos가 program을 실행할 수 있게 만듭니다. Project 2의 목적은 user program을 실행하는 것입니다. 현재 Pintos는 user program을 실행할 수 없으므로, 아직 완전한 operating system이라고 보기 어렵습니다. 이 강의는 그 project를 위한 background입니다.

## Running a Program in Pintos

Pintos나 다른 operating system에서 command line은 process를 실행하기 위한 argument로 전달됩니다. KAIST Pintos tree에서는 첫 user process가 `process_create_initd()`와 `thread_create()`를 통해 생성되고, executable은 `process_exec()`에서 load됩니다.

문제는 현재 `process_wait()`을 호출하는 body가 단순히 return한다는 것입니다. `process_wait()` 함수는 child process가 생성될 때까지 기다리고, 그 process가 끝날 때까지 기다려야 합니다. 하지만 현재는 아무것도 하지 않고 단순히 return합니다.

그 결과 Pintos는 다음과 같이 동작합니다. Pintos에서 `init`은 process zero입니다. 모든 process의 어머니 같은 process이고, process ID는 0입니다. 이 process가 새 process를 만들고 schedule하지만, schedule한 직후 바로 exit합니다. 운영체제가 exit하면 다른 program은 실행될 수 없습니다. 현재 Pintos에는 process가 제대로 실행될 방법이 없습니다.

최종 목표는 Pintos가 child process의 completion을 기다린 다음 계속 실행하게 만드는 것입니다. 꽤 많은 작업이 필요하므로, 단계별로 살펴봅니다.

## Executing a Program

실행에서 중요한 함수는 `process_exec()`입니다. 이 함수는 executable file의 이름을 포함하는 file name 또는 command line을 받습니다. 예를 들어 `a.out`일 수 있고, `ls` command를 수행하려면 file-name parameter로 `ls`가 전달됩니다.

초기 user process의 경우 `process_create_initd()`가 `thread_create()`를 호출합니다. `thread_create()`를 호출하면 Pintos는 새 thread를 생성합니다. 그 순간 새 thread가 만들어지고 execution이 계속됩니다.

## Creating a Thread

`thread_create()`는 새 `struct thread`를 만들고 initialize하며, kernel stack을 allocate하고, 실행할 function을 register한 다음 thread를 ready list에 추가합니다. function을 register한다는 것은 instruction pointer 또는 instruction counter를 주어진 process의 entry point에 맞추는 것을 의미합니다.

높은 수준에서 코드는 단순합니다. `thread_create()`에는 thread의 name, 설정할 default priority, function name, auxiliary parameter가 필요합니다.

먼저 kernel space에 4 KB짜리 single page를 allocate합니다. 그런 다음 그 page 안에서 thread structure를 initialize합니다. thread structure는 64 bytes, 128 bytes 또는 structure size만큼의 크기를 차지할 수 있습니다. 이후 모든 thread가 unique ID를 가지므로 thread ID를 allocate합니다.

또한 kernel stack도 allocate합니다. kernel stack에는 실행하려는 function을 포함한 여러 field를 initialize합니다. kernel stack에는 실행해야 할 function의 address가 들어 있습니다. 그런 다음 kernel은 thread를 unblock하여 ready list에 넣습니다. 이것이 thread가 생성되는 방식입니다.

## Starting a Process

지금까지 thread가 생성되어 ready list에 들어갔습니다. raw 강의 전사본에서는 이 단계를 예전 이름인 `start_process`로 설명합니다. 하지만 이 저장소의 현재 KAIST Pintos tree에는 별도의 `start_process()` 함수가 없고, 대응되는 흐름은 `process_create_initd()` -> `thread_create(..., initd, ...)` -> `initd()` -> `process_exec()`입니다.

`process_exec()`는 실행하려는 binary file의 이름인 file name을 받습니다. 먼저 이 binary file을 disk에서 memory로 load해야 합니다. 그 binary file에서 실행할 instruction의 위치를 얻고, user stack의 stack-top pointer도 얻습니다. 이것이 `load()`의 역할입니다.

load가 성공하면 process는 계속 실행됩니다. load가 실패하면 thread는 exit해야 합니다. exit할 때는 process 실행을 위해 allocate된 memory chunk들을 cleanup해야 합니다.

`load()`는 매우 중요한 함수입니다. binary file을 disk에서 memory로 load하고, user stack을 initialize하고, initial pointer들을 설정한 뒤 user program으로 jump하기 위한 대부분의 작업을 수행합니다.

## Loading a Program

`a.out` 같은 executable file과 memory가 있다고 해 봅시다. `load()`가 호출되면 kernel은 주어진 thread를 위한 page table을 생성합니다. 그런 다음 file을 열고 ELF header를 읽습니다.

Executable은 ELF format입니다. ELF header에는 file이 어떻게 구성되어 있는지에 대한 정보가 들어 있습니다. 여기에는 data section, BSS section, binary code의 위치가 포함됩니다. `load()`는 ELF header를 이용해 file을 parse하고, data section을 data segment에 load합니다. 또한 text section을 text segment에 읽어 넣습니다.

그런 다음 process를 위한 stack을 만들고 initialize합니다. thread structure는 이미 생성되어 있고, thread structure에는 page table을 가리키는 pointer가 있습니다. KAIST Pintos에서는 이 field가 `pml4`입니다. 이 page table은 data, text, stack 같은 structure들을 가리킵니다.

file을 load한다는 것은 많은 일을 포함합니다. binary file을 memory로 읽어 오는 것뿐 아니라, process 실행을 위해 data segment와 text segment를 initialize합니다.

실제 `load()` 함수는 file name과 output variable을 포함한 parameter들을 받습니다. `load()`가 실행된 뒤 한 variable에는 program의 starting entry point가 들어가고, 다른 variable에는 실행에 사용할 user stack의 stack top이 들어갑니다.

## After Loading

`load()`가 끝나면 operating system은 program file을 memory로 읽어 왔고, stack, data section, BSS section, text memory를 initialize한 상태입니다.

이제 우리가 해야 할 일을 살펴볼 차례입니다. 첫 번째로 argument를 전달하고 thread를 생성하기 위한 mechanism을 구현해야 합니다.

현재 Pintos에는 command-line argument를 tokenize하는 mechanism이 없습니다. 전체 command line을 그대로 process execution에 전달합니다. 수정 후에는 command line의 각 token을 나누어야 합니다. 그런 다음 program name을 식별하고, file name으로 executable을 찾고, argument를 user stack과 register에 전달할 수 있어야 합니다.

예를 들어 `echo` command라면 `x`, `y`, `z` 같은 argument를 user stack에 배치해서 `echo`가 그 argument를 사용해 자기 일을 수행할 수 있어야 합니다.

## Functions to Modify

이 project를 구현하는 방법은 여러 가지입니다. raw 강의에서는 `start_process`를 언급하지만, 이 KAIST Pintos tree에서 argument passing을 위해 주로 확장해야 하는 함수는 project document 기준으로 `process_exec()`입니다.

## Parse the Arguments and Push Them to the Stack

가장 중요한 작업은 argument를 parse하고 user program이 사용할 수 있도록 배치하는 것입니다.

`process_exec()` 안에서 Pintos는 command line을 받습니다. 이 string을 parse하고 file name을 추출하며, 첫 token을 program name으로 사용해야 합니다. 그러면 `load()`가 그 file name으로 executable을 찾아 실행하려고 시도합니다.

`process_exec()`에서는 file name을 parse하고 각 token을 tokenize한 뒤, user program을 위한 parameter를 배치해야 합니다. Standard library는 command line을 tokenize하는 데 사용할 수 있는 `strtok_r()` 함수를 제공합니다.

## Program Name and Thread Name

`process_create_initd()`는 `thread_create()`를 호출합니다. 여기에는 두 가지 중요한 부분이 있습니다. 첫째, 실행하려는 file의 이름을 thread name으로 전달합니다. 둘째, `initd()`를 실행할 function으로 지정해 thread를 만들고, `initd()`가 다시 `process_exec()`를 호출합니다.

## `process_exec()`

raw 강의에서는 이 구간을 `start_process`로 소개합니다. 이 저장소의 KAIST Pintos code에서 대응되는 함수는 `process_exec()`입니다. `process_exec()`에서는 Pintos가 interrupt frame을 allocate하고, program을 load하고, interrupt frame과 user stack을 initialize합니다. 그런 다음 argument를 설정하고 `do_iret()`을 통해 user program으로 jump합니다.

문제는 현재 Pintos가 user stack을 argument와 함께 initialize하는 mechanism을 갖고 있지 않다는 것입니다. 이 부분이 이 homework에서 구현해야 할 부분입니다.

그 전에 kernel에 들어가고 kernel에서 나오는 것이 무엇을 의미하는지 이해해야 합니다.

## Getting Into and Out of Kernel

KAIST Pintos에서 system call로 kernel에 들어가기 위해 user program은 `syscall` instruction을 실행합니다. 그러면 user program은 operating system으로 trap됩니다. kernel이 작업을 마치면 Pintos는 `do_iret()`과 `iretq` instruction을 통해 user mode로 돌아갑니다.

process의 virtual address space는 kernel space와 user space로 구성됩니다. user space에는 text, data, BSS, stack region이 있습니다. x86-64의 일반 실행에서는 stack pointer인 `rsp`가 user stack의 top을 가리킵니다.

CPU가 kernel에 들어가면 entry path는 user stack에서 kernel stack으로 전환합니다. stack을 user stack에서 kernel stack으로 전환한 뒤, user process가 사용하던 register들을 저장합니다. kernel이 user register를 kernel stack에 저장하기 위해 사용하는 data structure가 interrupt frame입니다.

CPU가 kernel로 trap되면 user mode에서 kernel mode로 전환되고, entry code가 register를 kernel space에 있는 interrupt frame에 저장합니다. system call path에서 중요한 instruction은 `syscall`입니다.

## `struct intr_frame`

`struct intr_frame` data structure는 CPU state와 general-purpose register들을 저장합니다.

interrupt frame의 일부는 CPU가 정의하고, 일부는 operating system이 정의합니다. x86 CPU architecture에서 동작하는 다른 operating system을 보더라도 CPU가 정의한 부분은 동일하지만, operating-system-defined part는 어떤 operating system을 쓰는지에 따라 달라질 수 있습니다.

interrupt frame은 kernel stack에 위치하며 user process의 register를 저장합니다.

## Getting Into Kernel

user program이 interrupt, exception, system call을 통해 kernel에 들어오면, operating system은 현재 실행 중인 process의 register를 kernel stack에 저장합니다. `rsp`는 user stack에서 kernel stack top으로 전환되고 register들이 저장됩니다.

처음에는 `rsp` register가 user stack의 top을 가리킵니다. entry path가 stack pointer를 user stack에서 kernel stack으로 전환합니다. 그런 다음 interrupt handler 또는 system call handler에서 execution이 시작됩니다.

interrupt 또는 system call entry path 안에서 Pintos는 general-purpose register와 관련 field를 저장합니다. user process가 사용한 register들을 kernel stack에 저장한 뒤, `rsp`는 interrupt frame의 top을 가리킵니다.

kernel에 들어가는 과정은 비쌉니다. stack pointer를 전환하고 많은 register를 저장해야 합니다. 일부는 hardware가 수행하지만, 여러 register는 software가 kernel stack에 저장해야 합니다.

## Back to Loading

`process_exec()`는 program name을 `load()`에 전달합니다. 그러면 `load()`는 그 file name으로 executable file을 찾아 memory로 load합니다.

file name은 load하려는 file의 이름입니다. entry point는 program이 load된 뒤 실행해야 하는 main function의 시작 address입니다. `load()`는 이 entry-point field를 initialize할 책임이 있습니다. 또한 user stack의 `rsp` field를 initialize하는 책임도 있습니다. 이 field에는 operating system이 program 실행을 시작할 때 사용할 user stack-top address가 들어 있습니다.

`process_exec()`의 body에는 중요한 세 단계가 있습니다. 첫째, executable을 load하고 user stack을 initialize합니다. 둘째, user program을 실행하기 전에 argument set을 user process에 전달합니다. 이 부분이 argument passing에서 확장되는 부분입니다. 셋째, kernel에서 나가 실행해야 할 user program으로 jump합니다.

빠진 stack setup code를 작성하기 전에, load와 kernel에서 나가는 방식이 어떻게 동작하는지 이해해야 합니다.

## Getting Out of the Kernel

kernel에서 나가는 과정은 기본적으로 assembly instruction으로 register를 복원하고 `iretq`를 실행하는 과정입니다.

`movq`를 통해 Pintos는 stack pointer가 interrupt frame의 현재 stack top을 가리키도록 설정합니다. 그런 다음 `do_iret()`을 호출합니다.

`do_iret()`에서는 Pintos가 register를 복원하고 `iretq` instruction을 호출합니다. 그 결과 `rsp`는 interrupt frame의 top을 가리키고, saved register들이 복원됩니다. 이후 `iretq`가 CPU-defined state를 복원하고 execution mode를 kernel mode에서 user mode로 바꿉니다.

`iretq` 실행 후 saved state가 CPU에 복원되고, `rsp`는 user stack의 stack-top address로 설정됩니다. 이것이 Pintos가 kernel에 들어가고 나오는 방식입니다.

기억해야 할 점이 하나 있습니다. thread가 처음 생성되면 interrupt frame은 비어 있습니다. process가 kernel에 들어올 때 interrupt frame은 값들로 채워집니다. kernel에 들어올 때 register들이 entry path를 통해 저장되고, kernel에서 나갈 때는 `iretq`가 사용됩니다.

하지만 새로 생성된 process는 아직 user space에 있었던 적이 없습니다. 따라서 interrupt frame이 비어 있습니다. process가 생성된 뒤 kernel을 떠나 user space로 가려면 operating system이 interrupt frame을 올바른 값으로 initialize해야 합니다. 이것이 `process_exec()`가 `do_iret()`을 호출하기 전에 하는 일입니다.

## Write a Function That Sets Up a Stack

실행하려는 function으로 실제로 jump하기 전에, parameter list가 올바르게 들어 있는 user stack을 먼저 설정해야 합니다. 이것이 우리가 작성할 기능입니다.

user address space를 생각해 봅시다. text region, data region, BSS region, stack이 있습니다. interrupt frame의 `rsp` field가 stack-top address를 담고 있다고 가정합니다. `process_exec()`에서는 process가 control을 되찾았을 때 parameter를 읽고 program을 실행할 수 있도록 user stack을 설정하는 code를 작성합니다.

stack-top address는 현재 interrupt frame의 `rsp` field에 저장되어 있습니다. 해야 할 일은 stack top부터 parameter들을 하나씩 배치하는 것입니다.

## x86-64 Calling Convention

네 개의 argument를 가진 command line을 생각해 봅시다. index는 0부터 3까지입니다. x86-64 calling convention에서는 argument를 특정 순서로 배치해야 합니다.

먼저 character string들을 stack에 둡니다. 그런 다음 각 string의 address와 null pointer sentinel을 오른쪽에서 왼쪽 순서로 push합니다. 첫 push 전에 stack pointer를 8의 배수로 내림해야 합니다. 그런 다음 `%rsi`가 `argv`를 가리키게 하고, `%rdi`를 `argc`로 설정한 뒤, 마지막으로 fake return address를 push합니다.

이것이 argument를 user stack에 배치할 때 따라야 하는 rule입니다.

## User Stack Layout in Function Call

argument 수가 4개이고 index가 0부터 3까지라고 가정합니다. parameter를 push하기 전 stack pointer는 stack top을 나타냅니다.

이제 개별 string을 stack에 배치합니다. 예를 들어 `bar\0`, `foo\0`, 다른 argument string, 그리고 `/bin/ls\0` 같은 program name을 둡니다.

중요한 것은 stack alignment입니다. KAIST Pintos에서는 `argv` pointer를 push하기 전에 stack pointer를 8의 배수로 내림합니다.

그런 다음 null pointer인 0을 push합니다. 이는 argument string pointer array의 끝을 의미합니다. 그다음 각 character string의 address를 역순으로 push합니다. argument 3, argument 2, argument 1, argument 0 순서입니다.

이후 이 parameter set의 starting address를 저장합니다. 이것이 `argv` address입니다. 다음으로 `%rsi`를 `argv` address로 설정하고, `%rdi`를 argument 수, 예를 들어 4로 설정합니다. 그런 다음 fake return address를 push합니다.

이 경우 새로 생성된 process이므로 실제 return address는 없습니다. 실행이 끝나면 돌아갈 곳이 없고 thread가 끝납니다. 따라서 fake return address로 0을 push합니다.

이 기능을 완전히 구현한 뒤에는 Pintos가 제공하는 `hex_dump()` 함수를 사용해 stack frame이 제대로 설정되었는지 확인할 수 있습니다. `hex_dump()`로 interrupt frame의 hex map을 dump하여 stack이 올바르게 설정되었는지 볼 수 있습니다.

이것으로 user stack setup 설명을 마칩니다.
