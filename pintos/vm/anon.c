/* anon.c: Implementation of page for non-disk image (a.k.a. anonymous page). */
/* anon.c: 디스크 이미지와 연결되지 않은 페이지, 즉 anonymous page 구현. */

#include "vm/vm.h"
#include "devices/disk.h"

/* DO NOT MODIFY BELOW LINE */
/* 아래 라인은 수정하지 않는다. */
static struct disk *swap_disk;
static bool anon_swap_in (struct page *page, void *kva);
static bool anon_swap_out (struct page *page);
static void anon_destroy (struct page *page);

/* DO NOT MODIFY this struct */
/* 이 struct는 수정하지 않는다. */
static const struct page_operations anon_ops = {
	.swap_in = anon_swap_in,
	.swap_out = anon_swap_out,
	.destroy = anon_destroy,
	.type = VM_ANON,
};

/* Initialize the data for anonymous pages */
/* anonymous page를 위한 데이터를 초기화한다. */
void
vm_anon_init (void) {
	/* TODO: Set up the swap_disk. */
	/* TODO: swap_disk를 설정한다. */
	swap_disk = NULL;
}

/* Initialize the file mapping */
/* 파일 맵핑을 초기화한다. */
bool
anon_initializer (struct page *page, enum vm_type type, void *kva) {
	/* Set up the handler */
	/* 핸들러를 설정한다. */
	page->operations = &anon_ops;

	struct anon_page *anon_page UNUSED = &page->anon;
	return true;
}

/* Swap in the page by read contents from the swap disk. */
/* swap disk에서 내용을 읽어 페이지를 swap in한다. */
static bool
anon_swap_in (struct page *page, void *kva) {
	struct anon_page *anon_page UNUSED = &page->anon;
	return true;
}

/* Swap out the page by writing contents to the swap disk. */
/* 내용을 swap disk에 써서 페이지를 swap out한다. */
static bool
anon_swap_out (struct page *page) {
	struct anon_page *anon_page UNUSED = &page->anon;
	return false;
}

/* Destroy the anonymous page. PAGE will be freed by the caller. */
/* anonymous page를 파괴한다. PAGE는 호출자가 해제한다. */
static void
anon_destroy (struct page *page) {
	struct anon_page *anon_page UNUSED = &page->anon;
}
