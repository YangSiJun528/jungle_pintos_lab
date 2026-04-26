/* page_cache.c: Implementation of Page Cache (Buffer Cache). */
/* page_cache.c: Page Cache(Buffer Cache) 구현. */

#include "vm/vm.h"
static bool page_cache_readahead (struct page *page, void *kva);
static bool page_cache_writeback (struct page *page);
static void page_cache_destroy (struct page *page);

/* DO NOT MODIFY this struct */
/* 이 struct는 수정하지 않는다. */
static const struct page_operations page_cache_op = {
	.swap_in = page_cache_readahead,
	.swap_out = page_cache_writeback,
	.destroy = page_cache_destroy,
	.type = VM_PAGE_CACHE,
};

tid_t page_cache_workerd;

/* The initializer of file vm */
/* file vm의 이니셜라이저. */
void
pagecache_init (void) {
	/* TODO: Create a worker daemon for page cache with page_cache_kworkerd */
	/* TODO: page_cache_kworkerd로 page cache 워커 데몬을 만든다. */
}

/* Initialize the page cache */
/* page cache를 초기화한다. */
bool
page_cache_initializer (struct page *page, enum vm_type type, void *kva) {
	/* Set up the handler */
	/* 핸들러를 설정한다. */
	page->operations = &page_cache_op;

}

/* Utilze the Swap in mechanism to implement readhead */
/* Swap in 메커니즘을 활용해 readahead를 구현한다. */
static bool
page_cache_readahead (struct page *page, void *kva) {
}

/* Utilze the Swap out mechanism to implement writeback */
/* Swap out 메커니즘을 활용해 writeback을 구현한다. */
static bool
page_cache_writeback (struct page *page) {
}

/* Destory the page_cache. */
/* page_cache를 파괴한다. */
static void
page_cache_destroy (struct page *page) {
}

/* Worker thread for page cache */
/* page cache용 워커 스레드. */
static void
page_cache_kworkerd (void *aux) {
}
