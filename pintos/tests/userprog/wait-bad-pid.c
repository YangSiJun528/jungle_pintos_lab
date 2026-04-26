/* Waits for an invalid pid.  This may fail or terminate the
   process with -1 exit code. */
/* 잘못된 pid를 wait한다. 이는 실패하거나 process를 exit code -1로 종료시킬 수 있다. */

#include <syscall.h>
#include "tests/main.h"

void
test_main (void) 
{
  wait ((pid_t) 0x0c020301);
}
