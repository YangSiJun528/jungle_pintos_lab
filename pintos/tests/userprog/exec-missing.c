/* Tries to execute a nonexistent process.
   The exec system call must return -1. */
/* 존재하지 않는 process를 실행하려고 시도한다.
   exec system call은 -1을 반환해야 한다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  msg ("exec(\"no-such-file\"): %d", exec ("no-such-file"));
}
