/* Tries to create a file with the same name as an existing
   directory, which must return failure. */
/* 기존 directory와 같은 이름의 file을 만들려고 시도하며,
   이는 failure를 반환해야 한다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  CHECK (mkdir ("abc"), "mkdir \"abc\"");
  CHECK (!create ("abc", 0), "create \"abc\" (must return false)");
}
