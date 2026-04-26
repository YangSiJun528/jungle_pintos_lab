/* This program attempts to execute code at a kernel virtual address. 
   This should terminate the process with a -1 exit code. */
/* 이 프로그램은 kernel virtual address에서 code를 실행하려고 시도한다.
   process는 exit code -1로 종료되어야 한다. */

#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  msg ("Congratulations - you have successfully called kernel code: %d", 
        ((int (*)(void))0x8004000000)());
  fail ("should have exited with -1");
}
