# Page Table, MMU, TLB로 이해하는 Virtual Memory 동작 원리

원본 영상: <https://youtu.be/B6tJxvYBNrU?si=_9xbHCzw9tP5ZiYe>

이 문서는 위 영상 스크립트의 내용을 바탕으로, virtual memory가 실제 하드웨어와 운영체제 안에서 어떻게 동작하는지 기술 포스팅 형태로 재구성한 글이다. 단순한 영상 요약이 아니라, Pintos를 공부할 때 헷갈리기 쉬운 page table, MMU, TLB, page fault의 관계를 중심으로 다시 설명한다.

주요 근거:

- 제공된 YouTube 스크립트
- `docs/reference/pintos-kaist-kr/3_project3/0_introduction.md`
- `docs/reference/pintos-kaist-kr/5_appendix/4_page_table.md`
- `docs/reference/pintos-kaist-kr/2_project2/0_introduction.md`

## 1. Virtual memory는 주소를 한 번 더 해석하는 시스템이다

프로그램이 사용하는 pointer 값은 실제 RAM의 물리 주소가 아니다. 프로그램은 자신만의 연속된 메모리 공간이 있는 것처럼 virtual address를 사용하고, CPU와 운영체제는 그 virtual address를 physical address로 바꿔 실제 RAM에 접근한다.

이때 변환의 기본 단위는 byte가 아니라 page다.

```text
virtual address
  = virtual page number + page offset

physical address
  = physical frame number + page offset
```

page는 virtual memory의 고정 크기 조각이고, frame은 physical memory의 고정 크기 조각이다. 일반적으로 둘 다 같은 크기를 가진다. KAIST Pintos reference에서도 page는 4,096 bytes이며, 64-bit virtual address의 하위 12 bit가 page offset이라고 설명한다.

중요한 점은 offset은 변하지 않는다는 것이다. 주소 변환은 virtual page number를 physical frame number로 바꾸는 일이고, page 안에서 몇 번째 byte인지를 나타내는 offset은 그대로 유지된다.

```text
virtual address:  [ virtual page number ][ offset ]
                              |
                              v
page table lookup:    virtual page -> physical frame
                              |
                              v
physical address: [ physical frame number ][ offset ]
```

따라서 virtual memory의 핵심 질문은 이것이다.

> 어떤 virtual page가 어떤 physical frame에 연결되어 있는가?

이 질문에 답하기 위해 운영체제와 CPU가 함께 사용하는 자료구조가 page table이다.

## 2. Page table은 주소 변환용 색인이다

page table은 virtual page를 physical frame으로 변환하기 위한 자료구조다. 각 process는 독립적인 user virtual memory를 가지므로, 원칙적으로 process마다 자기 주소 공간을 설명하는 page table이 필요하다.

page table entry, 즉 PTE에는 보통 다음 정보가 들어간다.

| 항목 | 의미 |
| --- | --- |
| physical frame number | virtual page가 실제로 연결된 frame |
| present/valid bit | 이 mapping이 현재 유효한지 |
| writable/read-only bit | write가 가능한지 |
| user/supervisor bit | user mode에서 접근 가능한지 |
| no-execute bit | 이 page의 내용을 instruction으로 실행할 수 있는지 |
| accessed bit | 최근 read/write 접근이 있었는지 |
| dirty bit | memory에 올라온 뒤 write로 수정되었는지 |
| global bit | 여러 address space에서 공통으로 쓰는 mapping인지 |

영상은 이해를 돕기 위해 page table을 “process마다 하나씩 존재하는 거대한 배열”처럼 설명한다. 예를 들어 virtual address space가 16 GB이고 page size가 4 KB라면 page 수는 다음과 같다.

```text
16 GB / 4 KB = 2^34 / 2^12 = 2^22 pages
```

즉 약 419만 개의 page table entry가 필요하다. entry 하나를 4 bytes로만 잡아도 한 process의 page table이 약 16 MB가 된다. process가 수십 개라면 page table만으로 수백 MB가 필요할 수 있다.

