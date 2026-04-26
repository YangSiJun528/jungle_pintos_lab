/* Try reading from fd 1 (stdout), 
   which may just fail or terminate the process with -1 exit
   code. */
/* fd 1(stdout)에서 read를 시도한다.
   이는 실패하거나 process를 exit code -1로 종료시킬 수 있다. */

#include <stdio.h>
#include <syscall.h>
#include "tests/main.h"

void
test_main (void) 
{
  char buf;
  read (STDOUT_FILENO, &buf, 1);
}
