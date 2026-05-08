# Pintos 제공 헬퍼 API 인덱스

이 문서는 현재 저장소의 Pintos 코드에서 공개 header를 통해 사용할 수 있는 함수, 함수형 매크로, 주요 상수를 인덱스 형태로 정리한다.

구체적인 구현 사용 경우나 구현 절차는 의도적으로 적지 않는다. 이 문서는 “무엇이 있는지 찾는 색인”으로만 사용한다.

## 범위

포함:

- `pintos/include` 아래 공개 header에 선언된 함수
- VM 구현과 관련 있는 주요 함수형 매크로
- 자료구조 라이브러리 API
- 현재 저장소에 이미 공개된 project-specific helper

제외:

- `.c` 파일 내부의 `static` helper
- 외부 Pintos 해답이나 블로그 자료
- 함수별 구체적 사용 예시
- Copy-on-write 전용 설명

기준:

- `pintos/include/vm/*.h`
- `pintos/include/threads/*.h`
- `pintos/include/lib/*.h`
- `pintos/include/lib/kernel/*.h`
- `pintos/include/filesys/*.h`
- `pintos/include/devices/*.h`
- `pintos/include/userprog/*.h`

## VM 인터페이스

Header:

- `pintos/include/vm/vm.h`
- `pintos/include/vm/uninit.h`
- `pintos/include/vm/anon.h`
- `pintos/include/vm/file.h`
- `pintos/include/vm/inspect.h`

VM subsystem이 다루는 page, frame, supplemental page table, lazy initialization, anonymous page, file-backed page 관련 interface를 제공한다. 일부 함수는 skeleton에서 선언되어 있고 Project 3 구현 대상이다.

### VM 타입과 매크로

| 이름 | 종류 |
|---|---|
| `enum vm_type` | enum |
| `VM_UNINIT` | enum value |
| `VM_ANON` | enum value |
| `VM_FILE` | enum value |
| `VM_PAGE_CACHE` | enum value |
| `VM_MARKER_0` | enum value |
| `VM_MARKER_1` | enum value |
| `VM_MARKER_END` | enum value |
| `VM_TYPE(type)` | macro |
| `swap_in(page, v)` | macro |
| `swap_out(page)` | macro |
| `destroy(page)` | macro |
| `vm_alloc_page(type, upage, writable)` | macro |

### VM 구조체

| 이름 | 종류 |
|---|---|
| `struct page` | struct |
| `struct frame` | struct |
| `struct page_operations` | struct |
| `struct supplemental_page_table` | struct |
| `struct uninit_page` | struct |
| `struct anon_page` | struct |
| `struct file_page` | struct |
| `typedef bool vm_initializer (struct page *, void *aux)` | function type |

### VM 함수

| 함수 |
|---|
| `void vm_init (void);` |
| `bool vm_try_handle_fault (struct intr_frame *f, void *addr, bool user, bool write, bool not_present);` |
| `bool vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable, vm_initializer *init, void *aux);` |
| `void vm_dealloc_page (struct page *page);` |
| `bool vm_claim_page (void *va);` |
| `enum vm_type page_get_type (struct page *page);` |

### Supplemental Page Table 함수

| 함수 |
|---|
| `void supplemental_page_table_init (struct supplemental_page_table *spt);` |
| `bool supplemental_page_table_copy (struct supplemental_page_table *dst, struct supplemental_page_table *src);` |
| `void supplemental_page_table_kill (struct supplemental_page_table *spt);` |
| `struct page *spt_find_page (struct supplemental_page_table *spt, void *va);` |
| `bool spt_insert_page (struct supplemental_page_table *spt, struct page *page);` |
| `void spt_remove_page (struct supplemental_page_table *spt, struct page *page);` |

### Uninit / Anon / File-backed 함수

| 함수 |
|---|
| `void uninit_new (struct page *page, void *va, vm_initializer *init, enum vm_type type, void *aux, bool (*initializer)(struct page *, enum vm_type, void *kva));` |
| `void vm_anon_init (void);` |
| `bool anon_initializer (struct page *page, enum vm_type type, void *kva);` |
| `void vm_file_init (void);` |
| `bool file_backed_initializer (struct page *page, enum vm_type type, void *kva);` |
| `void *do_mmap (void *addr, size_t length, int writable, struct file *file, off_t offset);` |
| `void do_munmap (void *va);` |
| `void register_inspect_intr (void);` |

## Virtual Address / Page Table

Header:

- `pintos/include/threads/vaddr.h`
- `pintos/include/threads/pte.h`
- `pintos/include/threads/mmu.h`

가상 주소의 page boundary 계산, user/kernel address 구분, x86-64 page table index/flag, PML4 mapping 관리 관련 helper를 제공한다.

### Virtual Address 상수와 매크로

