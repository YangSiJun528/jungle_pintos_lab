# [Week04] Pintos Project2 file manipulation

Source: https://youtu.be/SqMD8rbmEjY?si=accwnC44dvK9bpIs

## File Manipulation

이제 Pintos operating system에서 file manipulation feature를 구현합니다.

process descriptor는 `struct thread`입니다. thread structure 안에는 file descriptor table을 가리키는 pointer가 필요합니다. file descriptor table은 pointer의 array이고, array의 각 entry는 file object를 가리킵니다.

처음 entry들은 standard stream을 위해 사용됩니다. KAIST Pintos에서 file descriptor 0은 `STDIN_FILENO`, 즉 standard input이고, file descriptor 1은 `STDOUT_FILENO`, 즉 standard output입니다. 그 뒤 entry들은 일반 file descriptor entry로 사용됩니다.

현재 Pintos에는 file descriptor table이 없으므로 이를 구현해야 합니다.

## File Descriptor Table

각 process는 자기 own file descriptor table을 가집니다. 최대 size를 64 entries로 정의할 수 있습니다. file descriptor table은 `struct file *` pointer의 array입니다. 각 entry는 `struct file` object를 가리킵니다.

file descriptor는 file descriptor table의 index입니다. available entry에서 순차적으로 allocate됩니다. file descriptor 0과 1은 standard input과 standard output을 위해 reserved되어 있으며, `open`은 이 descriptor들을 return해서는 안 됩니다.

`open` system call은 file descriptor를 return합니다. file을 open하고, file descriptor table에서 empty entry를 scan한 다음, 그 entry가 opened `struct file`을 가리키도록 설정합니다.

`close` system call은 해당 table entry의 값을 0 또는 `NULL`로 reset합니다.

## File Descriptor Table Layout

64 entries를 가진 file descriptor table을 정의한다고 해 봅시다.

한 가지 approach는 file descriptor table을 `struct thread` 안에 직접 embed하는 것입니다. 다른 approach는 `struct thread` 안에 file descriptor table을 가리키는 pointer를 추가하고, 실제 table은 kernel memory에 별도로 allocate하는 것입니다. pointer approach를 사용하면 필요에 따라 두 thread가 같은 file descriptor table을 share할 수도 있습니다.

예시에서 thread A는 자기 file descriptor table을 가지고, thread B도 자기 file descriptor table을 가집니다. 각 `struct thread`는 자기 table을 가리키는 file descriptor table pointer를 가집니다.

thread가 생성될 때 operating system은 해당 thread를 위한 file descriptor table을 allocate하고, 그 table을 가리키는 pointer를 initialize합니다.

잊지 말아야 할 점은 file descriptor table을 initialize할 때 file descriptor 0은 standard input, file descriptor 1은 standard output을 위해 reserve해야 한다는 것입니다.

process가 terminate되면 모든 file을 close해야 합니다. 그런 다음 operating system은 file descriptor table을 deallocate합니다.

## Race Conditions

강조해야 할 점은 race condition입니다.

Pintos에서는 file system operation의 race condition을 피하기 위해 global lock을 사용합니다. global lock을 정의하고, file-system-related system call이 실행될 때마다 그 lock으로 보호합니다.

이는 file system operation의 race condition을 피하기 위한 것입니다.

## Page Fault for Tests

test를 위해 page fault handling을 수정해야 합니다. 일부 Pintos test는 kernel이 bad process를 제대로 handle하는지 확인합니다.

Pintos에서 user process에 page fault가 발생하면 kernel은 process를 terminate하고 process name과 exit status `-1`을 print해야 합니다. test requirement를 만족하기 위해 page fault handling을 수정합니다. 단순한 approach는 실패 처리를 `exit(-1)`과 같은 path로 보내는 것입니다.

## File Manipulation System Calls

다음은 file manipulation과 관련된 system call입니다.

`create`는 initial size를 가진 file을 생성합니다. 내부적으로 `filesys_create()`를 호출합니다.

`remove`는 주어진 이름의 file을 제거합니다. 내부적으로 `filesys_remove()`를 호출합니다.

`open`은 file을 엽니다. 내부적으로 `filesys_open()`을 호출하고, return된 `struct file *`을 file descriptor table에 저장한 뒤 file descriptor를 return합니다.

이 helper function들은 이미 Pintos에 정의되어 있습니다. 해야 할 일은 이 함수들을 호출하는 적절한 system call을 제공하는 것입니다.

`filesize`는 file의 length를 return합니다. 내부적으로 `file_length()`를 호출합니다.

## `read`

`read` system call에서는 standard input에서 읽는 경우와 다른 file descriptor에서 읽는 경우를 구분해야 합니다.

file descriptor가 0이면 keyboard에서 읽기 위해 `input_getc()`를 호출합니다.

0이 아닌 다른 file descriptor에 대해서는 `file_read()`를 호출합니다.

## `write`

`write` system call에도 같은 rule이 적용됩니다.

file descriptor가 1이면 console에 output을 씁니다. project document는 buffer를 `putbuf()`로 write하는 것을 권장합니다.

다른 file descriptor에 대해서는 `file_write()`를 호출합니다.

## Other File System Calls

`seek`라는 function은 `file_seek()`을 호출해 file의 current position을 변경합니다.

`tell`이라는 function은 `file_tell()`을 호출해 file의 current position을 return합니다.

또한 `close`는 `file_close()`로 file을 닫고, 관련 file descriptor table entry를 release합니다.

## Deny Writes to Executables

마지막 주제는 executable에 대한 write를 deny하는 것입니다.

operating system이 수정 중인 file을 execute하려고 하면 어떻게 될까요? 결과는 unpredictable할 수 있습니다. 이 주제의 목표는 execution을 위해 open된 file이 수정되지 않도록 하는 것입니다.

approach는 file이 execution을 위해 load될 때 `file_deny_write()`를 호출하고, process가 실행 중인 동안 그 file을 open 상태로 유지하는 것입니다. process가 exit할 때는 `file_allow_write()`를 호출하거나 file을 close합니다. 이 approach를 사용하면 Pintos가 running executable file에 대한 write를 deny할 수 있습니다.

이 모든 feature를 완전히 구현하면 test를 통과할 수 있어야 합니다.

## Summary of Functions to Add and Modify

Pintos에서 process를 생성하는 call flow는 다음과 같습니다.

initial process의 경우 먼저 Pintos가 `process_create_initd()`를 호출합니다. 그다음 `process_create_initd()`가 `thread_create()`를 호출합니다. 새 thread는 `initd()`를 실행하고, `initd()`는 `process_exec()`를 호출하며, `process_exec()`가 `load()`를 호출합니다.

각 단계에서 필요한 feature를 제공해야 합니다. `process_exec()`에서는 실행할 program의 name을 parse합니다. `thread_create()`에서는 thread를 생성하고 ready list에 추가합니다. 새 thread가 생성되어 ready list에 들어가지만, executable은 이후 `process_exec()`에서 load됩니다.

`process_exec()`에서는 execution이 kernel mode에서 user mode로 이동하여 user program을 시작할 수 있도록 interrupt frame을 준비합니다. 그런 다음 executable을 load합니다. 이 모든 것이 성공하면 user program이 실행될 수 있습니다.
