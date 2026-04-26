/* Tries to remove a parent of the current directory.  This must
   fail, because that directory is non-empty. */
/* current directory의 parent를 제거하려고 시도한다.
   해당 directory가 비어 있지 않으므로 이는 실패해야 한다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  CHECK (mkdir ("a"), "mkdir \"a\"");
  CHECK (chdir ("a"), "chdir \"a\"");
  CHECK (mkdir ("b"), "mkdir \"b\"");
  CHECK (chdir ("b"), "chdir \"b\"");
  CHECK (!remove ("/a"), "remove \"/a\" (must fail)");
}
