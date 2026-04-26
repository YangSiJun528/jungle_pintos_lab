/* Tries to mmap an invalid fd,
   which must either fail silently or terminate the process with
   exit code -1. */
/* 잘못된 fd로 mmap을 시도한다.
   이는 조용히 실패하거나 process를 exit code -1로 종료시켜야 한다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  CHECK (mmap ((void *) 0x10000000, 4096, 0, 0x5678, 0) == MAP_FAILED,
         "try to mmap invalid fd");
}

