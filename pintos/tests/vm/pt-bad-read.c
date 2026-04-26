/* Reads from a file into a bad address.
   The process must be terminated with -1 exit code. */
/* file에서 잘못된 address로 읽는다.
   process는 exit code -1로 종료되어야 한다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void)
{
  int handle;

  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
  read (handle, (char *) &handle - 4096, 1);
  fail ("survived reading data into bad address");
}
