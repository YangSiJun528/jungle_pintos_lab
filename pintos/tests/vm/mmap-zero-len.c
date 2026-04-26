/* Tries to mmap a zero length. mmap should fail if legnth is 0. 
 * In Linux kernels before 2.6.12, mmap succeeded even with length0. 
 * In this case: no mapping was created and the call returned addr.  
 * Since kernel 2.6.12, mapping zero length fails. We expect 
 * mmap in Pintos to fail (i.e. return MAP_FAILED) when length is 0. */
/* 길이가 0인 mmap을 시도한다. length가 0이면 mmap은 실패해야 한다.
 * Linux kernel 2.6.12 이전에는 length가 0이어도 mmap이 성공했다.
 * 이 경우 mapping은 생성되지 않고 call은 addr을 반환했다.
 * kernel 2.6.12부터는 길이가 0인 mapping이 실패한다. Pintos의 mmap도
 * length가 0일 때 실패해야 한다고 기대한다. 즉 MAP_FAILED를 반환해야 한다. */

#include <string.h>
#include <syscall.h>
#include "tests/vm/sample.inc"
#include "tests/lib.h"
#include "tests/main.h"

#define ACTUAL ((void *) 0x10000000)

void
test_main (void)
{
  int handle;
  void *map;

  /* Write file via mmap. */
  /* mmap을 통해 file에 쓴다. */
  CHECK (create ("sample.txt", strlen (sample)), "create \"sample.txt\"");
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
  CHECK ((map = mmap (ACTUAL, 0, 0, handle, 0)) == MAP_FAILED, 
			"try to mmap zero length");
 
}
