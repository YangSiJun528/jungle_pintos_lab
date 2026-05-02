# Project 2 user memory와 upage/kpage 정리

이 문서는 지금까지 헷갈렸던 내용을 Project 2 범위에 한해서 가볍게 정리한다. 구현 방향을 새로 제시하는 문서가 아니라, `docs/reference`와 현재 Pintos 코드의 인터페이스를 읽을 때 필요한 개념 정리다.

주요 근거:

- `docs/reference/pintos-kaist-kr/2_project2/0_introduction.md`
- `docs/reference/pintos-kaist-kr/2_project2/2_user_memory.md`
- `docs/reference/pintos-kaist-kr/5_appendix/3_virtual_address.md`
- `docs/reference/pintos-kaist-kr/5_appendix/4_page_table.md`
- `pintos/include/threads/vaddr.h`
- `pintos/include/threads/mmu.h`
- `pintos/userprog/process.c`

## 1. Project 2에는 일반적인 user heap이 없다

Project 2 기준으로는 일반 OS처럼 `malloc()`이 사용할 수 있는 user heap을 전제로 보면 안 된다. reference가 직접 말한다.

> Pintos는 memory에 들어가고 여러분이 구현한 system call만 사용하는 한 normal C program(일반 C 프로그램)을 실행할 수 있습니다. 특히 이 project에 필요한 system call 중 memory allocation(메모리 할당)을 허용하는 것이 없으므로 `malloc()`은 구현할 수 없습니다.
>
> `docs/reference/pintos-kaist-kr/2_project2/0_introduction.md`

따라서 Project 2에서 user memory는 대략 다음을 중심으로 이해하면 된다.

- ELF executable의 code segment
- initialized data segment
- uninitialized data segment, BSS
- 초기 user stack

reference의 Typical Memory Layout도 code/data/BSS/stack 구조를 보여 준다. 또 Project 2에서는 stack 크기가 고정이고, Project 3에서 grow 가능해진다고 설명한다.

> 이 project에서는 user stack(사용자 스택)의 크기가 고정되어 있지만, project 3에서는 grow(성장)할 수 있게 됩니다.
>
> `docs/reference/pintos-kaist-kr/2_project2/0_introduction.md`

즉 Project 2에서 “user가 heap을 늘리기 위해 page를 요청한다”는 식으로 이해하면 안 된다. Project 2의 page mapping은 주로 program load, 초기 stack, fork 같은 user address space 구성 과정에서 나온다.

## 2. user program이 보는 주소는 virtual address다

사용자 프로그램이 보는 포인터 값은 physical address가 아니라 user virtual address다. 겉으로는 하나의 선형 주소처럼 보인다.

```text
0x401000
0x401001
0x401002
...
```

하지만 CPU/MMU와 OS 입장에서는 그 주소를 page 단위로 해석한다.

```text
virtual address = page table index들 + page offset
```

Pintos의 `vaddr.h`는 이런 주소 계산을 돕는다.

- `PGSIZE`: page 크기, 4096 bytes
- `pg_ofs(va)`: address 안의 page offset
- `pg_round_down(va)`: 해당 주소가 속한 page의 시작 주소
- `is_user_vaddr(vaddr)`: user virtual address 범위인지 확인
- `is_kernel_vaddr(vaddr)`: kernel virtual address 범위인지 확인

여기서 중요한 점은, 주소값이 연속되어 보인다고 해서 실제 physical memory가 연속이라는 뜻은 아니라는 것이다. page table이 virtual page를 physical frame에 연결해 주기 때문에 사용자 프로그램은 연속된 주소 공간처럼 사용할 수 있다.

## 3. user virtual memory와 kernel virtual memory는 나뉜다

Pintos reference는 virtual memory를 user virtual memory와 kernel virtual memory로 나눈다.

> user virtual memory는 virtual address(가상 주소) `0`부터 `KERN_BASE`까지입니다. `KERN_BASE`는 `include/threads/vaddr.h`에 정의되어 있으며 기본값은 `0x8004000000`입니다. kernel virtual memory는 virtual address space(가상 주소 공간)의 나머지를 차지합니다.
>
> `docs/reference/pintos-kaist-kr/2_project2/0_introduction.md`

