/* Opens a directory, then tries to write to it, which must
   fail. */
/* directory를 연 뒤 write하려고 시도하며, 이는 실패해야 한다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  int fd;
  int retval;
  
  CHECK (mkdir ("xyzzy"), "mkdir \"xyzzy\"");
  CHECK ((fd = open ("xyzzy")) > 1, "open \"xyzzy\"");

  msg ("write \"xyzzy\"");
  retval = write (fd, "foobar", 6);
  CHECK (retval == -1,
         "write \"xyzzy\" (must return -1, actually %d)", retval);
}
