/* Tries to mmap with fd 0,
   which is the file descriptor for console input. 
	 mmap must fail silently or terminate the process with  
   exit code -1. */
/* console input용 file descriptor인 fd 0으로 mmap을 시도한다.
   mmap은 조용히 실패하거나 process를 exit code -1로 종료시켜야 한다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  CHECK (mmap ((void *) 0x10000000, 4096, 0, 0, 0) == MAP_FAILED,
         "try to mmap stdin");
}

