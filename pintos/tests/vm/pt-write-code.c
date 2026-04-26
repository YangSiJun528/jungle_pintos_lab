/* Try to write to the code segment.
   The process must be terminated with -1 exit code. */
/* code segment에 쓰려고 시도한다.
   process는 exit code -1로 종료되어야 한다. */

#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void)
{
  *(int *) test_main = 0;
  fail ("writing the code segment succeeded");
}
