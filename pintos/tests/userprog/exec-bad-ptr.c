/* Passes an invalid pointer to the exec system call.
   The process must be terminated with -1 exit code. */
/* exec system call에 잘못된 pointer를 전달한다.
   process는 exit code -1로 종료되어야 한다. */

#include <syscall.h>
#include "tests/main.h"

void
test_main (void) 
{
  exec ((char *) 0x20101234);
}
