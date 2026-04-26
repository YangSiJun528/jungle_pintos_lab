/* Tries to create a directory named as the empty string,
   which must return failure. */
/* 빈 문자열을 이름으로 가진 directory를 만들려고 시도하며,
   이는 failure를 반환해야 한다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  CHECK (!mkdir (""), "mkdir \"\" (must return false)");
}
