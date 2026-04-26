/* This program attempts to read kernel memory. 
   This should terminate the process with a -1 exit code. */
/* 이 프로그램은 kernel memory를 읽으려고 시도한다.
   process는 exit code -1로 종료되어야 한다. */

#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  msg ("Congratulations - you have successfully read kernel memory: %d", 
        *(int *)0x8004000000);
  fail ("should have exited with -1");
}
