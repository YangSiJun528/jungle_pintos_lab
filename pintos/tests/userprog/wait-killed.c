/* Wait for a process that will be killed for bad behavior. */
/* 잘못된 동작 때문에 kill될 process를 wait한다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  pid_t child;
  if ((child = fork ("child-bad"))){
    msg ("wait(exec()) = %d", wait (child));
  } else {
    exec ("child-bad");
  }
}
