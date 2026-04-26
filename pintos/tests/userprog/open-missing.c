/* Tries to open a nonexistent file. */
/* 존재하지 않는 file을 열려고 시도한다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  int handle = open ("no-such-file");
  if (handle != -1)
    fail ("open() returned %d", handle);
}
