/* uninit.c: Implementation of uninitialized page.
 *
 * All of the pages are born as uninit page. When the first page fault occurs,
 * the handler chain calls uninit_initialize (page->operations.swap_in).
 * The uninit_initialize function transmutes the page into the specific page
 * object (anon, file, page_cache), by initializing the page object,and calls
 * initialization callback that passed from vm_alloc_page_with_initializer
 * function.
 * */
/* uninit.c: 아직 초기화되지 않은 페이지 구현.
 *
 * 모든 페이지는 uninit page로 생성된다. 첫 page fault가 발생하면 핸들러 체인이
 * uninit_initialize(page->operations.swap_in)를 호출한다.
 * uninit_initialize 함수는 페이지 오브젝트를 초기화하고
 * vm_alloc_page_with_initializer 함수에서 전달된 initialization callback을
 * 호출해 해당 페이지를 구체적인 페이지 오브젝트(anon, file, page_cache)로
 * 변환한다.
 * */

#include "vm/vm.h"
#include "vm/uninit.h"

static bool uninit_initialize (struct page *page, void *kva);
static void uninit_destroy (struct page *page);

/* DO NOT MODIFY this struct */
/* 이 struct는 수정하지 않는다. */
static const struct page_operations uninit_ops = {
	.swap_in = uninit_initialize,
	.swap_out = NULL,
	.destroy = uninit_destroy,
	.type = VM_UNINIT,
};

/* DO NOT MODIFY this function */
/* 이 함수는 수정하지 않는다. */
void
uninit_new (struct page *page, void *va, vm_initializer *init,
		enum vm_type type, void *aux,
		bool (*initializer)(struct page *, enum vm_type, void *)) {
	ASSERT (page != NULL);

	*page = (struct page) {
		.operations = &uninit_ops,
		.va = va,
		.frame = NULL, /* no frame for now */
		/* 현재는 프레임이 없다. */
		.uninit = (struct uninit_page) {
			.init = init,
			.type = type,
			.aux = aux,
			.page_initializer = initializer,
		}
	};
}

/* Initalize the page on first fault */
/* 첫 fault에서 페이지를 초기화한다. */
static bool
uninit_initialize (struct page *page, void *kva) {
	struct uninit_page *uninit = &page->uninit;

	/* Fetch first, page_initialize may overwrite the values */
	/* page_initialize가 값을 덮어쓸 수 있으므로 먼저 가져온다. */
	vm_initializer *init = uninit->init;
	void *aux = uninit->aux;

	/* TODO: You may need to fix this function. */
	/* TODO: 이 함수를 수정해야 할 수도 있다. */
	return uninit->page_initializer (page, uninit->type, kva) &&
		(init ? init (page, aux) : true);
}

/* Free the resources hold by uninit_page. Although most of pages are transmuted
 * to other page objects, it is possible to have uninit pages when the process
 * exit, which are never referenced during the execution.
 * PAGE will be freed by the caller. */
/* uninit_page가 들고 있는 리소스를 해제한다. 대부분의 페이지는 다른 페이지
 * 오브젝트로 변환되지만, 실행 중 한 번도 참조되지 않은 uninit page가 프로세스
 * 종료 시점에 남아 있을 수 있다.
 * PAGE는 호출자가 해제한다. */
static void
uninit_destroy (struct page *page) {
	struct uninit_page *uninit UNUSED = &page->uninit;
	/* TODO: Fill this function.
	 * TODO: If you don't have anything to do, just return. */
	/* TODO: 이 함수를 채운다.
	 * TODO: 할 일이 없다면 그냥 리턴한다. */
}