| 이름 | 종류 |
|---|---|
| `BITMASK(SHIFT, CNT)` | macro |
| `PGSHIFT` | constant macro |
| `PGBITS` | constant macro |
| `PGSIZE` | constant macro |
| `PGMASK` | constant macro |
| `pg_ofs(va)` | macro |
| `pg_no(va)` | macro |
| `pg_round_up(va)` | macro |
| `pg_round_down(va)` | macro |
| `pg_next(va)` | macro |
| `KERN_BASE` | constant macro |
| `USER_STACK` | constant macro |
| `is_user_vaddr(vaddr)` | macro |
| `is_kernel_vaddr(vaddr)` | macro |
| `ptov(paddr)` | macro |
| `vtop(vaddr)` | macro |

### PTE 상수와 매크로

| 이름 | 종류 |
|---|---|
| `PML4SHIFT` | constant macro |
| `PDPESHIFT` | constant macro |
| `PDXSHIFT` | constant macro |
| `PTXSHIFT` | constant macro |
| `PML4(la)` | macro |
| `PDPE(la)` | macro |
| `PDX(la)` | macro |
| `PTX(la)` | macro |
| `PTE_ADDR(pte)` | macro |
| `PTE_FLAGS` | constant macro |
| `PTE_ADDR_MASK` | constant macro |
| `PTE_AVL` | constant macro |
| `PTE_P` | constant macro |
| `PTE_W` | constant macro |
| `PTE_U` | constant macro |
| `PTE_A` | constant macro |
| `PTE_D` | constant macro |
| `is_writable(pte)` | macro |
| `is_user_pte(pte)` | macro |
| `is_kern_pte(pte)` | macro |
| `pte_get_paddr(pte)` | macro |

### PML4 / MMU 함수

| 함수 |
|---|
| `uint64_t *pml4e_walk (uint64_t *pml4, const uint64_t va, int create);` |
| `uint64_t *pml4_create (void);` |
| `bool pml4_for_each (uint64_t *, pte_for_each_func *, void *);` |
| `void pml4_destroy (uint64_t *pml4);` |
| `void pml4_activate (uint64_t *pml4);` |
| `void *pml4_get_page (uint64_t *pml4, const void *upage);` |
| `bool pml4_set_page (uint64_t *pml4, void *upage, void *kpage, bool rw);` |
| `void pml4_clear_page (uint64_t *pml4, void *upage);` |
| `bool pml4_is_dirty (uint64_t *pml4, const void *upage);` |
| `void pml4_set_dirty (uint64_t *pml4, const void *upage, bool dirty);` |
| `bool pml4_is_accessed (uint64_t *pml4, const void *upage);` |
| `void pml4_set_accessed (uint64_t *pml4, const void *upage, bool accessed);` |

## Page Allocator / Heap Allocator

Header:

- `pintos/include/threads/palloc.h`
- `pintos/include/threads/malloc.h`

page 단위 allocator와 일반 heap allocator interface를 제공한다. VM에서는 page 단위 메모리와 metadata용 동적 메모리를 구분해서 볼 때 이 섹션을 참고한다.

### Page allocator 상수

| 이름 | 종류 |
|---|---|
| `enum palloc_flags` | enum |
| `PAL_ASSERT` | enum value |
| `PAL_ZERO` | enum value |
| `PAL_USER` | enum value |
| `user_page_limit` | extern variable |

### Page allocator 함수

| 함수 |
|---|
| `uint64_t palloc_init (void);` |
| `void *palloc_get_page (enum palloc_flags);` |
| `void *palloc_get_multiple (enum palloc_flags, size_t page_cnt);` |
| `void palloc_free_page (void *);` |
| `void palloc_free_multiple (void *, size_t page_cnt);` |

### Heap allocator 함수

| 함수 |
|---|
| `void malloc_init (void);` |
| `void *malloc (size_t);` |
| `void *calloc (size_t, size_t);` |
| `void *realloc (void *, size_t);` |
| `void free (void *);` |

## Thread / Process / Interrupt

Header:

- `pintos/include/threads/thread.h`
- `pintos/include/threads/interrupt.h`
- `pintos/include/userprog/process.h`
- `pintos/include/userprog/exception.h`
- `pintos/include/userprog/syscall.h`
- `pintos/include/userprog/gdt.h`
- `pintos/include/userprog/tss.h`

thread lifecycle, user process lifecycle, interrupt frame, syscall/exception 초기화, 현재 저장소의 userprog 보조 helper를 모아 둔 영역이다. VM fault handling과 process address space lifecycle을 확인할 때 같이 봐야 한다.

### Thread 상수와 타입

| 이름 | 종류 |
|---|---|
| `enum thread_status` | enum |
| `THREAD_RUNNING` | enum value |
| `THREAD_READY` | enum value |
| `THREAD_BLOCKED` | enum value |
| `THREAD_DYING` | enum value |
| `typedef int tid_t` | typedef |
| `TID_ERROR` | constant macro |
| `PRI_MIN` | constant macro |
| `PRI_DEFAULT` | constant macro |
| `PRI_MAX` | constant macro |
| `struct thread` | struct |
| `struct child_state` | struct |
| `struct file_descriptor` | struct |
| `typedef void thread_func (void *aux)` | function type |

