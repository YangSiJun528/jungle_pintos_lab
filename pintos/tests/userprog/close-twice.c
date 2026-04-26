/* Opens a file and then tries to close it twice.  The second
   close must either fail silently or terminate with exit code
   -1. */
/* file을 연 뒤 두 번 close하려고 시도한다. 두 번째 close는 조용히 실패하거나 exit
   code -1로 종료되어야 한다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  int handle;
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
  msg ("close \"sample.txt\"");
  close (handle);
  msg ("close \"sample.txt\" again");
  close (handle);
}
