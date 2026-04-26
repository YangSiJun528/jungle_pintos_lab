/* Passes a bad pointer to the create system call,
   which must cause the process to be terminated with exit code
   -1. */
/* create system call에 잘못된 pointer를 전달한다.
   이로 인해 process는 exit code -1로 종료되어야 한다. */

#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  msg ("create(0x20101234): %d", create ((char *) 0x20101234, 0));
}