### Thread 함수

| 함수 |
|---|
| `void thread_init (void);` |
| `void thread_start (void);` |
| `void thread_tick (void);` |
| `void thread_print_stats (void);` |
| `tid_t thread_create (const char *name, int priority, thread_func *, void *);` |
| `void thread_block (void);` |
| `void thread_unblock (struct thread *);` |
| `struct thread *thread_current (void);` |
| `tid_t thread_tid (void);` |
| `const char *thread_name (void);` |
| `void thread_exit (void) NO_RETURN;` |
| `void thread_yield (void);` |
| `void thread_yield_if_needed (void);` |
| `void thread_sleep (int64_t wakeup_tick);` |
| `void threads_wakeup (int64_t ticks);` |
| `int thread_get_priority (void);` |
| `void thread_set_priority (int);` |
| `int thread_get_nice (void);` |
| `void thread_set_nice (int);` |
| `int thread_get_recent_cpu (void);` |
| `int thread_get_load_avg (void);` |
| `void do_iret (struct intr_frame *tf);` |
| `bool cmp_priority_more (const struct list_elem *a, const struct list_elem *b, void *aux UNUSED);` |
| `bool cmp_donors_priority_more (const struct list_elem *a, const struct list_elem *b, void *aux UNUSED);` |
| `void thread_donors_recalc_priorities (void);` |
| `void thread_mlfqs_recalc_priorities (void);` |
| `void thread_mlfqs_incr_recent_cpu (void);` |
| `void thread_mlfqs_recalc_shcd_queue (void);` |

### 현재 저장소의 userprog 보조 함수

| 함수 |
|---|
| `void init_child_state (struct child_state *);` |
| `void child_state_release (struct child_state *);` |
| `struct child_state *child_lookup (tid_t tid);` |
| `int fd_alloc (struct file *);` |
| `struct file *fd_lookup (int);` |
| `bool fd_close (int);` |
| `bool cmp_fd_less (const struct list_elem *a, const struct list_elem *b, void *aux UNUSED);` |

### Interrupt 타입과 함수

| 이름 / 함수 |
|---|
| `enum intr_level` |
| `INTR_OFF` |
| `INTR_ON` |
| `struct gp_registers` |
| `struct intr_frame` |
| `typedef void intr_handler_func (struct intr_frame *);` |
| `enum intr_level intr_get_level (void);` |
| `enum intr_level intr_set_level (enum intr_level);` |
| `enum intr_level intr_enable (void);` |
| `enum intr_level intr_disable (void);` |
| `void intr_init (void);` |
| `void intr_register_ext (uint8_t vec, intr_handler_func *, const char *name);` |
| `void intr_register_int (uint8_t vec, int dpl, enum intr_level, intr_handler_func *, const char *name);` |
| `bool intr_context (void);` |
| `void intr_yield_on_return (void);` |
| `void intr_dump_frame (const struct intr_frame *);` |
| `const char *intr_name (uint8_t vec);` |

### User process / syscall kernel 함수

| 함수 |
|---|
| `tid_t process_create_initd (const char *file_name);` |
| `tid_t process_fork (const char *name, struct intr_frame *if_);` |
| `int process_exec (void *f_name);` |
| `int process_wait (tid_t);` |
| `void process_exit (void);` |
| `void process_activate (struct thread *next);` |
| `void syscall_init (void);` |
| `void exception_init (void);` |
| `void exception_print_stats (void);` |
| `void gdt_init (void);` |
| `void tss_init (void);` |
| `void tss_update (struct thread *next);` |

## Synchronization

Header:

- `pintos/include/threads/synch.h`

semaphore, lock, condition variable, optimization barrier를 제공한다. 여러 thread/process가 공유하는 kernel 자료구조를 보호하거나 event 대기를 표현할 때 쓰이는 기본 동기화 interface다.

### 구조체와 매크로

| 이름 | 종류 |
|---|---|
| `struct semaphore` | struct |
| `struct lock` | struct |
| `struct condition` | struct |
| `barrier()` | macro |

### Semaphore 함수

| 함수 |
|---|
| `void sema_init (struct semaphore *, unsigned value);` |
| `void sema_down (struct semaphore *);` |
| `bool sema_try_down (struct semaphore *);` |
| `void sema_up (struct semaphore *);` |
| `void sema_self_test (void);` |

### Lock 함수

| 함수 |
|---|
| `void lock_init (struct lock *);` |
| `void lock_acquire (struct lock *);` |
| `bool lock_try_acquire (struct lock *);` |
| `void lock_release (struct lock *);` |
| `bool lock_held_by_current_thread (const struct lock *);` |

### Condition variable 함수

