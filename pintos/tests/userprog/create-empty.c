/* Tries to create a file with the empty string as its name. */
/* 빈 문자열을 이름으로 가진 file을 만들려고 시도한다. */

#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  msg ("create(\"\"): %d", create ("", 0));
}
