# swap in/out 구현 흐름

## 목적

이번 변경의 핵심 목적은 user pool frame이 부족할 때 기존처럼 panic으로 멈추지 않고, 기존 frame 하나를 evict한 뒤 그 frame을 새 page에 재사용하게 만드는 것이다.

기존 흐름에서는 `vm_get_frame()`이 `palloc_get_page(PAL_USER)` 실패 시 바로 `PANIC ("todo - need_evict")`로 끝났다. 따라서 `swap-anon`, `swap-file`, `swap-iter`처럼 의도적으로 메모리를 압박하는 테스트에서는 page fault를 복구할 방법이 없었다.

새 흐름은 다음과 같다.

```text
page fault
  -> vm_try_handle_fault()
  -> vm_do_claim_page()
  -> vm_get_frame()
      -> palloc_get_page(PAL_USER)
      -> 실패하면 vm_evict_frame()
  -> pml4_set_page()
  -> swap_in(page, frame->kva)
```

## 바뀐 데이터 구조

### `struct anon_page`

파일: `pintos/include/vm/anon.h`

anonymous page가 swap disk의 어느 slot에 저장되어 있는지 기억하기 위해 두 필드를 추가했다.

- `swap_slot`: swap disk 안의 page 단위 slot 번호
- `in_swap`: 현재 page 내용이 physical frame이 아니라 swap slot에 있는지 나타내는 플래그

이 상태가 없으면 page를 swap out한 뒤 page fault가 다시 발생했을 때 disk의 어느 위치에서 내용을 읽어야 하는지 알 수 없다.

### `struct frame`

파일: `pintos/include/vm/vm.h`

frame에 `owner`를 추가했다.

```text
frame -> page
frame -> owner thread
```

eviction은 전역 `frame_table`에서 frame을 고르기 때문에 victim frame이 항상 현재 thread의 page라고 볼 수 없다. 따라서 `pml4_clear_page()`를 호출할 때 `thread_current()->pml4`가 아니라 `victim->owner->pml4`를 사용해야 한다.

## 초기화 흐름

파일: `pintos/vm/vm.c`, `pintos/vm/anon.c`

부팅 중 VM subsystem은 다음 순서로 초기화된다.

```text
vm_init()
  -> vm_anon_init()
  -> vm_file_init()
  -> list_init(&frame_table)
```

`vm_anon_init()`에서는 swap subsystem을 준비한다.

```text
vm_anon_init()
  -> disk_get(1, 1)
  -> lock_init(&swap_lock)
  -> bitmap_create(disk_size(swap_disk) / SECTORS_PER_PAGE)
```

swap disk는 sector 단위 장치이고 page는 4096 bytes이다. `DISK_SECTOR_SIZE`가 512 bytes이므로 page 하나는 8 sectors를 차지한다.

```text
SECTORS_PER_PAGE = PGSIZE / DISK_SECTOR_SIZE
                 = 4096 / 512
                 = 8
```

따라서 bitmap의 bit 하나는 swap disk의 8-sector page slot 하나를 의미한다.

## 일반 page fault 처리 흐름

page fault 진입점은 `userprog/exception.c`의 `page_fault()`다.

```text
page_fault()
  -> fault_addr = rcr2()
  -> not_present/write/user 계산
  -> vm_try_handle_fault(f, fault_addr, user, write, not_present)
```

VM이 처리 가능한 fault면 `vm_try_handle_fault()`가 true를 반환하고 user program으로 복귀한다.

`vm_try_handle_fault()`의 핵심 흐름은 다음과 같다.

```text
vm_try_handle_fault()
  -> va = pg_round_down(addr)
  -> spt_find_page(&thread_current()->spt, va)
      -> page가 없고 stack growth 조건이면 vm_stack_growth()
      -> page가 없고 stack growth도 아니면 false
  -> write가 read-only page를 건드렸으면 false
  -> vm_do_claim_page(page)
```

즉 page fault 복구의 실제 작업은 `vm_do_claim_page()`로 모인다.

## frame 확보 흐름

파일: `pintos/vm/vm.c`

`vm_do_claim_page()`는 먼저 `vm_get_frame()`으로 physical frame을 구한다.

```text
vm_do_claim_page(page)
  -> frame = vm_get_frame()
  -> frame->page = page
  -> frame->owner = thread_current()
  -> page->frame = frame
  -> pml4_set_page(current->pml4, page->va, frame->kva, page->writeable)
  -> swap_in(page, frame->kva)
  -> list_push_back(&frame_table, &frame->elem)
```

