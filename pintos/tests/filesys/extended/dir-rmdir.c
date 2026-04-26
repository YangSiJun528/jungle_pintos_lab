/* Creates and removes a directory, then makes sure that it's
   really gone. */
/* directory를 만들고 제거한 뒤 실제로 사라졌는지 확인한다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  CHECK (mkdir ("a"), "mkdir \"a\"");
  CHECK (remove ("a"), "rmdir \"a\"");
  CHECK (!chdir ("a"), "chdir \"a\" (must return false)");
}
