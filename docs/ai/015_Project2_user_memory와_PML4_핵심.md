# Project 2 user memory와 PML4 핵심

이 문서는 Pintos Project 2의 user memory 접근을 이해하기 위한 핵심 개념을 정리한다. 범위는 Project 2의 `userprog` 중심이며, x86-64 PML4, `upage`, `kpage`, `vaddr.h`, `mmu.h`를 실제 코드 읽기에 필요한 수준으로 다룬다.

주요 근거:

- `docs/reference/pintos-kaist-kr/2_project2/0_introduction.md`
- `docs/reference/pintos-kaist-kr/2_project2/2_user_memory.md`
- `docs/reference/pintos-kaist-kr/5_appendix/3_virtual_address.md`
- `docs/reference/pintos-kaist-kr/5_appendix/4_page_table.md`
- `pintos/include/threads/vaddr.h`
- `pintos/include/threads/mmu.h`
- `pintos/userprog/process.c`

## 1. Project 2의 user memory 범위

Project 2의 user program은 memory에 들어가고, 구현된 system call만 사용하는 일반 C program을 실행하는 것을 목표로 한다. Project 2에서 요구되는 system call에는 user memory allocation을 제공하는 호출이 포함되어 있지 않으므로 user-level `malloc()` 구현은 Project 2 범위에 들어가지 않는다.

Project 2에서 주로 다루는 user memory 영역은 다음과 같다.

- executable의 code segment
- initialized data segment
- uninitialized data segment, BSS
- 초기 user stack

reference의 Typical Memory Layout은 user stack, BSS, initialized data, code segment 순서로 user virtual memory 배치를 설명한다. 또한 Project 2에서는 user stack 크기가 고정되어 있고, stack growth는 Project 3에서 다루는 기능으로 분리되어 있다.

## 2. 주소와 메모리 객체의 큰 그림

user program이 보는 pointer 값은 user virtual address다. CPU는 virtual address를 page table을 통해 physical address로 변환한다.

```text
user pointer
  -> user virtual address
  -> page table / PML4 lookup
  -> physical frame + page offset
  -> 실제 memory 접근
```

기본 용어는 다음처럼 잡으면 된다.

- **virtual address**: program이나 kernel code가 pointer 값으로 사용하는 주소
- **page**: page-size 단위의 virtual memory 영역
- **physical frame**: page-size 단위의 실제 physical memory 영역
- **page table**: virtual page를 physical frame에 연결하는 CPU/MMU용 자료구조
- **PML4**: x86-64 4-level page table의 최상위 table

Pintos는 virtual memory를 user virtual memory와 kernel virtual memory로 나눈다. 경계는 `KERN_BASE`이며, KAIST Pintos의 기본값은 `0x8004000000`이다.

```text
0                         KERN_BASE
|-------------------------|-------------------------|
 user virtual memory       kernel virtual memory
```

user process는 자신의 user virtual memory에 접근한다. kernel은 kernel virtual memory에 접근하고, 현재 실행 중인 user process의 user virtual memory도 접근할 수 있다. kernel 안에서도 unmapped user virtual address를 접근하면 page fault가 발생할 수 있다.

## 3. x86-64 PML4 최소 개념

x86-64 virtual address는 page table lookup을 위한 index들과 page offset으로 해석된다.

```text
63          48 47            39 38            30 29            21 20         12 11         0
+-------------+----------------+----------------+----------------+-------------+------------+
| Sign Extend |    Page-Map    | Page-Directory | Page-directory |  Page-Table |    Page    |
|             | Level-4 Offset |    Pointer     |     Offset     |   Offset    |   Offset   |
+-------------+----------------+----------------+----------------+-------------+------------+
                  9 bits           9 bits           9 bits         9 bits       12 bits
```

하위 12 bit는 page 안의 offset이다. 그 위의 bit들은 각 page table level의 index로 사용된다. page table walk는 이 index들을 따라 내려가며 최종적으로 physical frame을 찾고, virtual address의 offset을 그대로 더해 실제 접근 위치를 만든다.

Pintos의 page table은 `pml4`라는 `uint64_t *`로 표현된다. `pml4_activate()`는 CPU가 사용할 active page table을 바꾸는 역할을 한다. Project 2 코드는 PML4 내부 구조를 직접 다루는 세부 작업을 `mmu.h`의 `pml4_*` 인터페이스를 통해 수행한다.

4-level 구조는 넓은 virtual address space를 sparse하게 관리하기 위한 계층 구조다. 사용하지 않는 큰 주소 영역에 대해 모든 page table entry를 한 번에 만들지 않고, 필요한 하위 table을 필요한 시점에 둔다.

## 4. `vaddr.h`: virtual address helper

`vaddr.h`는 virtual address 자체를 page 단위로 다루기 위한 매크로를 제공한다.

| 항목 | 역할 |
| --- | --- |
| `PGSIZE` | page 크기, 4096 bytes |
| `PGMASK` | page offset bit mask |
| `pg_ofs(va)` | `va` 안의 page offset 추출 |
| `pg_no(va)` | `va`의 page number 추출 |
| `pg_round_down(va)` | `va`가 속한 page의 시작 주소 |
| `pg_round_up(va)` | 가까운 page boundary로 올림 |

