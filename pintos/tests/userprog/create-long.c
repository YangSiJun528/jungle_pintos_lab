/* Tries to create a file with a name that is much too long,
   which must fail. */
/* 너무 긴 이름을 가진 file을 만들려고 시도하며, 이는 실패해야 한다. */

#include <string.h>
#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  static char name[512];
  memset (name, 'x', sizeof name);
  name[sizeof name - 1] = '\0';
  
  msg ("create(\"x...\"): %d", create (name, 0));
}
