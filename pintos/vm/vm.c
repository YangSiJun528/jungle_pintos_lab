/* vm.c: Generic interface for virtual memory objects. */
/* vm.c: 가상 메모리 오브젝트를 위한 공통 인터페이스. */

#include "vm/vm.h"
#include <string.h>
#include "threads/malloc.h"
#include "threads/mmu.h"
#include "threads/vaddr.h"
#include "userprog/process.h"
#include "vm/inspect.h"

static struct list frame_table;

static struct frame *vm_get_victim (void);
static bool vm_do_claim_page (struct page *page);
static struct frame *vm_evict_frame (void);
static struct frame *vm_get_frame (void);
static void vm_stack_growth (void *addr);
static bool vm_handle_wp (struct page *page);

static bool spt_hash_less (const struct hash_elem *a,
		const struct hash_elem *b, void *aux UNUSED);
static uint64_t spt_hash (const struct hash_elem *e, void *aux UNUSED);
static void spt_hash_destroy (struct hash_elem *e, void *aux UNUSED);

/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
/* 각 서브시스템의 초기화 코드를 호출해 가상 메모리 서브시스템을 초기화한다. */
void
vm_init (void) {
	vm_anon_init ();
	vm_file_init ();
#ifdef EFILESYS  /* For project 4 */
	/* project 4용. */
	pagecache_init ();
#endif
	register_inspect_intr ();
	/* DO NOT MODIFY UPPER LINES. */
	/* 위 라인들은 수정하지 않는다. */
	list_init (&frame_table);
}

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
/* 페이지의 타입을 얻는다. 이 함수는 페이지가 초기화된 뒤의 타입을 알고 싶을 때
 * 유용하다.
 * 이 함수는 이미 완전히 구현되어 있다. */
enum vm_type
page_get_type (struct page *page) {
	int ty = VM_TYPE (page->operations->type);
	switch (ty) {
		case VM_UNINIT:
			return VM_TYPE (page->uninit.type);
		default:
			return ty;
	}
}

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
/* 이니셜라이저가 있는 pending 페이지 오브젝트를 만든다. 페이지를 만들고 싶다면
 * 직접 만들지 말고 이 함수나 `vm_alloc_page`를 통해 만든다. */
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) {
	struct supplemental_page_table *spt = &thread_current ()->spt;

	ASSERT (pg_round_down (upage) == upage);
	ASSERT (VM_TYPE (type) != VM_UNINIT);

	if (spt_find_page (spt, upage) != NULL) {
		PANIC ("page found in vm_alloc_init");
	}

	struct page *new_page = malloc (sizeof (struct page));
	if (new_page == NULL) {
		PANIC ("out of memory - new_page");
	}

	switch (VM_TYPE (type)) {
		case VM_ANON:
			uninit_new (new_page, upage, init, VM_ANON, aux,
					anon_initializer);
			break;
		case VM_FILE:
			uninit_new (new_page, upage, init, VM_FILE, aux,
					file_backed_initializer);
			break;
#ifdef EFILESYS  /* For project 4 */
		case VM_PAGE_CACHE:
			uninit_new (new_page, upage, init, VM_PAGE_CACHE, aux,
					page_cache_initializer);
			break;
#endif
		default:
			PANIC ("Unsupported VM Type(%d)", VM_TYPE (type));
			break;
	}

	new_page->writeable = writable;
	new_page->mmaped_size = 0;

	spt_insert_page (spt, new_page);
	return true;
}

/* Find VA from spt and return page. On error, return NULL. */
/* spt에서 VA에 해당하는 페이지를 찾아 리턴한다. 에러가 있으면 NULL을 리턴한다. */
struct page *
spt_find_page (struct supplemental_page_table *spt, void *va) {
	ASSERT ((uintptr_t) va == (uintptr_t) pg_round_down (va));

	struct page page = {
		.va = va,
	};
	struct hash_elem *e = hash_find (&spt->table, &page.elem);

	return e == NULL ? NULL : hash_entry (e, struct page, elem);
}

