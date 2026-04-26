/* Writes to a file through a mapping, and unmaps the file,
   then reads the data in the file back using the read system
   call to verify. */
/* mapping을 통해 file에 쓰고 file을 unmap한 뒤,
   read system call로 file의 data를 다시 읽어 검증한다. */

#include <string.h>
#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

#define ACTUAL ((void *) 0x10000000)

void
test_main (void)
{
  int handle;
  void *map;
  char buf[1024];

  /* Write file via mmap. */
  /* mmap을 통해 file에 쓴다. */
  CHECK ((handle = open ("large.txt")) > 1, "open \"large.txt\"");
  CHECK ((map = mmap (ACTUAL, 4096, 0, handle, 0)) != MAP_FAILED, "mmap \"large.txt\" with writable=0");
  msg ("about to write into read-only mmap'd memory");
  *((int *)map) = 0;
  msg ("Error should have occured");
  munmap (map);
  close (handle);
}
