/* Checks if anonymous pages are lazy loaded  */
/* anonymous page가 lazy load되는지 확인한다. */

#include <string.h>
#include <syscall.h>
#include <stdio.h>
#include <stdint.h>
#include "tests/lib.h"
#include "tests/main.h"

#define PAGE_SIZE 4096
#define CHUNK_PAGE_COUNT 3
#define CHUNK_SIZE (CHUNK_PAGE_COUNT * PAGE_SIZE)

static char buf[CHUNK_SIZE];

void
test_main (void)
{
	size_t i, j;
	void *pa;

	msg ("initial pages status");
	for (i = 0 ; i < CHUNK_PAGE_COUNT ; i++) {
		// All pages for buf should not be loaded yet.
		// buf의 모든 page는 아직 load되지 않아야 한다.
		pa = get_phys_addr(&buf[i*PAGE_SIZE]);
		CHECK (pa == 0, "check if page is not loaded");
	}

	msg ("load pages");
	for (i = 0 ; i < CHUNK_PAGE_COUNT ; i++) {
		msg ("load page [%zu]", i);
		// Pages are loaded here.
		// 여기에서 page가 load된다.
		buf[i*PAGE_SIZE] = i;
		for (j = 0 ; j < CHUNK_PAGE_COUNT ; j++) {
			// Pages that have been accessed should be loaded
			// 접근된 page는 load되어야 한다.
			// Pages that have not been accessed should not be loaded.
			// 접근되지 않은 page는 load되지 않아야 한다.
			pa = get_phys_addr(&buf[j*PAGE_SIZE]);
			if (j <= i) {
				CHECK (pa != 0, "check if page is loaded");
				CHECK (buf[j*PAGE_SIZE] == (char) j, "check memory content");
			}
			else {
				CHECK (pa == 0, "check if page is not loaded");
			}
		}
	}
}

