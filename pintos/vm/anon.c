/* anon.c: Implementation of page for non-disk image (a.k.a. anonymous page). */
/* anon.c: 디스크 이미지와 연결되지 않은 페이지, 즉 anonymous page 구현. */

#include "vm/vm.h"
#include <string.h>
#include "bitmap.h"
#include "devices/disk.h"
#include "threads/synch.h"
#include "threads/vaddr.h"

/* DO NOT MODIFY BELOW LINE */
/* 아래 라인은 수정하지 않는다. */
static struct disk *swap_disk;
static struct bitmap *swap_table;
static struct lock swap_lock;
static bool anon_swap_in (struct page *page, void *kva);
static bool anon_swap_out (struct page *page);
static void anon_destroy (struct page *page);

#define SECTORS_PER_PAGE (PGSIZE / DISK_SECTOR_SIZE)

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
	swap_disk = disk_get (1, 1);
	lock_init (&swap_lock);

	if (swap_disk == NULL) {
		swap_table = NULL;
		return;
	}
	swap_table = bitmap_create (disk_size (swap_disk) / SECTORS_PER_PAGE);
	if (swap_table == NULL) {
		PANIC ("failed to create swap table");
	}
}

/* Initialize the file mapping */
/* 파일 맵핑을 초기화한다. */
bool
anon_initializer (struct page *page, enum vm_type type, void *kva) {
	/* Set up the handler */
	/* 핸들러를 설정한다. */
	page->operations = &anon_ops;

	struct anon_page *anon_page = &page->anon;
	anon_page->swap_slot = BITMAP_ERROR;
	anon_page->in_swap = false;
	return true;
}

/* Swap in the page by read contents from the swap disk. */
/* swap disk에서 내용을 읽어 페이지를 swap in한다. */
static bool
anon_swap_in (struct page *page, void *kva) {
	struct anon_page *anon_page = &page->anon;

	if (!anon_page->in_swap) {
		memset (kva, 0, PGSIZE);
		return true;
	}

	ASSERT (swap_disk != NULL);
	ASSERT (swap_table != NULL);

	lock_acquire (&swap_lock);
	for (size_t i = 0; i < SECTORS_PER_PAGE; i++) {
		disk_read (swap_disk, anon_page->swap_slot * SECTORS_PER_PAGE + i,
				(uint8_t *) kva + i * DISK_SECTOR_SIZE);
	}
	bitmap_reset (swap_table, anon_page->swap_slot);
	anon_page->swap_slot = BITMAP_ERROR;
	anon_page->in_swap = false;
	lock_release (&swap_lock);
	return true;
}

/* Swap out the page by writing contents to the swap disk. */
/* 내용을 swap disk에 써서 페이지를 swap out한다. */
static bool
anon_swap_out (struct page *page) {
	struct anon_page *anon_page = &page->anon;
	size_t slot;

	ASSERT (page->frame != NULL);
	ASSERT (swap_disk != NULL);
	ASSERT (swap_table != NULL);

	lock_acquire (&swap_lock);
	slot = bitmap_scan_and_flip (swap_table, 0, 1, false);
	if (slot == BITMAP_ERROR) {
		lock_release (&swap_lock);
		PANIC ("swap disk is full");
	}

	for (size_t i = 0; i < SECTORS_PER_PAGE; i++) {
		disk_write (swap_disk, slot * SECTORS_PER_PAGE + i,
				(uint8_t *) page->frame->kva + i * DISK_SECTOR_SIZE);
	}
	anon_page->swap_slot = slot;
	anon_page->in_swap = true;
	lock_release (&swap_lock);
	return true;
}

/* Destroy the anonymous page. PAGE will be freed by the caller. */
/* anonymous page를 파괴한다. PAGE는 호출자가 해제한다. */
static void
anon_destroy (struct page *page) {
	struct anon_page *anon_page = &page->anon;

	if (anon_page->in_swap) {
		ASSERT (swap_table != NULL);
		lock_acquire (&swap_lock);
		bitmap_reset (swap_table, anon_page->swap_slot);
		anon_page->swap_slot = BITMAP_ERROR;
		anon_page->in_swap = false;
		lock_release (&swap_lock);
	}
}
