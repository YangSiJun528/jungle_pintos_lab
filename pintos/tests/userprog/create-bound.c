/* Opens a file whose name spans the boundary between two pages.
   This is valid, so it must succeed. */
/* 이름이 두 page 사이의 boundary에 걸쳐 있는 file을 연다.
   이는 유효하므로 성공해야 한다. */

#include <syscall.h>
#include "tests/userprog/boundary.h"
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  msg ("create(\"quux.dat\"): %d",
       create (copy_string_across_boundary ("quux.dat"), 0));
}