user/kernel 주소 범위 확인에는 다음 매크로가 쓰인다.

| 항목 | 역할 |
| --- | --- |
| `KERN_BASE` | kernel virtual memory 시작 주소 |
| `is_user_vaddr(vaddr)` | user virtual address 범위 확인 |
| `is_kernel_vaddr(vaddr)` | kernel virtual address 범위 확인 |
| `USER_STACK` | user stack 시작 주소 |

physical address와 kernel virtual address 변환에는 다음 매크로가 쓰인다.

| 항목 | 역할 |
| --- | --- |
| `ptov(paddr)` | physical address에 대응되는 kernel virtual address 반환 |
| `vtop(vaddr)` | kernel virtual address에 대응되는 physical address 반환 |

Pintos는 kernel virtual memory를 physical memory에 일대일로 매핑한다. 이 덕분에 kernel은 physical frame을 kernel virtual address로 접근할 수 있다.

## 5. `mmu.h`: page table interface

`mmu.h`는 PML4 page table을 다루는 인터페이스를 제공한다.

주소 공간 lifecycle에 관련된 함수는 다음과 같다.

| 함수 | 역할 |
| --- | --- |
| `pml4_create()` | 새 page table 생성. kernel mapping은 포함하고 user mapping은 포함하지 않음 |
| `pml4_activate(pml4)` | CPU가 사용할 active page table 설정 |
| `pml4_destroy(pml4)` | page table과 관련 resource 해제 |

mapping 조회와 갱신에 관련된 함수는 다음과 같다.

| 함수 | 역할 |
| --- | --- |
| `pml4_set_page(pml4, upage, kpage, rw)` | user page `upage`를 `kpage`가 식별하는 frame에 매핑 |
| `pml4_get_page(pml4, uaddr)` | `uaddr`에 매핑된 frame의 kernel virtual address 조회 |
| `pml4_clear_page(pml4, upage)` | 해당 user page를 not-present로 표시 |

accessed/dirty bit 관련 함수도 `mmu.h`에 있다. Project 2 핵심 흐름에서는 자주 다루지 않지만, Project 3의 page replacement, swap, mmap write-back 같은 기능에서 중요해진다.

## 6. kernel이 관리하는 physical frame

physical memory의 page-size 조각인 frame은 kernel이 전역적으로 관리한다. Pintos의 page allocator는 memory를 kernel pool과 user pool로 나누어 관리한다. Project 2의 user page용 frame은 `palloc_get_page(PAL_USER)`로 user pool에서 얻는 흐름이 코드에 나타난다.

kernel은 frame을 여러 용도로 사용할 수 있다.

```text
kernel 용도
  -> kernel 자료구조, page table, buffer 등

user process 용도
  -> user page에 연결할 frame
  -> executable segment, 초기 stack 등
```

user process가 사용할 frame도 kernel이 먼저 확보하고 초기화한다. kernel은 확보한 frame을 kernel virtual address로 접근할 수 있고, 이 주소가 Project 2 코드에서 `kpage`로 나타난다. 이후 process의 page table에 user virtual address인 `upage`가 같은 frame을 가리키도록 mapping을 설치한다.

```text
1. kernel이 user page용 frame을 확보한다.
2. kernel이 frame을 kpage로 접근해 내용을 채운다.
3. kernel이 process의 pml4에 upage -> frame mapping을 설치한다.
4. user process가 upage로 같은 frame에 접근한다.
```

frame은 allocator 기준으로 available한 memory에서 얻는다. kernel 자료구조용 frame, page table용 frame, user page용 frame은 kernel의 allocator와 mapping 절차를 통해 용도가 구분된다.

## 7. `upage`와 `kpage`

`pml4_set_page()`의 핵심 인자는 `upage`와 `kpage`다.

```c
bool pml4_set_page (uint64_t *pml4, void *upage, void *kpage, bool rw);
```

의미는 다음과 같다.

| 이름 | 의미 |
| --- | --- |
| `upage` | user process가 볼 page-aligned user virtual address |
| `kpage` | kernel이 physical frame을 접근할 때 쓰는 kernel virtual address |
| `rw` | user mapping의 writable 여부 |

이 함수는 process의 page table에 다음 mapping을 설치한다.

```text
upage -> kpage가 식별하는 physical frame
```

실제 mapping의 핵심은 `kpage`가 가리키는 physical frame을 user virtual page인 `upage`에 연결하는 것이다.

## 8. 하나의 physical frame을 바라보는 두 virtual mapping

하나의 physical frame은 kernel virtual address와 user virtual address 양쪽에서 접근될 수 있다.

```text
kernel virtual address: kpage ──┐
                                ├── physical frame F
user virtual address:   upage ──┘
```

이 구조에서 실제 데이터가 저장되는 physical frame은 하나다. kernel address space에는 `kpage`라는 virtual mapping이 있고, user process의 address space에는 `upage`라는 virtual mapping이 있다. 두 virtual address는 서로 다르지만 같은 physical frame으로 translate된다.

메모리 사용 관점에서는 다음처럼 구분한다.

