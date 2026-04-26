/* This program attempts to write to kernel memory. 
   This should terminate the process with a -1 exit code. */
/* 이 프로그램은 kernel memory에 쓰려고 시도한다.
   process는 exit code -1로 종료되어야 한다. */

#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  *(int *)0x8004000000 = 42;
  fail ("should have exited with -1");
}
