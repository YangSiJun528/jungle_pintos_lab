/* Child process run by wait-killed test.
   Tries to execute pintos, which should then terminate the process with a
   -1 exit code because `pintos` is not present in Pintos. */
/* wait-killed 테스트가 실행하는 child process.
   pintos 실행을 시도하며, Pintos 안에는 `pintos`가 없으므로 process는 exit code
   -1로 종료되어야 한다. */

#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  exec ("pintos");
  fail ("should have exited with -1");
}
