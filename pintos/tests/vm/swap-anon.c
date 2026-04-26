/* Checks if anonymous pages 
 * are swapped out and swapped in properly 
 * For this test, Pintos memory size is 10MB 
 * First, allocates big chunks of memory, 
 * does some write operations on each chunk, 
 * then check if the data is consistent
 * Lastly, frees the allocated memory. */
/* anonymous page가 올바르게 swap-out되고 swap-in되는지 확인한다.
 * 이 테스트에서 Pintos memory size는 10 MB다.
 * 먼저 큰 memory chunk를 할당하고,
 * 각 chunk에 write operation을 수행한 뒤,
 * data가 일관적인지 확인한다.
 * 마지막으로 할당된 memory를 free한다. */

#include <string.h>
#include <stdint.h>
#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"


#define PAGE_SHIFT 12
#define PAGE_SIZE (1 << PAGE_SHIFT)
#define ONE_MB (1 << 20) // 1MB
// 1 MB를 나타낸다.
#define CHUNK_SIZE (20*ONE_MB)
#define PAGE_COUNT (CHUNK_SIZE / PAGE_SIZE)

static char big_chunks[CHUNK_SIZE];

void
test_main (void) 
{
	size_t i;
    void* pa;
    char *mem;

    for (i = 0 ; i < PAGE_COUNT ; i++) {
        if(!(i % 512))
            msg ("write sparsely over page %zu", i);
        mem = (big_chunks+(i*PAGE_SIZE));
        *mem = (char)i;
    }

    for (i = 0 ; i < PAGE_COUNT ; i++) {
        mem = (big_chunks+(i*PAGE_SIZE));
        if((char)i != *mem) {
		    fail ("data is inconsistent");
        }
        if(!(i % 512))
            msg ("check consistency in page %zu", i);
    }
}