`vm_get_frame()`은 먼저 새 user frame allocation을 시도한다.

```text
vm_get_frame()
  -> palloc_get_page(PAL_USER)
      -> 성공: 새 struct frame 생성
      -> 실패: vm_evict_frame()
```

성공하면 새 frame을 만들고, 실패하면 기존 frame 하나를 evict해서 재사용한다.

## eviction 흐름

파일: `pintos/vm/vm.c`

현재 victim 선택 정책은 가장 단순한 FIFO다.

```text
vm_evict_frame()
  -> vm_get_victim()
      -> list_pop_front(&frame_table)
  -> page = victim->page
  -> swap_out(page)
  -> pml4_clear_page(victim->owner->pml4, page->va)
  -> page->frame = NULL
  -> victim->page = NULL
  -> victim->owner = NULL
  -> memset(victim->kva, 0, PGSIZE)
  -> return victim
```

여기서 중요한 순서는 `swap_out(page)`가 먼저이고, 그 다음 page table mapping을 제거하는 것이다. mapping을 먼저 지우면 dirty bit 확인이나 frame 내용 write-back에 필요한 정보가 사라질 수 있다.

`swap_out(page)`는 macro라서 실제 호출 대상은 page type에 따라 달라진다.

```text
swap_out(page)
  -> page->operations->swap_out(page)
      -> VM_ANON이면 anon_swap_out()
      -> VM_FILE이면 file_backed_swap_out()
```

## anonymous page swap out

파일: `pintos/vm/anon.c`

anonymous page는 backing file이 없으므로 swap disk에 page 전체를 저장한다.

```text
anon_swap_out(page)
  -> bitmap_scan_and_flip(swap_table, 0, 1, false)
      -> 빈 swap slot 하나 확보
  -> for i in 0..7:
       disk_write(swap_disk, slot * 8 + i, frame->kva + i * 512)
  -> page->anon.swap_slot = slot
  -> page->anon.in_swap = true
```

이후 `vm_evict_frame()`이 `page->frame = NULL`로 바꾸므로, page metadata만 남고 실제 내용은 swap disk에 있다.

## anonymous page swap in

swapped-out anonymous page에 다시 접근하면 page table에는 mapping이 없으므로 page fault가 발생한다.

```text
page_fault()
  -> vm_try_handle_fault()
  -> spt_find_page()
      -> 기존 page metadata 발견
  -> vm_do_claim_page(page)
  -> vm_get_frame()
  -> pml4_set_page()
  -> swap_in(page, frame->kva)
      -> anon_swap_in(page, frame->kva)
```

`anon_swap_in()`은 `in_swap` 상태에 따라 두 가지로 나뉜다.

```text
anon_swap_in(page, kva)
  -> in_swap == false:
       memset(kva, 0, PGSIZE)
  -> in_swap == true:
       for i in 0..7:
         disk_read(swap_disk, swap_slot * 8 + i, kva + i * 512)
       bitmap_reset(swap_table, swap_slot)
       swap_slot = BITMAP_ERROR
       in_swap = false
```

첫 fault에서 새 anonymous page를 claim하는 경우에는 swap disk에 저장된 내용이 없으므로 zero page로 만든다. 반대로 evicted page가 다시 fault되면 `swap_slot`에서 8 sectors를 읽어 frame에 복구한다.

## file-backed page swap out/in

파일: `pintos/vm/file.c`

file-backed page는 anonymous page처럼 swap disk slot을 쓰지 않는다. 원본 file이 backing store 역할을 한다.

### swap out

```text
file_backed_swap_out(page)
  -> pml4 = page->frame->owner->pml4
  -> pml4_is_dirty(pml4, page->va)
      -> dirty면 file_write_at(file, frame->kva, read_bytes, ofs)
      -> dirty bit clear
  -> true
```

dirty가 아니면 file 내용과 memory 내용이 같다고 보고 write-back을 생략한다.

### swap in

```text
file_backed_swap_in(page, kva)
  -> file_read_at(file, kva, read_bytes, ofs)
  -> memset(kva + read_bytes, 0, zero_bytes)
```

이 흐름은 mmap page가 evict된 뒤 다시 접근될 때 사용된다.

