/* Child process run by exec-multiple, exec-one, wait-simple, and
   wait-twice tests.
   Just prints a single message and terminates. */
/* exec-multiple, exec-one, wait-simple, wait-twice 테스트가 실행하는 child process.
   단일 메시지를 출력하고 종료한다. */

#include <stdio.h>
#include "tests/lib.h"

int
main (void) 
{
  test_name = "child-simple";

  msg ("run");
  return 81;
}
