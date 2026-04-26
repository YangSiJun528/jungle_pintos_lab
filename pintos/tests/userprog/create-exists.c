/* Verifies that trying to create a file under a name that
   already exists will fail. */
/* 이미 존재하는 이름으로 file을 만들려고 하면 실패하는지 검증한다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  CHECK (create ("quux.dat", 0), "create quux.dat");
  CHECK (create ("warble.dat", 0), "create warble.dat");
  CHECK (!create ("quux.dat", 0), "try to re-create quux.dat");
  CHECK (create ("baffle.dat", 0), "create baffle.dat");
  CHECK (!create ("warble.dat", 0), "try to re-create quux.dat");
}