또 kernel virtual memory는 global하고, physical memory에 일대일로 매핑된다고 설명한다.

> Pintos에서 kernel virtual memory는 `KERN_BASE`부터 시작하여 physical memory(물리 메모리)에 one-to-one(일대일)로 mapping됩니다.
>
> `docs/reference/pintos-kaist-kr/2_project2/0_introduction.md`

따라서 같은 physical frame을 두 관점에서 볼 수 있다.

```text
user 쪽 관점:   upage, user virtual address
kernel 쪽 관점: kpage, kernel virtual address
실제 대상:      physical frame
```

다만 `kpage`는 user에게 보이는 주소가 아니다. kernel이 그 frame을 읽고 쓰기 위해 사용하는 kernel virtual address다.

## 4. upage와 kpage의 의미

Project 2에서 가장 중요한 함수 중 하나는 `pml4_set_page()`다.

```c
bool pml4_set_page (uint64_t *pml4, void *upage, void *kpage, bool rw);
```

reference는 이 함수를 다음처럼 설명한다.

> user page upage에서 kernel virtual address(커널 가상 주소) kpage로 identify(식별)되는 frame으로의 mapping을 ... 추가합니다.
>
> `docs/reference/pintos-kaist-kr/5_appendix/4_page_table.md`

이를 풀어 쓰면 다음과 같다.

```text
upage:
  user process가 볼 page-aligned user virtual address

kpage:
  kernel이 physical frame을 접근할 때 쓰는 kernel virtual address

pml4_set_page:
  현재 process의 page table에
  upage -> kpage가 가리키는 physical frame
  매핑을 설치한다
```

즉 page table에 “kernel virtual address 자체”를 user 주소로 붙이는 것이 아니다. 내부적으로는 `kpage`가 가리키는 physical frame이 user virtual page에 연결된다.

현재 Project 2 코드의 `install_page()` 주석도 같은 방향이다.

> 유저 가상 주소 UPAGE에서 커널 가상 주소 KPAGE로 가는 맵핑을 페이지 테이블에 추가한다.
>
> `pintos/userprog/process.c`

정확히 말하면, “UPAGE에서 KPAGE로 간다”는 표현은 API 관점의 설명이고, 실제 의미는 `UPAGE -> KPAGE가 식별하는 physical frame`이다.

## 5. Project 2에서는 대체로 미리 매핑한다

Project 2의 `load_segment()` 흐름은 lazy fault 방식이 아니다. 현재 코드에서도 Project 2 전용 블록에서 다음 순서로 동작한다.

```text
1. palloc_get_page(PAL_USER)로 user page용 frame을 얻는다.
2. file_read()로 executable 내용을 kpage에 읽는다.
3. 남은 부분을 zero로 채운다.
4. install_page(upage, kpage, writable)를 호출한다.
5. install_page()는 pml4_set_page()로 page table mapping을 설치한다.
```

초기 stack도 비슷하다.

```text
1. palloc_get_page(PAL_USER | PAL_ZERO)로 zeroed frame을 얻는다.
2. USER_STACK - PGSIZE 위치에 install_page()로 매핑한다.
3. rsp를 USER_STACK으로 설정한다.
```

따라서 Project 2 기준으로는 “page fault가 나면 그때 연결한다”가 기본 흐름이 아니다. executable segment와 초기 stack은 실행 전에 frame을 얻고 page table에 매핑해 둔다.

Project 2에서 page fault는 대체로 invalid access를 나타낸다. reference도 user가 kernel memory나 unmapped user address에 접근하면 page fault가 발생하고 process가 terminate된다고 설명한다.

## 6. page table은 vtable이 아니다

여기서는 `vtable`이라는 표현보다 `page table` 또는 Pintos 이름인 `pml4`가 맞다.

reference는 Pintos의 page table을 `pml4`라고 설명한다.

> Pintos의 page table, 즉 project에서 사용할 page table은 table이 4 level(단계)을 가지기 때문에 Intel processor documentation(문서)에서 Page-Map-Level-4라고 부르는 `pml4`입니다.
>
> `docs/reference/pintos-kaist-kr/5_appendix/4_page_table.md`

