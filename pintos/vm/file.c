/* file.c: Implementation of memory backed file object (mmaped object). */
/* file.c: 메모리로 backing되는 파일 오브젝트, 즉 mmap된 오브젝트 구현. */

#include "vm/vm.h"
#include <round.h>
#include "threads/mmu.h"
#include "threads/vaddr.h"
#include "userprog/process.h"

static bool file_backed_swap_in (struct page *page, void *kva);
static bool file_backed_swap_out (struct page *page);
static void file_backed_destroy (struct page *page);

/* DO NOT MODIFY this struct */
/* 이 struct는 수정하지 않는다. */
static const struct page_operations file_ops = {
	.swap_in = file_backed_swap_in,
	.swap_out = file_backed_swap_out,
	.destroy = file_backed_destroy,
	.type = VM_FILE,
};

/* The initializer of file vm */
/* file vm의 이니셜라이저. */
void
vm_file_init (void) {
}

/* Initialize the file backed page */
/* file-backed page를 초기화한다. */
bool
file_backed_initializer (struct page *page, enum vm_type type UNUSED,
		void *kva UNUSED) {
	/* Set up the handler */
	/* 핸들러를 설정한다. */
	page->operations = &file_ops;

	struct file_page *file_page UNUSED = &page->file;
	return true;
}

/* Swap in the page by read contents from the file. */
/* 파일에서 내용을 읽어 페이지를 swap in한다. */
static bool
file_backed_swap_in (struct page *page, void *kva UNUSED) {
	struct file_page *file_page UNUSED = &page->file;
	return false;
}

/* Swap out the page by writeback contents to the file. */
/* 내용을 파일에 writeback해서 페이지를 swap out한다. */
static bool
file_backed_swap_out (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
	return false;
}

/* Destory the file backed page. PAGE will be freed by the caller. */
/* file-backed page를 파괴한다. PAGE는 호출자가 해제한다. */
static void
file_backed_destroy (struct page *page) {
	struct file_page *file_page = &page->file;

	file_close (file_page->file);
}

/* Do the mmap */
/* mmap을 수행한다. */
void *
do_mmap (void *addr, size_t u_length, int writable,
		struct file *file, off_t offset) {
	struct supplemental_page_table *spt = &thread_current ()->spt;
	struct page *page = spt_find_page (spt, addr);
	bool page_already_exists = page != NULL
			&& page_get_type (page) == VM_FILE
			&& page->mmaped_size != 0;
	size_t read_bytes;
	size_t zero_bytes;
	off_t file_len;

	if (page_already_exists)
		return NULL;

	file_len = file_length (file);
	if (u_length >= (size_t) file_len) {
		read_bytes = file_len;
		zero_bytes = ROUND_UP (u_length, PGSIZE) - file_len;
	} else {
		read_bytes = u_length;
		zero_bytes = 0;
	}

	if (!load_segment (file, offset, addr, read_bytes, zero_bytes, writable))
		return NULL;

	page = spt_find_page (spt, addr);
	if (page == NULL)
		return NULL;
	page->mmaped_size = (read_bytes + zero_bytes) / PGSIZE;
	return addr;
}

/* Do the munmap */
/* munmap을 수행한다. */
void
do_munmap (void *addr) {
	struct supplemental_page_table *spt = &thread_current ()->spt;
	struct page *page = spt_find_page (spt, addr);
	uint64_t mmaped_size;

	if (page == NULL)
		return;

	mmaped_size = page->mmaped_size;
	if (mmaped_size == 0)
		return;

	for (uint64_t i = 0; i < mmaped_size; i++) {
		page = spt_find_page (spt, (uint8_t *) addr + i * PGSIZE);
		if (page != NULL)
			spt_remove_page (spt, page);
	}
}