이 계산은 일부러 단순화된 모델이다. 실제 현대 시스템은 이렇게 전체 virtual address space를 flat array 하나로 전부 만들지 않는다. x86-64는 multi-level page table을 사용하고, KAIST Pintos에서도 page table은 4단계 구조인 PML4로 표현된다. 사용하지 않는 큰 virtual address 영역에 대해서는 하위 table을 만들지 않아 sparse한 주소 공간을 더 효율적으로 다룰 수 있다.

그래도 이 단순화는 중요한 직관을 준다. virtual memory는 “공짜 추상화”가 아니다. 주소 변환 정보를 저장하는 데도 RAM이 필요하고, 그 정보를 찾는 데도 시간이 든다.

## 3. Context switch 때 active page table이 바뀐다

process마다 다른 virtual address space를 가진다면, 같은 virtual address라도 process에 따라 다른 physical frame을 가리킬 수 있다.

```text
process A: virtual page 0x1000 -> frame X
process B: virtual page 0x1000 -> frame Y
```

따라서 CPU가 현재 어떤 page table을 기준으로 주소를 변환해야 하는지 알아야 한다. 이를 위해 CPU에는 현재 활성화된 page table의 기준 위치를 가리키는 protected register가 있다. 영상에서는 이를 page table base register라고 설명한다.

KAIST Pintos reference도 같은 개념을 설명한다. user virtual memory는 per-process이고, kernel이 한 process에서 다른 process로 switch할 때 processor의 page directory base register를 변경하여 user virtual address space도 switch한다. Pintos 코드에서는 이 흐름이 `pml4_activate()`와 연결된다.

즉 context switch는 register와 stack만 바꾸는 일이 아니다. user process를 전환할 때는 “앞으로의 virtual address를 어떤 page table로 해석할 것인가”도 함께 바뀐다.

## 4. PTE의 permission bit는 주소 변환과 보호를 동시에 수행한다

page table은 단순히 주소만 바꾸는 표가 아니다. 각 PTE의 bit들은 메모리 보호 정책도 담고 있다.

예를 들어 user/supervisor bit는 해당 page를 user mode code가 접근할 수 있는지 결정한다. kernel page가 supervisor-only로 표시되어 있으면 일반 user program은 그 page를 읽거나 쓸 수 없다. 이 장치가 없다면 user program이 kernel memory를 직접 읽거나 수정할 수 있으므로 process 격리와 운영체제 보호가 깨진다.

writable bit도 중요하다. code segment나 read-only data가 writable하지 않게 mapping되면, accidental write나 공격 코드가 해당 영역을 덮어쓰는 일을 막을 수 있다.

no-execute bit는 data page의 내용을 instruction으로 실행하지 못하게 막는 보안 기능이다. 모든 architecture나 모든 설정에서 같은 이름으로 쓰이는 것은 아니지만, “이 page는 data로만 사용하고 code로 실행하지 않는다”는 정책을 page 단위로 표현한다는 점이 핵심이다.

Pintos에서 Project 2 user memory를 다룰 때도 같은 큰 원리가 적용된다. user program은 자기 user virtual memory에만 접근할 수 있고, kernel virtual memory에 접근하려고 하면 `userprog/exception.c`의 `page_fault()`가 처리하는 page fault가 발생한다.

## 5. Accessed bit와 dirty bit는 page 교체의 관찰 도구다

accessed bit와 dirty bit는 page replacement를 이해할 때 특히 중요하다.

accessed bit는 page에 read 또는 write가 발생했는지를 나타낸다. dirty bit는 page가 write로 수정되었는지를 나타낸다. KAIST Pintos Project 3 reference에 따르면 x86-64 hardware는 page에 read/write가 발생하면 accessed bit를 1로 만들고, write가 발생하면 dirty bit를 1로 만든다. CPU는 이 bit들을 자동으로 0으로 되돌리지 않지만, OS는 필요할 때 reset할 수 있다.

이 bit들이 필요한 이유는 page eviction 때문이다.

physical memory가 부족하면 운영체제는 어떤 frame을 비워야 한다. 그런데 아무 page나 내보내면 성능이 크게 나빠질 수 있다. 최근 사용된 page는 다시 필요할 가능성이 높고, dirty page는 disk나 file에 write-back해야 하므로 비용이 더 크다.

그래서 OS는 accessed bit를 보고 “최근에 사용되었는가”를 추정하고, dirty bit를 보고 “내보낼 때 write-back이 필요한가”를 판단한다.