| 함수 |
|---|
| `void cond_init (struct condition *);` |
| `void cond_wait (struct condition *, struct lock *);` |
| `void cond_signal (struct condition *, struct lock *);` |
| `void cond_broadcast (struct condition *, struct lock *);` |

## Kernel Data Structures

Header:

- `pintos/include/lib/kernel/list.h`
- `pintos/include/lib/kernel/hash.h`
- `pintos/include/lib/kernel/bitmap.h`

kernel 내부에서 사용할 수 있는 intrusive list, intrusive hash table, bitmap 자료구조를 제공한다. 동적 allocation을 자동으로 해 주는 container가 아니라, element를 구조체 안에 포함시키는 방식의 자료구조가 많다.

### List 매크로와 타입

| 이름 | 종류 |
|---|---|
| `struct list_elem` | struct |
| `struct list` | struct |
| `list_entry(LIST_ELEM, STRUCT, MEMBER)` | macro |
| `typedef bool list_less_func (...)` | function type |

### List 함수

| 함수 |
|---|
| `void list_init (struct list *);` |
| `struct list_elem *list_begin (struct list *);` |
| `struct list_elem *list_next (struct list_elem *);` |
| `struct list_elem *list_end (struct list *);` |
| `struct list_elem *list_rbegin (struct list *);` |
| `struct list_elem *list_prev (struct list_elem *);` |
| `struct list_elem *list_rend (struct list *);` |
| `struct list_elem *list_head (struct list *);` |
| `struct list_elem *list_tail (struct list *);` |
| `void list_insert (struct list_elem *, struct list_elem *);` |
| `void list_splice (struct list_elem *before, struct list_elem *first, struct list_elem *last);` |
| `void list_push_front (struct list *, struct list_elem *);` |
| `void list_push_back (struct list *, struct list_elem *);` |
| `struct list_elem *list_remove (struct list_elem *);` |
| `struct list_elem *list_pop_front (struct list *);` |
| `struct list_elem *list_pop_back (struct list *);` |
| `struct list_elem *list_front (struct list *);` |
| `struct list_elem *list_back (struct list *);` |
| `size_t list_size (struct list *);` |
| `bool list_empty (struct list *);` |
| `void list_reverse (struct list *);` |
| `void list_sort (struct list *, list_less_func *, void *aux);` |
| `void list_insert_ordered (struct list *, struct list_elem *, list_less_func *, void *aux);` |
| `void list_unique (struct list *, struct list *duplicates, list_less_func *, void *aux);` |
| `struct list_elem *list_max (struct list *, list_less_func *, void *aux);` |
| `struct list_elem *list_min (struct list *, list_less_func *, void *aux);` |

### Hash 매크로와 타입

| 이름 | 종류 |
|---|---|
| `struct hash_elem` | struct |
| `struct hash` | struct |
| `struct hash_iterator` | struct |
| `hash_entry(HASH_ELEM, STRUCT, MEMBER)` | macro |
| `typedef uint64_t hash_hash_func (...)` | function type |
| `typedef bool hash_less_func (...)` | function type |
| `typedef void hash_action_func (...)` | function type |

### Hash 함수

| 함수 |
|---|
| `bool hash_init (struct hash *, hash_hash_func *, hash_less_func *, void *aux);` |
| `void hash_clear (struct hash *, hash_action_func *);` |
| `void hash_destroy (struct hash *, hash_action_func *);` |
| `struct hash_elem *hash_insert (struct hash *, struct hash_elem *);` |
| `struct hash_elem *hash_replace (struct hash *, struct hash_elem *);` |
| `struct hash_elem *hash_find (struct hash *, struct hash_elem *);` |
| `struct hash_elem *hash_delete (struct hash *, struct hash_elem *);` |
| `void hash_apply (struct hash *, hash_action_func *);` |
| `void hash_first (struct hash_iterator *, struct hash *);` |
| `struct hash_elem *hash_next (struct hash_iterator *);` |
| `struct hash_elem *hash_cur (struct hash_iterator *);` |
| `size_t hash_size (struct hash *);` |
| `bool hash_empty (struct hash *);` |
| `uint64_t hash_bytes (const void *, size_t);` |
| `uint64_t hash_string (const char *);` |
| `uint64_t hash_int (int);` |

### Bitmap 매크로와 함수

