/* Child process of mmap-exit.
   Mmaps a file and writes to it via the mmap'ing, then exits
   without calling munmap.  The data in the mapped region must be
   written out at program termination. */
/* mmap-exit의 child process.
   file을 mmap하고 mmap을 통해 write한 뒤, munmap을 호출하지 않고 종료한다.
   mapped region의 data는 program 종료 시 기록되어야 한다. */

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

  CHECK (create ("sample.txt", sizeof sample), "create \"sample.txt\"");
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
  CHECK (mmap (ACTUAL, sizeof sample, 1, handle, 0) != MAP_FAILED, "mmap \"sample.txt\"");
  memcpy (ACTUAL, sample, sizeof sample);
}

