/* Passes an invalid pointer to the write system call.
   The process must be terminated with -1 exit code. */
/* write system call에 잘못된 pointer를 전달한다.
   process는 exit code -1로 종료되어야 한다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  int handle;
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");

  write (handle, (char *) 0x10123420, 123);
  fail ("should have exited with -1");
}