```text
accessed = 0, dirty = 0
  -> 최근 사용되지 않았고 수정도 안 됨. eviction 후보로 비교적 부담이 작음.

accessed = 1, dirty = 0
  -> 최근 읽혔거나 실행됨. 다시 필요할 수 있음.

accessed = 1, dirty = 1
  -> 최근 사용되었고 수정됨. evict하려면 backing store 반영이 필요할 수 있음.
```

Pintos reference는 aliases도 주의하라고 한다. 같은 physical frame을 두 개 이상의 virtual page가 가리킬 수 있는데, accessed/dirty bit는 실제 접근에 사용된 PTE에서만 갱신된다. Pintos에서는 user virtual page와 그 frame을 접근하는 kernel virtual address가 alias 관계가 될 수 있으므로, VM 구현에서는 이 점을 고려해야 한다.

## 6. Page table만 쓰면 memory access가 너무 비싸다

여기서 성능 문제가 생긴다. CPU가 어떤 instruction을 실행하다가 memory에 접근한다고 해보자. 프로그램이 제공한 주소는 virtual address다. 실제 RAM에 접근하려면 먼저 page table을 봐서 physical address로 바꿔야 한다.

단순한 모델에서는 memory access 하나가 다음처럼 바뀐다.

```text
1. page table entry를 읽기 위해 RAM 접근
2. 변환된 physical address로 실제 data를 읽기 위해 RAM 접근
```

결과적으로 원래 한 번이면 될 memory access가 두 번이 된다. multi-level page table에서는 page table walk 자체가 여러 단계의 memory access를 요구할 수도 있다. CPU에 비해 main memory가 느리다는 점을 생각하면, 모든 load/store마다 이 비용을 그대로 치르는 것은 받아들이기 어렵다.

이 문제를 줄이기 위해 CPU 안에는 MMU와 TLB가 있다.

## 7. MMU는 주소 변환과 보호를 수행하는 하드웨어다

MMU, 즉 Memory Management Unit은 CPU가 생성한 virtual address를 physical address로 변환하는 하드웨어 구성 요소다. 보통 CPU core와 cache 가까이에 통합되어 있다.

MMU의 역할은 단순 번역기가 아니다.

- virtual address를 physical address로 변환한다.
- PTE의 permission bit를 확인한다.
- user mode가 supervisor page에 접근하는지 검사한다.
- read-only page에 write하려는지 검사한다.
- invalid mapping에 접근하면 exception을 발생시킨다.

즉 MMU는 주소 변환기이면서 동시에 memory protection의 집행자다. 운영체제가 page table에 정책을 기록하면, MMU가 실제 memory access 순간마다 그 정책을 강제한다.

이 관점에서 system call의 user pointer 검증도 이해할 수 있다. KAIST Pintos Project 2 reference는 user pointer를 명시적으로 검증하는 방법과, `KERN_BASE` 아래인지만 확인한 뒤 실제 접근에서 발생하는 page fault를 처리하는 방법을 설명한다. 후자는 MMU가 잘못된 접근을 잡아주는 성질을 이용하는 방식이다.

## 8. TLB는 page table lookup 결과를 저장하는 작은 cache다

MMU 안에는 TLB, 즉 Translation Lookaside Buffer가 있다. TLB는 최근 사용한 virtual page to physical frame 변환 결과를 저장하는 작은 cache다.

page table이 전체 주소 공간에 대한 큰 지도라면, TLB는 최근 자주 본 주소 변환만 적어 둔 작은 메모지에 가깝다.

TLB entry에는 보통 다음 정보가 들어간다.

| 항목 | 의미 |
| --- | --- |
| virtual page number | lookup key |
| physical frame number | 변환 결과 |
| permission/status bits | 접근 가능 여부와 상태 |
| address space identifier | 어느 address space의 entry인지 구분하는 값 |

영상은 address space identifier, 즉 ASID를 소개한다. ASID가 있으면 TLB가 여러 process의 translation을 동시에 보관할 수 있다. 그렇지 않으면 context switch 때 이전 process의 TLB entry가 새 process의 같은 virtual page number와 충돌할 수 있으므로 TLB를 flush해야 한다.

