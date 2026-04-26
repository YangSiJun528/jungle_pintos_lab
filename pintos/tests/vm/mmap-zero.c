/* Tries to map a zero-length file, which may or may not work but
   should not terminate the process or crash.
   Then dereferences the address that we tried to map,
   and the process must be terminated with -1 exit code. */
/* 길이가 0인 file을 mapping하려고 시도한다. 이는 성공할 수도 실패할 수도 있지만,
   process를 종료하거나 crash시키면 안 된다.
   그런 다음 mapping하려고 했던 address를 역참조하며,
   process는 exit code -1로 종료되어야 한다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void)
{
  char *data = (char *) 0x7f000000;
  int handle;

  CHECK (create ("empty", 0), "create empty file \"empty\"");
  CHECK ((handle = open ("empty")) > 1, "open \"empty\"");

  /* Calling mmap() might succeed or fail.  We don't care. */
  /* mmap() 호출은 성공할 수도 실패할 수도 있다. 여기서는 상관하지 않는다. */
  msg ("mmap \"empty\"");
  mmap (data, 0, 0, handle, 0);

  /* Regardless of whether the call worked, *data should cause
     the process to be terminated. */
  /* call 성공 여부와 관계없이 *data는 process를 종료시켜야 한다. */
  fail ("unmapped memory is readable (%d)", *data);
}

