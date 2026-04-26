/* Tries to close an invalid fd, which must either fail silently
   or terminate with exit code -1. */
/* 잘못된 fd를 close하려고 시도한다. 이는 조용히 실패하거나 exit code -1로 종료되어야
   한다. */

#include <syscall.h>
#include "tests/main.h"

void
test_main (void) 
{
  close (0x20101234);
}