Project 2에서 각 process는 자기 user virtual address space를 가지며, kernel이 process를 switch할 때 page table도 switch한다.

> user virtual memory는 per-process(프로세스별)입니다. kernel이 한 process에서 다른 process로 switch할 때, processor의 page directory base register(페이지 디렉터리 기준 레지스터)를 변경하여 user virtual address space도 switch합니다.
>
> `docs/reference/pintos-kaist-kr/2_project2/0_introduction.md`

즉 `pml4`는 “이 process의 user virtual address들이 어떤 frame에 연결되는지”를 CPU/MMU가 알 수 있게 하는 구조다.

## 7. system call에서 user pointer를 다룰 때

system call 인자로 넘어온 포인터 값 자체는 그냥 주소값이다. 하지만 그 포인터가 가리키는 데이터를 kernel이 읽거나 쓰려면 조심해야 한다.

reference는 invalid pointer 종류를 직접 말한다.

> user는 null pointer(널 포인터), unmapped virtual memory를 가리키는 pointer, 또는 kernel virtual address space(`KERN_BASE` 위)를 가리키는 pointer를 전달할 수 있기 때문입니다.
>
> `docs/reference/pintos-kaist-kr/2_project2/0_introduction.md`

그래서 Project 2에서 user pointer 접근을 볼 때는 다음을 구분해야 한다.

```text
포인터 값 자체를 syscall argument로 받기
  -> register에서 주소값을 읽는 것

포인터가 가리키는 user memory의 내용 읽기/쓰기
  -> user memory access 검증 필요
```

검증에는 최소한 두 층이 있다.

- 주소가 user range인가: `is_user_vaddr()`
- 실제로 현재 page table에 매핑되어 있는가: `pml4_get_page()` 또는 page fault 처리

reference는 올바른 방식으로 두 가지를 제시한다.

- user pointer의 validity를 verify한 뒤 dereference
- `KERN_BASE` 아래인지 확인하고 dereference한 뒤 page fault를 처리

Project 2 문맥에서 단순히 “주소 숫자가 user range 안이다”만으로 충분하지 않다. unmapped user virtual address도 kernel 안에서 접근하면 page fault가 날 수 있다.

## 8. 지금까지의 핵심 정리

Project 2 기준으로 이해하면 다음과 같다.

```text
user program이 보는 주소
  -> user virtual address

kernel이 frame을 접근하는 주소
  -> kernel virtual address, 여기서 kpage라고 자주 부름

실제 메모리 조각
  -> physical frame

page table mapping
  -> upage -> kpage가 식별하는 physical frame

virtual mapping과 physical frame
  -> kpage와 upage라는 두 virtual mapping이 하나의 physical frame을 가리킬 수 있음

Project 2의 mapping 시점
  -> 주로 load/stack setup 시점에 미리 매핑

Project 2의 user heap
  -> 일반적인 malloc 기반 heap 없음

system call user pointer
  -> 주소값과 그 주소가 가리키는 데이터 접근을 구분해야 함
```

한 문장으로 줄이면 다음과 같다.

> Project 2에서 kernel은 executable segment와 초기 stack을 위해 frame을 확보하고, 그 frame을 kernel virtual address인 kpage로 채운 뒤, process의 pml4에 upage가 그 frame을 가리키도록 매핑한다.

## 9. 하나의 physical frame을 바라보는 두 virtual mapping

`upage`와 `kpage`의 관계는 하나의 physical frame에 두 virtual address가 연결되는 구조로 이해하면 된다.

```text
kernel virtual address: kpage ──┐
                                ├── physical frame F
user virtual address:   upage ──┘
```

이 구조에서 실제 데이터가 저장되는 공간은 physical frame `F` 하나다. kernel address space에는 `kpage`라는 virtual mapping이 있고, user process의 address space에는 `upage`라는 virtual mapping이 있다. 두 mapping은 서로 다른 virtual address를 사용하지만, 최종적으로 같은 physical frame으로 translate된다.

Project 2의 실행 파일 load 흐름에 대입하면 다음과 같다.

