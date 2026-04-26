/* Tries to open the same file twice,
   which must succeed and must return a different file descriptor
   in each case. */
/* 같은 file을 두 번 열려고 시도한다.
   이는 성공해야 하며, 각각 서로 다른 file descriptor를 반환해야 한다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  int h1 = open ("sample.txt");
  int h2 = open ("sample.txt");  

  CHECK ((h1 = open ("sample.txt")) > 1, "open \"sample.txt\" once");
  CHECK ((h2 = open ("sample.txt")) > 1, "open \"sample.txt\" again");
  if (h1 == h2)
    fail ("open() returned %d both times", h1);
}