| 이름 / 함수 |
|---|
| `BITMAP_ERROR` |
| `struct bitmap *bitmap_create (size_t bit_cnt);` |
| `struct bitmap *bitmap_create_in_buf (size_t bit_cnt, void *, size_t byte_cnt);` |
| `size_t bitmap_buf_size (size_t bit_cnt);` |
| `void bitmap_destroy (struct bitmap *);` |
| `size_t bitmap_size (const struct bitmap *);` |
| `void bitmap_set (struct bitmap *, size_t idx, bool);` |
| `void bitmap_mark (struct bitmap *, size_t idx);` |
| `void bitmap_reset (struct bitmap *, size_t idx);` |
| `void bitmap_flip (struct bitmap *, size_t idx);` |
| `bool bitmap_test (const struct bitmap *, size_t idx);` |
| `void bitmap_set_all (struct bitmap *, bool);` |
| `void bitmap_set_multiple (struct bitmap *, size_t start, size_t cnt, bool);` |
| `size_t bitmap_count (const struct bitmap *, size_t start, size_t cnt, bool);` |
| `bool bitmap_contains (const struct bitmap *, size_t start, size_t cnt, bool);` |
| `bool bitmap_any (const struct bitmap *, size_t start, size_t cnt);` |
| `bool bitmap_none (const struct bitmap *, size_t start, size_t cnt);` |
| `bool bitmap_all (const struct bitmap *, size_t start, size_t cnt);` |
| `size_t bitmap_scan (const struct bitmap *, size_t start, size_t cnt, bool);` |
| `size_t bitmap_scan_and_flip (struct bitmap *, size_t start, size_t cnt, bool);` |
| `size_t bitmap_file_size (const struct bitmap *);` |
| `bool bitmap_read (struct bitmap *, struct file *);` |
| `bool bitmap_write (const struct bitmap *, struct file *);` |
| `void bitmap_dump (const struct bitmap *);` |

## File System / File / Inode

Header:

- `pintos/include/filesys/filesys.h`
- `pintos/include/filesys/file.h`
- `pintos/include/filesys/inode.h`
- `pintos/include/filesys/directory.h`
- `pintos/include/filesys/free-map.h`
- `pintos/include/filesys/fat.h`
- `pintos/include/filesys/page_cache.h`
- `pintos/include/filesys/fsutil.h`

file system, open file object, inode, directory, free map, FAT 관련 interface를 제공한다. VM의 file-backed page나 mmap 구현을 이해할 때 file object와 inode 계층을 구분해서 볼 수 있다.

### File system 전역과 상수

| 이름 | 종류 |
|---|---|
| `FREE_MAP_SECTOR` | constant macro |
| `ROOT_DIR_SECTOR` | constant macro |
| `filesys_disk` | extern variable |
| `filesys_lock` | extern variable |
| `NAME_MAX` | constant macro |
| `FAT_MAGIC` | constant macro |
| `EOChain` | constant macro |
| `SECTORS_PER_CLUSTER` | constant macro |
| `FAT_BOOT_SECTOR` | constant macro |
| `ROOT_DIR_CLUSTER` | constant macro |
| `typedef uint32_t cluster_t` | typedef |
| `struct page_cache` | struct |

### Filesys 함수

| 함수 |
|---|
| `void filesys_init (bool format);` |
| `void filesys_done (void);` |
| `bool filesys_create (const char *name, off_t initial_size);` |
| `struct file *filesys_open (const char *name);` |
| `bool filesys_remove (const char *name);` |

### File 함수

| 함수 |
|---|
| `struct file *file_open (struct inode *);` |
| `struct file *file_reopen (struct file *);` |
| `struct file *file_duplicate (struct file *file);` |
| `void file_close (struct file *);` |
| `struct inode *file_get_inode (struct file *);` |
| `off_t file_read (struct file *, void *, off_t);` |
| `off_t file_read_at (struct file *, void *, off_t size, off_t start);` |
| `off_t file_write (struct file *, const void *, off_t);` |
| `off_t file_write_at (struct file *, const void *, off_t size, off_t start);` |
| `void file_deny_write (struct file *);` |
| `void file_allow_write (struct file *);` |
| `void file_seek (struct file *, off_t);` |
| `off_t file_tell (struct file *);` |
| `off_t file_length (struct file *);` |

### Inode 함수

| 함수 |
|---|
| `void inode_init (void);` |
| `bool inode_create (disk_sector_t, off_t);` |
| `struct inode *inode_open (disk_sector_t);` |
| `struct inode *inode_reopen (struct inode *);` |
| `disk_sector_t inode_get_inumber (const struct inode *);` |
| `void inode_close (struct inode *);` |
| `void inode_remove (struct inode *);` |
| `off_t inode_read_at (struct inode *, void *, off_t size, off_t offset);` |
| `off_t inode_write_at (struct inode *, const void *, off_t size, off_t offset);` |
| `void inode_deny_write (struct inode *);` |
| `void inode_allow_write (struct inode *);` |
| `off_t inode_length (const struct inode *);` |

### Directory 함수

| 함수 |
|---|
| `bool dir_create (disk_sector_t sector, size_t entry_cnt);` |
| `struct dir *dir_open (struct inode *);` |
| `struct dir *dir_open_root (void);` |
| `struct dir *dir_reopen (struct dir *);` |
| `void dir_close (struct dir *);` |
| `struct inode *dir_get_inode (struct dir *);` |
| `bool dir_lookup (const struct dir *, const char *name, struct inode **);` |
| `bool dir_add (struct dir *, const char *name, disk_sector_t);` |
| `bool dir_remove (struct dir *, const char *name);` |
| `bool dir_readdir (struct dir *, char name[NAME_MAX + 1]);` |