```text
1. kernel이 user page용 physical frame F를 확보한다.
2. kernel은 frame F를 kpage라는 kernel virtual address로 접근한다.
3. kernel은 kpage에 executable 내용이나 zero-filled data를 채운다.
4. kernel은 process의 pml4에 upage -> frame F mapping을 설치한다.
5. user program은 upage로 같은 frame F의 내용을 읽거나 실행한다.
```

가상 주소 공간 기준으로는 mapping이 둘이다.

```text
kernel 쪽 mapping: kpage -> frame F
user 쪽 mapping:   upage -> frame F
```

물리 메모리 기준으로는 데이터 page가 하나다.

```text
physical frame: frame F 하나
```

따라서 메모리 사용을 볼 때는 다음처럼 구분해야 한다.

- virtual address는 kernel 쪽과 user 쪽에 각각 존재할 수 있다.
- page table entry는 mapping 수만큼 필요할 수 있다.
- 실제 데이터가 담긴 physical frame은 하나일 수 있다.

이 구조는 kernel이 physical frame을 관리하고 초기화하면서도, user process가 자기 user virtual address로 같은 frame에 접근할 수 있게 만든다. user에게 `kpage`를 직접 주는 방식이 아니라, user address space 안의 `upage`가 같은 frame을 가리키도록 page table entry를 설치하는 방식이다.

권한은 user mapping의 page table entry가 결정한다. user mode 접근에는 user-accessible bit가 필요하고, write 가능 여부는 writable bit에 의해 결정된다. kernel은 kernel virtual mapping을 통해 frame에 접근할 수 있지만, user process는 자기 page table에 허용된 `upage` mapping을 통해서만 접근한다.

## 10. Project 3 이후에는 무엇이 달라지는가

Project 3 VM부터는 위 그림에 lazy loading, supplemental page table, stack growth, anonymous page, mmap, swap이 추가된다. 이 부분은 Project 2 범위를 넘으므로 맛보기로만 정리한다.

### lazy loading

Project 2에서는 segment page를 미리 읽고 매핑하는 쪽으로 이해하면 된다. Project 3에서는 처음부터 physical frame을 붙이지 않고, page fault가 났을 때 내용을 가져오는 방식이 들어온다.

reference는 lazy loading을 다음처럼 설명한다.

> page가 allocate(할당)되어 그에 대응하는 page struct는 있지만 dedicated physical frame(전용 물리 프레임)은 없고, page의 실제 content(내용)도 아직 load되지 않았습니다. content는 page fault(페이지 폴트)가 signal(신호)하는, 정말 필요해지는 시점에만 load됩니다.
>
> `docs/reference/pintos-kaist-kr/3_project3/2_anon.md`

### supplemental page table

Project 3에서는 “지금 pml4에 present mapping이 있는가”와 “이 주소가 valid한 user page인가”가 분리된다.

```text
pml4/page table:
  CPU가 실제로 translate할 present mapping

supplemental page table:
  이 virtual page가 어떤 데이터를 가져야 하는지에 대한 kernel metadata
```

따라서 `pml4_get_page()`가 `NULL`이어도 supplemental page table 기준으로는 valid page일 수 있고, page fault 때 frame을 붙일 수 있다.

### stack growth

Project 2에서는 stack이 고정 크기다. Project 3에서는 stack access처럼 보이는 fault에 대해 anonymous page를 추가로 allocate해서 stack을 grow할 수 있다.

### anonymous page와 heap

Project 3 reference는 anonymous page가 stack과 heap처럼 file backing이 없는 memory에 사용된다고 설명한다. 다만 Pintos 과제에서 일반 OS의 `brk`/`sbrk` 기반 heap 확장을 그대로 구현한다고 단정하면 안 된다. Project 3 문서에서 명시적으로 다루는 것은 VM page type, lazy loading, stack growth, swapping, mmap 등이다.

### mmap

Project 3의 `mmap`은 file-backed page다. file 내용을 virtual page에 연결하고, 필요하면 page fault 때 frame을 확보해 file에서 내용을 읽어 온다.

Project 2의 핵심과 비교하면:

```text
Project 2:
  load/stack setup 때 frame을 얻고 pml4에 미리 매핑

Project 3:
  먼저 metadata를 만들고, fault 때 frame을 얻고 pml4에 매핑하는 경우가 많아짐
```
