#ifndef VM_VM_H
#define VM_VM_H
#include <stdbool.h>
#include "threads/palloc.h"

enum vm_type {
	/* page not initialized */
	/* 아직 초기화되지 않은 페이지. */
	VM_UNINIT = 0,
	/* page not related to the file, aka anonymous page */
	/* 파일과 관련 없는 페이지, 즉 anonymous page. */
	VM_ANON = 1,
	/* page that realated to the file */
	/* 파일과 관련된 페이지. */
	VM_FILE = 2,
	/* page that hold the page cache, for project 4 */
	/* page cache를 담는 페이지, project 4용. */
	VM_PAGE_CACHE = 3,

	/* Bit flags to store state */
	/* 상태를 저장하기 위한 bit flag. */

	/* Auxillary bit flag marker for store information. You can add more
	 * markers, until the value is fit in the int. */
	/* 정보를 저장하기 위한 auxiliary bit flag marker. 값이 int에 들어맞는 범위라면
	 * marker를 더 추가할 수 있다. */
	VM_MARKER_0 = (1 << 3),
	VM_MARKER_1 = (1 << 4),

	/* DO NOT EXCEED THIS VALUE. */
	/* 이 값을 넘기지 않는다. */
	VM_MARKER_END = (1 << 31),
};

#include "vm/uninit.h"
#include "vm/anon.h"
#include "vm/file.h"
#ifdef EFILESYS
#include "filesys/page_cache.h"
#endif

struct page_operations;
struct thread;

#define VM_TYPE(type) ((type) & 7)

/* The representation of "page".
 * This is kind of "parent class", which has four "child class"es, which are
 * uninit_page, file_page, anon_page, and page cache (project4).
 * DO NOT REMOVE/MODIFY PREDEFINED MEMBER OF THIS STRUCTURE. */
/* "page"의 표현.
 * 일종의 "parent class"이며 uninit_page, file_page, anon_page,
 * page cache(project4)라는 네 가지 "child class"를 가진다.
 * 이 구조체에 미리 정의된 멤버를 제거하거나 수정하지 않는다. */
struct page {
	const struct page_operations *operations;
	void *va;              /* Address in terms of user space */
	/* 유저 공간 기준 주소. */
	struct frame *frame;   /* Back reference for frame */
	/* 프레임으로 되돌아가는 back reference. */

	/* Your implementation */
	/* 직접 구현할 부분. */

	/* Per-type data are binded into the union.
	 * Each function automatically detects the current union */
	/* 타입별 데이터는 union에 묶여 있다.
	 * 각 함수는 현재 union을 자동으로 감지한다. */
	union {
		struct uninit_page uninit;
		struct anon_page anon;
		struct file_page file;
#ifdef EFILESYS
		struct page_cache page_cache;
#endif
	};
};

/* The representation of "frame" */
/* "frame"의 표현. */
struct frame {
	void *kva;
	struct page *page;
};

/* The function table for page operations.
 * This is one way of implementing "interface" in C.
 * Put the table of "method" into the struct's member, and
 * call it whenever you needed. */
/* page operation을 위한 함수 테이블.
 * C에서 "interface"를 구현하는 한 가지 방식이다.
 * "method" 테이블을 struct의 멤버에 넣고 필요할 때 호출한다. */
struct page_operations {
	bool (*swap_in) (struct page *, void *);
	bool (*swap_out) (struct page *);
	void (*destroy) (struct page *);
	enum vm_type type;
};

#define swap_in(page, v) (page)->operations->swap_in ((page), v)
#define swap_out(page) (page)->operations->swap_out (page)
#define destroy(page) \
	if ((page)->operations->destroy) (page)->operations->destroy (page)

/* Representation of current process's memory space.
 * We don't want to force you to obey any specific design for this struct.
 * All designs up to you for this. */
/* 현재 프로세스 메모리 공간의 표현.
 * 이 struct에 대해 특정 design을 강제하지 않는다.
 * 여기서는 모든 design을 직접 정하면 된다. */
struct supplemental_page_table {
};

#include "threads/thread.h"
void supplemental_page_table_init (struct supplemental_page_table *spt);
bool supplemental_page_table_copy (struct supplemental_page_table *dst,
		struct supplemental_page_table *src);
void supplemental_page_table_kill (struct supplemental_page_table *spt);
struct page *spt_find_page (struct supplemental_page_table *spt,
		void *va);
bool spt_insert_page (struct supplemental_page_table *spt, struct page *page);
void spt_remove_page (struct supplemental_page_table *spt, struct page *page);

void vm_init (void);
bool vm_try_handle_fault (struct intr_frame *f, void *addr, bool user,
		bool write, bool not_present);

#define vm_alloc_page(type, upage, writable) \
	vm_alloc_page_with_initializer ((type), (upage), (writable), NULL, NULL)
bool vm_alloc_page_with_initializer (enum vm_type type, void *upage,
		bool writable, vm_initializer *init, void *aux);
void vm_dealloc_page (struct page *page);
bool vm_claim_page (void *va);
enum vm_type page_get_type (struct page *page);

#endif  /* VM_VM_H */
