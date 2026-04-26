/* Mmaps a 128 kB file "sorts" the bytes in it, using quick sort,
   a multi-pass divide and conquer algorithm.  */
/* 128 kB file을 mmap하고, multi-pass divide and conquer algorithm인
   quick sort를 사용해 그 안의 byte를 "sort"한다. */

#include <debug.h>
#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"
#include "tests/vm/qsort.h"

int
main (int argc UNUSED, char *argv[]) 
{
  test_name = "child-qsort-mm";

  int handle;
  unsigned char *p = (unsigned char *) 0x10000000;

  quiet = true;

  CHECK ((handle = open (argv[1])) > 1, "open \"%s\"", argv[1]);
  CHECK (mmap (p, 4096*33, 1, handle, 0) != MAP_FAILED, "mmap \"%s\"", argv[1]);
  qsort_bytes (p, 1024 * 128);
  
  return 80;
}