- virtual mapping은 kernel 쪽과 user 쪽에 각각 존재할 수 있다.
- page table entry는 mapping 수만큼 존재할 수 있다.
- 실제 데이터 page인 physical frame은 하나일 수 있다.

이 구조는 kernel이 frame을 초기화하고 관리하면서도, user process가 자기 user virtual address로 같은 frame에 접근할 수 있게 만든다. user mode 접근 가능 여부와 writable 여부는 user mapping의 page table flag가 결정한다.

## 9. Project 2 load와 stack 흐름

Project 2의 executable segment load는 미리 frame을 확보하고 page table에 매핑하는 방식으로 이해하면 된다.

```text
1. ELF segment의 user virtual page인 upage를 정한다.
2. kernel이 user pool에서 frame을 하나 얻는다.
3. kernel은 그 frame을 kpage로 접근한다.
4. file_read()로 executable 내용을 kpage에 읽는다.
5. 남은 byte는 zero로 채운다.
6. install_page(upage, kpage, writable)를 호출한다.
7. install_page()는 pml4_set_page()로 upage -> frame mapping을 설치한다.
```

초기 user stack도 같은 구조를 쓴다.

```text
1. zero-filled user page용 frame을 얻는다.
2. USER_STACK - PGSIZE 위치에 mapping한다.
3. rsp를 USER_STACK으로 설정한다.
```

현재 `pintos/userprog/process.c`의 Project 2 전용 `load_segment()`와 `setup_stack()`은 이 흐름을 따른다.

## 10. system call에서 user pointer 다루기

system call 인자로 받은 pointer 값과 그 pointer가 가리키는 memory 접근은 구분해서 봐야 한다.

```text
pointer 값 자체
  -> register에서 받은 user virtual address 값

pointer가 가리키는 data
  -> user virtual memory 접근
```

reference는 user가 null pointer, unmapped virtual memory pointer, kernel virtual address pointer를 전달할 수 있다고 설명한다. Project 2의 system call 구현은 이런 invalid pointer가 kernel이나 다른 process에 피해를 주지 않도록 처리해야 한다.

검증은 보통 두 층으로 나뉜다.

- 주소 범위 확인: `is_user_vaddr()`
- 현재 page table mapping 확인: `pml4_get_page()` 또는 page fault 처리

`is_user_vaddr()`는 address range를 확인한다. 해당 주소가 현재 process의 page table에 present mapping을 가지고 있는지는 별도 확인이 필요하다. kernel 안에서도 unmapped user virtual address를 접근하면 page fault가 발생할 수 있다.

reference는 user memory 접근을 처리하는 합리적인 방식으로 다음 두 가지를 제시한다.

- user pointer의 validity를 verify한 뒤 dereference
- `KERN_BASE` 아래 주소인지 확인한 뒤 dereference하고 page fault를 처리

어느 방식을 선택하든 system call 중간에 lock이나 memory 같은 resource를 확보한 상태라면 invalid user pointer 처리 시 resource cleanup이 필요하다.

## 11. Project 2 핵심 요약

Project 2의 핵심 관계는 다음과 같다.

```text
user pointer
  -> user virtual address

upage
  -> page-aligned user virtual address

kpage
  -> kernel이 frame을 접근하는 kernel virtual address

physical frame
  -> 실제 data가 들어 있는 page-size physical memory

pml4 / page table
  -> upage를 physical frame에 연결하는 CPU/MMU용 mapping
```

전체 흐름은 다음 한 줄로 요약할 수 있다.

```text
kernel이 frame을 kpage로 채운 뒤, process의 pml4에 upage -> frame mapping을 설치한다.
```

Project 2에서는 이 흐름이 주로 executable segment load와 초기 stack setup에서 나타난다. system call에서 user pointer를 받을 때는 pointer 값의 범위와 실제 mapping 여부를 검증해야 한다.

## 12. Project 3 이후 맛보기

Project 3 VM부터는 Project 2의 eager mapping 흐름 위에 lazy loading, supplemental page table, stack growth, anonymous page, mmap, swap이 추가된다.

### lazy loading

Project 3에서는 page metadata를 먼저 만들고, 실제 frame과 content는 page fault 시점에 준비할 수 있다. 이때 page fault는 invalid access만을 의미하지 않고, file이나 swap slot에서 page를 가져와야 한다는 신호가 될 수 있다.

### supplemental page table

PML4/page table은 CPU가 실제로 translate할 present mapping을 담는다. supplemental page table은 virtual page가 어떤 data를 가져야 하는지에 대한 kernel metadata를 담는다. 따라서 VM 이후에는 present mapping과 valid page 정보가 분리된다.

### stack growth

Project 3에서는 stack access로 판단되는 fault에 대해 anonymous page를 추가로 allocate하여 stack을 늘릴 수 있다.

### anonymous page와 mmap

anonymous page는 file backing이 없는 page type이고, stack과 heap 같은 메모리 개념과 연결된다. `mmap`은 file-backed page를 user virtual page에 연결하는 기능이다. 두 경우 모두 필요 시 frame을 확보하고 page table mapping을 설치하는 흐름으로 이어진다.