### Free map / FAT / Page cache 함수

| 함수 |
|---|
| `void free_map_init (void);` |
| `void free_map_read (void);` |
| `void free_map_create (void);` |
| `void free_map_open (void);` |
| `void free_map_close (void);` |
| `bool free_map_allocate (size_t, disk_sector_t *);` |
| `void free_map_release (disk_sector_t, size_t);` |
| `void fat_init (void);` |
| `void fat_open (void);` |
| `void fat_close (void);` |
| `void fat_create (void);` |
| `cluster_t fat_create_chain (cluster_t clst);` |
| `void fat_remove_chain (cluster_t clst, cluster_t pclst);` |
| `cluster_t fat_get (cluster_t clst);` |
| `void fat_put (cluster_t clst, cluster_t val);` |
| `disk_sector_t cluster_to_sector (cluster_t clst);` |
| `void page_cache_init (void);` |
| `bool page_cache_initializer (struct page *page, enum vm_type type, void *kva);` |

### File system utility 함수

| 함수 |
|---|
| `void fsutil_ls (char **argv);` |
| `void fsutil_cat (char **argv);` |
| `void fsutil_rm (char **argv);` |
| `void fsutil_put (char **argv);` |
| `void fsutil_get (char **argv);` |

## Disk / Device / Timer

Header:

- `pintos/include/devices/disk.h`
- `pintos/include/devices/input.h`
- `pintos/include/devices/timer.h`
- `pintos/include/devices/serial.h`
- `pintos/include/devices/kbd.h`
- `pintos/include/devices/intq.h`
- `pintos/include/devices/vga.h`

disk sector I/O, timer, console input/output 장치, serial, keyboard, interrupt queue 같은 device-level interface를 제공한다. VM에서는 swap disk나 테스트 실행 로그를 이해할 때 관련될 수 있다.

### Disk 상수와 함수

| 이름 / 함수 |
|---|
| `DISK_SECTOR_SIZE` |
| `typedef uint32_t disk_sector_t` |
| `PRDSNu` |
| `void disk_init (void);` |
| `void disk_print_stats (void);` |
| `struct disk *disk_get (int chan_no, int dev_no);` |
| `disk_sector_t disk_size (struct disk *);` |
| `void disk_read (struct disk *, disk_sector_t, void *);` |
| `void disk_write (struct disk *, disk_sector_t, const void *);` |
| `void register_disk_inspect_intr ();` |

### Timer 함수

| 이름 / 함수 |
|---|
| `TIMER_FREQ` |
| `void timer_init (void);` |
| `void timer_calibrate (void);` |
| `int64_t timer_ticks (void);` |
| `int64_t timer_elapsed (int64_t);` |
| `void timer_sleep (int64_t ticks);` |
| `void timer_msleep (int64_t milliseconds);` |
| `void timer_usleep (int64_t microseconds);` |
| `void timer_nsleep (int64_t nanoseconds);` |
| `void timer_print_stats (void);` |

### Input / Serial / Keyboard / VGA / Interrupt Queue 함수

| 함수 |
|---|
| `void input_init (void);` |
| `void input_putc (uint8_t);` |
| `uint8_t input_getc (void);` |
| `bool input_full (void);` |
| `void serial_init_queue (void);` |
| `void serial_putc (uint8_t);` |
| `void serial_flush (void);` |
| `void serial_notify (void);` |
| `void kbd_init (void);` |
| `void kbd_print_stats (void);` |
| `void vga_putc (int);` |
| `void intq_init (struct intq *);` |
| `bool intq_empty (const struct intq *);` |
| `bool intq_full (const struct intq *);` |
| `uint8_t intq_getc (struct intq *);` |
| `void intq_putc (struct intq *, uint8_t);` |

## C Library / Kernel Library

Header:

- `pintos/include/lib/string.h`
- `pintos/include/lib/stdio.h`
- `pintos/include/lib/stdlib.h`
- `pintos/include/lib/ctype.h`
- `pintos/include/lib/random.h`
- `pintos/include/lib/round.h`
- `pintos/include/lib/debug.h`
- `pintos/include/lib/kernel/stdio.h`
- `pintos/include/lib/kernel/console.h`

Pintos kernel/user 공통 C library와 kernel 출력/debug helper를 제공한다. 표준 C library 전체가 아니라 Pintos가 제공하는 제한된 subset과 안전한 대체 함수들을 기준으로 본다.

### String / memory 함수

