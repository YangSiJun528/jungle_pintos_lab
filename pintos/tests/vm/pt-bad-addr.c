/* Accesses a bad address.
   The process must be terminated with -1 exit code. */
/* 잘못된 address에 접근한다.
   process는 exit code -1로 종료되어야 한다. */

#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void)
{
  fail ("bad addr read as %d", *(int *) 0x04000000);
}