architecture마다 이름과 세부 동작은 다르다. 예를 들어 x86 계열에서는 PCID 같은 기능이 이 역할과 관련된다. Pintos reference는 `pml4_*` 함수들이 필요한 경우 TLB를 flush한다고 설명한다. 학습 단계에서는 “TLB entry는 page table의 일부를 빠르게 재사용하기 위한 cache이고, address space 전환과 밀접하게 연결된다” 정도로 잡으면 충분하다.

TLB는 매우 작다. 영상에서는 보통 수십에서 천여 개 entry 규모라고 설명한다. page table 전체에 비하면 극히 작지만, 프로그램은 같은 code와 data 주변을 반복해서 접근하는 경향이 강하다. 이 locality 덕분에 작은 TLB도 큰 효과를 낸다.

현대 CPU는 instruction fetch용 iTLB와 data access용 dTLB를 나누어 두기도 하고, cache처럼 여러 level의 TLB hierarchy를 두기도 한다.

## 9. Memory access는 hit와 miss로 나뉜다

TLB가 있으면 memory access 흐름은 다음처럼 나뉜다.

TLB hit:

```text
1. CPU가 virtual address 생성
2. MMU가 virtual page number와 offset 분리
3. TLB에서 virtual page number lookup
4. matching entry가 있고 valid하면 physical frame number 획득
5. frame number + offset으로 physical address 생성
6. cache 또는 main memory 접근
```

TLB hit에서는 full page table을 보지 않아도 된다. 주소 변환이 빠르게 끝난다.

TLB miss:

```text
1. CPU가 virtual address 생성
2. MMU가 TLB lookup
3. TLB에 entry가 없음
4. page table walk로 PTE 탐색
5. 유효한 mapping이면 TLB에 entry를 채움
6. physical address를 만들어 memory 접근
```

TLB miss는 훨씬 비싸다. page table을 읽어야 하고, multi-level page table에서는 여러 단계의 table을 따라가야 한다. 그래도 한 번 miss를 처리해 TLB에 채워 두면, 같은 page에 대한 이후 접근은 hit가 될 가능성이 높다.

TLB가 가득 차 있으면 기존 entry 중 하나를 evict해야 한다. replacement policy는 구현마다 다를 수 있다. LRU 계열 정책을 쓸 수도 있고, round-robin이나 random replacement 같은 단순한 정책을 쓸 수도 있다.

## 10. Page fault는 항상 같은 의미가 아니다

MMU가 page table 또는 TLB entry를 확인했는데 mapping이 invalid이거나 permission에 맞지 않으면 exception을 발생시킨다. 이를 page fault라고 부른다.

page fault는 단순히 “프로그램이 잘못된 주소를 접근했다”는 뜻만은 아니다. 상황에 따라 의미가 달라진다.

Project 2 관점에서는 page fault가 대체로 invalid memory access를 의미한다. user program이 kernel virtual memory에 접근하거나, unmapped user address를 접근하면 process를 종료해야 한다.

Project 3 VM 관점에서는 page fault가 정상적인 동작의 일부가 될 수 있다. 예를 들어 lazy loading에서는 page table에 아직 physical frame mapping이 없지만, supplemental page table에는 “이 page는 executable file의 어느 offset에서 가져오면 된다”는 정보가 있을 수 있다. 이때 page fault handler는 faulted page를 확인하고, frame을 얻고, file이나 swap에서 data를 가져온 뒤, page table mapping을 설치한다.

KAIST Pintos Project 3 reference도 이 차이를 명확히 말한다. Project 2에서 page fault는 항상 kernel이나 user program의 bug를 나타냈지만, Project 3에서는 file이나 swap slot에서 page를 가져와야 한다는 의미일 수 있다.

따라서 page fault를 이해할 때는 먼저 질문해야 한다.

```text
이 fault는 invalid access인가,
아니면 아직 materialize되지 않은 valid page에 대한 demand인가?
```

이 구분을 가능하게 하는 자료구조가 supplemental page table이다. hardware page table이 “현재 physical frame에 mapping되어 있는가”를 말한다면, supplemental page table은 “이 virtual page가 유효하다면 그 내용은 어디에서 가져와야 하는가”를 설명한다.