| 함수 |
|---|
| `void *memcpy (void *, const void *, size_t);` |
| `void *memmove (void *, const void *, size_t);` |
| `char *strncat (char *, const char *, size_t);` |
| `int memcmp (const void *, const void *, size_t);` |
| `int strcmp (const char *, const char *);` |
| `void *memchr (const void *, int, size_t);` |
| `char *strchr (const char *, int);` |
| `size_t strcspn (const char *, const char *);` |
| `char *strpbrk (const char *, const char *);` |
| `char *strrchr (const char *, int);` |
| `size_t strspn (const char *, const char *);` |
| `char *strstr (const char *, const char *);` |
| `void *memset (void *, int, size_t);` |
| `size_t strlen (const char *);` |
| `size_t strlcpy (char *, const char *, size_t);` |
| `size_t strlcat (char *, const char *, size_t);` |
| `char *strtok_r (char *, const char *, char **);` |
| `size_t strnlen (const char *, size_t);` |

### String 금지 치환 매크로

| 이름 |
|---|
| `strcpy` |
| `strncpy` |
| `strcat` |
| `strncat` |
| `strtok` |

### Stdio 함수와 매크로

| 이름 / 함수 |
|---|
| `STDIN_FILENO` |
| `STDOUT_FILENO` |
| `int printf (const char *, ...);` |
| `int snprintf (char *, size_t, const char *, ...);` |
| `int vprintf (const char *, va_list);` |
| `int vsnprintf (char *, size_t, const char *, va_list);` |
| `int putchar (int);` |
| `int puts (const char *);` |
| `void hex_dump (uintptr_t ofs, const void *, size_t size, bool ascii);` |
| `void __vprintf (const char *format, va_list args, void (*output) (char, void *), void *aux);` |
| `void __printf (const char *format, void (*output) (char, void *), void *aux, ...);` |
| `void putbuf (const char *, size_t);` |
| `void console_init (void);` |
| `void console_panic (void);` |
| `void console_print_stats (void);` |

### Stdio 금지 치환 매크로

| 이름 |
|---|
| `sprintf` |
| `vsprintf` |

### Stdlib 함수

| 함수 |
|---|
| `int atoi (const char *);` |
| `void qsort (void *array, size_t cnt, size_t size, int (*compare) (const void *, const void *));` |
| `void *bsearch (const void *key, const void *array, size_t cnt, size_t size, int (*compare) (const void *, const void *));` |
| `void sort (void *array, size_t cnt, size_t size, int (*compare) (const void *, const void *, void *aux), void *aux);` |
| `void *binary_search (const void *key, const void *array, size_t cnt, size_t size, int (*compare) (const void *, const void *, void *aux), void *aux);` |

### Ctype inline 함수

| 함수 |
|---|
| `static inline int islower (int c);` |
| `static inline int isupper (int c);` |
| `static inline int isalpha (int c);` |
| `static inline int isdigit (int c);` |
| `static inline int isalnum (int c);` |
| `static inline int isxdigit (int c);` |
| `static inline int isspace (int c);` |
| `static inline int isblank (int c);` |
| `static inline int isgraph (int c);` |
| `static inline int isprint (int c);` |
| `static inline int iscntrl (int c);` |
| `static inline int isascii (int c);` |
| `static inline int ispunct (int c);` |
| `static inline int tolower (int c);` |
| `static inline int toupper (int c);` |

### Random / Round / Debug

| 이름 / 함수 |
|---|
| `void random_init (unsigned seed);` |
| `void random_bytes (void *, size_t);` |
| `unsigned long random_ulong (void);` |
| `ROUND_UP(X, STEP)` |
| `DIV_ROUND_UP(X, STEP)` |
| `ROUND_DOWN(X, STEP)` |
| `UNUSED` |
| `NO_RETURN` |
| `NO_INLINE` |
| `PRINTF_FORMAT(FMT, FIRST)` |
| `PANIC(...)` |
| `ASSERT(CONDITION)` |
| `NOT_REACHED()` |
| `void debug_panic (const char *file, int line, const char *function, const char *message, ...);` |
| `void debug_backtrace (void);` |

## Fixed-point Helpers

Header:

- `pintos/include/threads/fixed-point.h`

17.14 fixed-point arithmetic helper를 제공한다. 주로 scheduler 쪽 계산을 위해 있는 영역이며, VM API는 아니지만 현재 저장소에서 제공되는 일반 helper로 색인에 포함한다.

| 이름 / 함수 |
|---|
| `FP_F` |
| `typedef int fp32_t` |
| `static inline fp32_t fp (const int n);` |
| `static inline int fp_int_trunc (const fp32_t x);` |
| `static inline int fp_int_rnd (const fp32_t x);` |
| `static inline fp32_t fp_add (const fp32_t x, const fp32_t y);` |
| `static inline fp32_t fp_sub (const fp32_t x, const fp32_t y);` |
| `static inline fp32_t fp_add_i (const fp32_t x, const int n);` |
| `static inline fp32_t fp_sub_i (const fp32_t x, const int n);` |
| `static inline fp32_t fp_mul (const fp32_t x, const fp32_t y);` |
| `static inline fp32_t fp_mul_i (const fp32_t x, const int n);` |
| `static inline fp32_t fp_div (const fp32_t x, const fp32_t y);` |
| `static inline fp32_t fp_div_i (const fp32_t x, const int n);` |