/* Insert PAGE into spt with validation. */
/* 검증을 거쳐 PAGE를 spt에 삽입한다. */
bool
spt_insert_page (struct supplemental_page_table *spt, struct page *page) {
	ASSERT (spt != NULL);
	ASSERT (page != NULL);

	struct hash_elem *old = hash_insert (&spt->table, &page->elem);

	ASSERT (old == NULL);
	return true;
}

void
spt_remove_page (struct supplemental_page_table *spt, struct page *page) {
	ASSERT (spt != NULL);
	ASSERT (page != NULL);

	struct hash_elem *e = hash_delete (&spt->table, &page->elem);

	ASSERT (e != NULL);
	spt_hash_destroy (e, NULL);
}

/* Get the struct frame, that will be evicted. */
/* evict될 struct frame을 얻는다. */
static struct frame *
vm_get_victim (void) {
	struct frame *victim = NULL;
	/* TODO: The policy for eviction is up to you. */
	/* TODO: eviction 정책은 직접 정한다. */

	return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
/* 페이지 하나를 evict하고 그에 대응하는 프레임을 리턴한다.
 * 에러가 있으면 NULL을 리턴한다. */
static struct frame *
vm_evict_frame (void) {
	struct frame *victim UNUSED = vm_get_victim ();
	/* TODO: swap out the victim and return the evicted frame. */
	/* TODO: victim을 swap out하고 evict된 프레임을 리턴한다. */

	return NULL;
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
/* palloc()으로 프레임을 얻는다. 사용 가능한 페이지가 없으면 페이지를 evict해서
 * 리턴한다. 이 함수는 항상 유효한 주소를 리턴한다. 즉, 유저 풀 메모리가 가득
 * 차 있으면 프레임을 evict해서 사용 가능한 메모리 공간을 얻는다. */
static struct frame *
vm_get_frame (void) {
	struct frame *frame;
	void *kva = palloc_get_page (PAL_USER);

	if (kva == NULL) {
		PANIC ("todo - need_evict");
	}

	frame = malloc (sizeof *frame);
	if (frame == NULL) {
		PANIC ("out of memory - frame");
	}

	frame->kva = kva;
	frame->page = NULL;

	memset (frame->kva, 0, PGSIZE);

	ASSERT (frame != NULL);
	ASSERT (frame->page == NULL);
	return frame;
}

/* Growing the stack. */
/* 스택을 확장한다. */
static void
vm_stack_growth (void *addr) {
	if (!vm_alloc_page (VM_ANON | VM_MARKER_0, addr, true)) {
		goto panic;
	}
	if (!vm_claim_page (addr)) {
		goto panic;
	}

	return;
panic:
	PANIC ("FAILED vm_stack_growth");
}

/* Handle the fault on write_protected page */
/* write-protected 페이지에서 발생한 fault를 처리한다. */
static bool
vm_handle_wp (struct page *page UNUSED) {
	return false;
}

/* Return true on success */
/* 성공하면 true를 리턴한다. */
bool
vm_try_handle_fault (struct intr_frame *f, void *addr, bool user,
		bool write, bool not_present) {
	struct supplemental_page_table *spt = &thread_current ()->spt;
	struct page *page = NULL;
	void *va = pg_round_down (addr);

	page = spt_find_page (spt, va);
	if (page == NULL) {
		uintptr_t rsp = user ? f->rsp : thread_current ()->rsp_at_syscall;
		bool is_valid_stack_area = validate_stack_area (rsp, addr);

		if (is_valid_stack_area) {
			vm_stack_growth (va);
			return true;
		}
		return false;
	}

	if (!not_present)
		return write ? vm_handle_wp (page) : false;
	if (write && !page->writeable)
		return false;

	return vm_do_claim_page (page);
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
/* 페이지를 해제한다.
 * 이 함수는 수정하지 않는다. */
void
vm_dealloc_page (struct page *page) {
	destroy (page);
	free (page);
}

/* Claim the page that allocate on VA. */
/* VA에 할당된 페이지를 claim한다. */
bool
vm_claim_page (void *va) {
	struct page *page = spt_find_page (&thread_current ()->spt, va);

	ASSERT (page != NULL);
	return vm_do_claim_page (page);
}

/* Claim the PAGE and set up the mmu. */
/* PAGE를 claim하고 mmu를 설정한다. */
static bool
vm_do_claim_page (struct page *page) {
	struct frame *frame = vm_get_frame ();
	list_push_back (&frame_table, &frame->elem);

	frame->page = page;
	page->frame = frame;

	pml4_set_page (thread_current ()->pml4, page->va, frame->kva,
			page->writeable);

	swap_in (page, frame->kva);
	return true;
}

/* Initialize new supplemental page table */
/* 새 supplemental page table을 초기화한다. */
void
supplemental_page_table_init (struct supplemental_page_table *spt) {
	hash_init (&spt->table, spt_hash, spt_hash_less, NULL);
}

/* Copy supplemental page table from src to dst */
/* supplemental page table을 src에서 dst로 복사한다. */
bool
supplemental_page_table_copy (struct supplemental_page_table *dst,
		struct supplemental_page_table *src) {
	struct hash_iterator i;

	hash_first (&i, &src->table);
	while (hash_next (&i)) {
		struct page *src_page = hash_entry (hash_cur (&i), struct page, elem);
		void *uva = src_page->va;

		if (VM_TYPE (src_page->operations->type) == VM_UNINIT) {
			struct page_lazy_load_aux *src_aux = src_page->uninit.aux;
			struct page_lazy_load_aux *dst_aux = malloc (sizeof *dst_aux);

			ASSERT (dst_aux != NULL);

			memcpy (dst_aux, src_aux, sizeof *src_aux);
			ASSERT (dst_aux->file != NULL);

			vm_alloc_page_with_initializer (page_get_type (src_page), uva,
					src_page->writeable, src_page->uninit.init, dst_aux);
		} else {
			ASSERT (src_page->frame != NULL);
			void *src_kva = src_page->frame->kva;

			vm_alloc_page (page_get_type (src_page), uva,
					src_page->writeable);
			vm_claim_page (uva);
			struct page *dst_page = spt_find_page (dst, uva);

			ASSERT (dst_page != NULL);
			ASSERT (dst_page->frame != NULL);
			memcpy (dst_page->frame->kva, src_kva, PGSIZE);
		}
	}
	return true;
}

/* Free the resource hold by the supplemental page table */
/* supplemental page table이 들고 있는 리소스를 해제한다. */
void
supplemental_page_table_kill (struct supplemental_page_table *spt) {
	hash_destroy (&spt->table, spt_hash_destroy);
}

void
destroy_frame_if_exists (struct page *page) {
	if (page->frame == NULL)
		return;

	list_remove (&page->frame->elem);
	pml4_clear_page (thread_current ()->pml4, page->va);
	palloc_free_page (page->frame->kva);
	free (page->frame);
}

bool
validate_stack_area (uintptr_t rsp, void *addr) {
	void *va = pg_round_down (addr);
	uintptr_t fault = (uintptr_t) addr;
	bool is_in_stack_area =
		(uintptr_t) va < USER_STACK && MIN_USER_STACK < (uintptr_t) va;
	bool is_cmd_push = rsp == fault + 8;
	bool is_btw_rsp = rsp < fault;

	return is_in_stack_area && (is_cmd_push || is_btw_rsp);
}

static bool
spt_hash_less (const struct hash_elem *a, const struct hash_elem *b,
		void *aux UNUSED) {
	const struct page *pa = hash_entry (a, struct page, elem);
	const struct page *pb = hash_entry (b, struct page, elem);

	return (uintptr_t) pa->va < (uintptr_t) pb->va;
}

static uint64_t
spt_hash (const struct hash_elem *e, void *aux UNUSED) {
	const struct page *page = hash_entry (e, struct page, elem);

	return hash_bytes (&page->va, sizeof page->va);
}

static void
spt_hash_destroy (struct hash_elem *e, void *aux UNUSED) {
	struct page *page = hash_entry (e, struct page, elem);

	destroy_frame_if_exists (page);
	vm_dealloc_page (page);
}
