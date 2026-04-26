/* Try writing to fd 0 (stdin), 
   which may just fail or terminate the process with -1 exit
   code. */
/* fd 0(stdin)에 write를 시도한다.
   이는 실패하거나 process를 exit code -1로 종료시킬 수 있다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  char buf = 123;
  write (0, &buf, 1);
}
