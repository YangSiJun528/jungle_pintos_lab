/* Child process for mmap-inherit test.
   Tries to write to a mapping present in the parent.
   The process must be terminated with -1 exit code. */
/* mmap-inherit 테스트용 child process.
   parent에 있는 mapping에 쓰려고 시도한다.
   process는 exit code -1로 종료되어야 한다. */

#include <string.h>
#include "tests/vm/sample.inc"
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void)
{
  memset ((char *) 0x54321000, 0, 4096);
  fail ("child can modify parent's memory mappings");
}