## 11. 전체 흐름을 하나로 묶어 보기

memory access 하나를 전체 흐름으로 연결하면 다음과 같다.

```text
program instruction
  -> virtual address 생성
  -> MMU가 page number와 offset 분리
  -> TLB lookup
      -> hit:
           permission 확인
           physical frame + offset으로 접근
      -> miss:
           active page table walk
           valid PTE면 TLB fill 후 접근
           invalid 또는 permission violation이면 page fault
  -> page fault handler
      -> invalid access면 process 종료
      -> valid demand page면 frame 확보, data load, mapping 설치, 재시도
```

여기서 운영체제와 하드웨어의 역할은 분명히 나뉜다.

| 구성 요소 | 역할 |
| --- | --- |
| OS | page table 생성, mapping 설치, permission 설정, fault 처리 |
| MMU | virtual address 변환, permission 검사, fault 발생 |
| TLB | 최근 주소 변환 결과 cache |
| page table | hardware가 읽는 공식 주소 변환 자료구조 |
| supplemental page table | OS가 fault 처리를 위해 유지하는 보조 metadata |

OS는 정책과 자료구조를 준비한다. MMU는 매 memory access마다 그 정책을 빠르게 집행한다. TLB는 반복되는 주소 변환 비용을 줄인다. page fault는 이 흐름이 실패했을 때 OS가 다시 개입하는 지점이다.

## 12. Pintos를 읽을 때의 연결점

이 영상의 설명을 Pintos에 연결하면 다음 정도를 기억하면 좋다.

첫째, Pintos의 page table은 x86-64 hardware page table이고, reference에서는 이를 `pml4`라고 부른다. `pml4_activate()`는 active page table을 바꿔 CPU가 사용할 주소 변환 기준을 바꾸는 함수다.

둘째, `pml4_set_page()`는 user page를 physical frame에 연결하는 mapping을 설치한다. 인자로 보이는 `upage`는 user virtual address이고, `kpage`는 kernel이 같은 frame에 접근하기 위해 사용하는 kernel virtual address다.

셋째, `pml4_get_page()`는 user virtual address가 현재 어떤 frame에 mapping되어 있는지 확인하는 데 쓰인다. Project 2의 user pointer 검증 흐름을 읽을 때 중요하다.

넷째, `pml4_clear_page()`는 mapping을 완전히 잊는 것이 아니라 page를 not-present로 표시해 이후 접근이 fault를 일으키도록 만든다. reference는 이때 accessed/dirty bit 같은 다른 bit는 보존될 수 있다고 설명한다.

다섯째, Project 3에서는 page fault가 곧바로 버그라고 단정되지 않는다. supplemental page table을 조회해 valid한 lazy page인지 확인하고, 필요하면 frame을 얻어 page table mapping을 완성해야 한다.

이 정도 큰 그림을 잡고 나면, VM 코드는 “주소를 직접 다루는 복잡한 코드”가 아니라 다음 책임들을 나눠 구현하는 코드로 보인다.

- virtual page의 존재와 backing source를 추적한다.
- physical frame을 확보하고 부족하면 evict한다.
- page table에 현재 mapping을 설치하거나 제거한다.
- accessed/dirty bit를 참고해 replacement와 write-back을 결정한다.
- invalid access와 valid demand fault를 구분한다.

## 마무리

virtual memory는 프로그램에게 독립적인 address space를 제공하지만, 그 환상은 page table, MMU, TLB, page fault handler가 촘촘히 협력할 때만 유지된다.

page table은 virtual page와 physical frame의 관계를 기록한다. MMU는 그 기록을 읽어 주소를 변환하고 접근 권한을 검사한다. TLB는 반복되는 변환을 cache해서 성능 문제를 줄인다. page fault는 hardware가 더 이상 혼자 처리할 수 없는 상황에서 운영체제에게 제어권을 넘기는 신호다.

Pintos Project 3의 VM은 바로 이 협력을 직접 구현해 보는 과제다. 단, 구현을 시작하기 전에 이 글의 핵심 흐름을 먼저 익혀 두면 `pml4`, supplemental page table, frame table, swap, page fault handler가 각각 왜 필요한지 훨씬 선명하게 보인다.
