/* Wait for a subprocess to finish, twice.
   The first call must wait in the usual way and return the exit code.
   The second wait call must return -1 immediately. */
/* subprocess가 끝나기를 두 번 기다린다.
   첫 번째 call은 일반적인 방식으로 기다린 뒤 exit code를 반환해야 한다.
   두 번째 wait call은 즉시 -1을 반환해야 한다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  pid_t child;
  if ((child = fork ("child-simple"))){
    msg ("wait(exec()) = %d", wait (child));
    msg ("wait(exec()) = %d", wait (child));
  } else {
    exec ("child-simple");
  }
}
