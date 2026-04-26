/* This program attempts to read memory at an address that is not mapped.
   This should terminate the process with a -1 exit code. */
/* 이 프로그램은 mapping되지 않은 address의 memory를 읽으려고 시도한다.
   process는 exit code -1로 종료되어야 한다. */

#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  msg ("Congratulations - you have successfully dereferenced NULL: %d", 
        *(int *)NULL);
  fail ("should have exited with -1");
}