## Low-level Intrinsic / I/O

Header:

- `pintos/include/intrinsic.h`
- `pintos/include/threads/io.h`

CPU register, descriptor table, TLB invalidation, port I/O 같은 낮은 수준의 inline helper를 제공한다. 대부분 architecture-specific helper라서 직접 사용할 때는 header와 호출 맥락을 반드시 같이 확인해야 한다.

### Intrinsic inline 함수

| 함수 |
|---|
| `static inline void lcr3 (uint64_t val);` |
| `static inline void lgdt (const struct desc_ptr *dtr);` |
| `static inline void lldt (uint16_t sel);` |
| `static inline void ltr (uint16_t sel);` |
| `static inline void lidt (const struct desc_ptr *dtr);` |
| `static inline void invlpg (uint64_t addr);` |
| `static inline uint64_t read_eflags (void);` |
| `static inline uint64_t rcr3 (void);` |
| `static inline uint64_t rrax (void);` |
| `static inline uint64_t rrdi (void);` |
| `static inline uint64_t rrsi (void);` |
| `static inline uint64_t rrdx (void);` |
| `static inline uint64_t rr10 (void);` |
| `static inline uint64_t rr8 (void);` |
| `static inline uint64_t rr9 (void);` |
| `static inline uint64_t rrcx (void);` |
| `static inline uint64_t rrsp (void);` |
| `static inline uint64_t rcr2 (void);` |
| `static inline void write_msr (uint32_t ecx, uint64_t val);` |

### Port I/O inline 함수

| 함수 |
|---|
| `static inline uint8_t inb (uint16_t port);` |
| `static inline void insb (uint16_t port, void *addr, size_t cnt);` |
| `static inline uint16_t inw (uint16_t port);` |
| `static inline void insw (uint16_t port, void *addr, size_t cnt);` |
| `static inline uint32_t inl (uint16_t port);` |
| `static inline void insl (uint16_t port, void *addr, size_t cnt);` |
| `static inline void outb (uint16_t port, uint8_t data);` |
| `static inline void outsb (uint16_t port, const void *addr, size_t cnt);` |
| `static inline void outw (uint16_t port, uint16_t data);` |
| `static inline void outsw (uint16_t port, const void *addr, size_t cnt);` |
| `static inline void outl (uint16_t port, uint32_t data);` |
| `static inline void outsl (uint16_t port, const void *addr, size_t cnt);` |

## User-space Library API

Header:

- `pintos/include/lib/user/syscall.h`
- `pintos/include/lib/user/stdio.h`

이 섹션은 user program에서 보이는 API 색인이다.
kernel 내부 helper가 아니라 user program이 syscall wrapper나 user stdio를 통해 호출하는 interface다.

| 이름 / 함수 |
|---|
| `pid_t` |
| `MAP_FAILED` |
| `void halt (void) NO_RETURN;` |
| `void exit (int status) NO_RETURN;` |
| `pid_t fork (const char *thread_name);` |
| `int exec (const char *file);` |
| `int wait (pid_t);` |
| `bool create (const char *file, unsigned initial_size);` |
| `bool remove (const char *file);` |
| `int open (const char *file);` |
| `int filesize (int fd);` |
| `int read (int fd, void *buffer, unsigned length);` |
| `int write (int fd, const void *buffer, unsigned length);` |
| `void seek (int fd, unsigned position);` |
| `unsigned tell (int fd);` |
| `void close (int fd);` |
| `int dup2 (int oldfd, int newfd);` |
| `void *mmap (void *addr, size_t length, int writable, int fd, off_t offset);` |
| `void munmap (void *addr);` |
| `bool chdir (const char *dir);` |
| `bool mkdir (const char *dir);` |
| `bool readdir (int fd, char name[READDIR_MAX_LEN + 1]);` |
| `bool isdir (int fd);` |
| `int inumber (int fd);` |
| `int symlink (const char *target, const char *linkpath);` |
| `int hprintf (int, const char *, ...);` |
| `int vhprintf (int, const char *, va_list);` |

## 주의 표시

이 인덱스에는 다음 성격의 항목이 섞여 있다.

- 이미 구현되어 제공되는 library/helper
- skeleton에 선언되어 있으나 과제에서 구현해야 하는 interface
- Project 4 또는 extra 기능과 관련된 API
- 현재 저장소에서 userprog 진행 중 추가된 helper

따라서 실제 사용할 때는 반드시 해당 header와 현재 구현 파일을 같이 확인한다. 이 문서는 사용 가능성 판단을 대신하지 않고, 이름을 찾기 위한 색인이다.
