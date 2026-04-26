/* Passes an invalid pointer to the read system call.
   The process must be terminated with -1 exit code. */
/* read system call에 잘못된 pointer를 전달한다.
   process는 exit code -1로 종료되어야 한다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  int handle;
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");

  read (handle, (char *) 0xc0100000, 123);
  fail ("should not have survived read()");
}
