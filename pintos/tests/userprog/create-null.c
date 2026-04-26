/* Tries to create a file with the null pointer as its name.
   The process must be terminated with exit code -1. */
/* null pointer를 이름으로 사용해 file을 만들려고 시도한다.
   process는 exit code -1로 종료되어야 한다. */

#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  msg ("create(NULL): %d", create (NULL, 0));
}
