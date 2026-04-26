/* Tests argument passing to child processes. */
/* child process로 argument가 전달되는지 테스트한다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  msg ("I'm your father");
  exec ("child-args childarg");
}
