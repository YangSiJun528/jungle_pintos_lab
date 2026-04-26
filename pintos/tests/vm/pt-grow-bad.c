/* Read from an address 4,096 bytes below the stack pointer.
   The process must be terminated with -1 exit code. */
/* stack pointer보다 4,096 byte 아래 address에서 읽는다.
   process는 exit code -1로 종료되어야 한다. */

#include <string.h>
#include "tests/arc4.h"
#include "tests/cksum.h"
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void)
{
  asm volatile ("movq -4096(%rsp), %rax");
}
