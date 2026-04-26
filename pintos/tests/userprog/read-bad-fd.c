/* Tries to read from an invalid fd,
   which must either fail silently or terminate the process with
   exit code -1. */
/* 잘못된 fd에서 read하려고 시도한다.
   이는 조용히 실패하거나 process를 exit code -1로 종료시켜야 한다. */

#include <limits.h>
#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  char buf;
  read (0x20101234, &buf, 1);
  read (5, &buf, 1);
  read (1234, &buf, 1);
  read (-1, &buf, 1);
  read (-1024, &buf, 1);
  read (INT_MIN, &buf, 1);
  read (INT_MAX, &buf, 1);
}
