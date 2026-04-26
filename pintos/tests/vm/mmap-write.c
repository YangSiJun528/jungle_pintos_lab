/* Writes to a file through a mapping, and unmaps the file,
   then reads the data in the file back using the read system
   call to verify. */
/* mapping을 통해 file에 쓰고 file을 unmap한 뒤,
   read system call로 file의 data를 다시 읽어 검증한다. */

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
  char buf[1024];

  /* Write file via mmap. */
  /* mmap을 통해 file에 쓴다. */
  CHECK (create ("sample.txt", strlen (sample)), "create \"sample.txt\"");
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
  CHECK ((map = mmap (ACTUAL, 4096, 1, handle, 0)) != MAP_FAILED, "mmap \"sample.txt\"");
  memcpy (ACTUAL, sample, strlen (sample));
  munmap (map);

  /* Read back via read(). */
  /* read()를 통해 다시 읽는다. */
  read (handle, buf, strlen (sample));
  CHECK (!memcmp (buf, sample, strlen (sample)),
         "compare read data against written data");
  close (handle);
}