## lazy loading과 swap in의 차이

`vm_do_claim_page()`는 항상 `swap_in(page, frame->kva)`를 호출하지만, 실제 의미는 page 상태에 따라 다르다.

### 아직 `VM_UNINIT`인 page

```text
swap_in(page, kva)
  -> uninit_initialize(page, kva)
      -> page_initializer(page, type, kva)
          -> anon_initializer() 또는 file_backed_initializer()
      -> init(page, aux)가 있으면 lazy_load_segment() 등 호출
```

즉 첫 page fault에서는 swap disk에서 복구한다기보다, uninit page를 실제 page type으로 변환하고 lazy loading callback을 실행한다.

### 이미 initialized된 뒤 evicted된 page

```text
swap_in(page, kva)
  -> anon_swap_in() 또는 file_backed_swap_in()
```

이 경우에는 page operation이 이미 `anon_ops` 또는 `file_ops`로 바뀌어 있으므로, disk나 file에서 내용을 복구한다.

## page destroy 흐름

process 종료나 `munmap()` 등으로 SPT entry가 제거되면 다음 흐름으로 page resource를 정리한다.

```text
supplemental_page_table_kill()
  -> hash_destroy(..., spt_hash_destroy)
      -> destroy(page)
      -> destroy_frame_if_exists(page)
      -> free(page)
```

anonymous page destroy는 swap slot 누수를 막는다.

```text
anon_destroy(page)
  -> in_swap == true이면 bitmap_reset(swap_table, swap_slot)
```

file-backed page destroy는 frame이 있으면 dirty 여부를 확인해 write-back하고 file을 닫는다. frame이 이미 evict되어 없으면 write-back할 memory가 없으므로 file만 닫는다.

```text
file_backed_destroy(page)
  -> page->frame != NULL:
       dirty면 file_write_at()
       file_close()
  -> page->frame == NULL:
       file_close()
```

## 전체 호출 흐름 요약

### frame이 충분할 때

```text
page_fault()
  -> vm_try_handle_fault()
  -> vm_do_claim_page()
  -> vm_get_frame()
      -> palloc_get_page(PAL_USER) 성공
  -> pml4_set_page()
  -> swap_in()
  -> frame_table에 frame 등록
```

### frame이 부족할 때

```text
page_fault()
  -> vm_try_handle_fault()
  -> vm_do_claim_page()
  -> vm_get_frame()
      -> palloc_get_page(PAL_USER) 실패
      -> vm_evict_frame()
          -> vm_get_victim()
          -> swap_out(victim->page)
          -> pml4_clear_page(victim->owner->pml4, victim->page->va)
          -> victim frame 초기화
  -> evicted frame 재사용
  -> pml4_set_page()
  -> swap_in()
  -> frame_table에 다시 등록
```

### swapped-out anonymous page에 다시 접근할 때

```text
user memory access
  -> page fault
  -> vm_try_handle_fault()
  -> spt_find_page()로 기존 page 발견
  -> vm_do_claim_page()
  -> vm_get_frame()
  -> pml4_set_page()
  -> anon_swap_in()
      -> disk_read()
      -> swap slot free
```

## 검증한 테스트

Docker 안에서 다음을 확인했다.

```bash
make -C /workspace/pintos/vm
make -C /workspace/pintos/vm build/tests/vm/swap-anon.result
make -C /workspace/pintos/vm build/tests/vm/swap-file.result
make -C /workspace/pintos/vm build/tests/vm/swap-iter.result
make -C /workspace/pintos/vm build/tests/vm/swap-fork.result
```

통과한 테스트는 `swap-anon`, `swap-file`, `swap-iter`, `swap-fork`다. 전체 `vm` test suite는 아직 별도로 돌리지 않았다.

## 남은 주의점

현재 victim 선택은 FIFO라서 구현은 단순하지만, accessed bit를 이용한 clock 계열 정책보다 교체 품질은 낮다. 다만 이번 목표는 최소 수정으로 swap in/out 경로를 연결하는 것이었으므로, 정책 고도화는 별도 작업으로 분리하는 편이 낫다.

또한 frame table 자체에는 별도 lock을 추가하지 않았다. 현재 테스트 범위에서는 통과했지만, 더 강한 동시성 상황까지 고려하려면 `frame_table` 접근을 보호하는 lock을 추가하는 설계를 검토할 수 있다.
