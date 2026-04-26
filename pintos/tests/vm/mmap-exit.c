/* Executes child-mm-wrt and verifies that the writes that should
   have occurred really did. */
/* child-mm-wrt를 실행하고 발생해야 하는 write가 실제로 발생했는지 검증한다. */

#include <syscall.h>
#include "tests/vm/sample.inc"
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void)
{
  pid_t child;

  /* Make child write file. */
  /* child가 file에 쓰도록 한다. */
  quiet = true;
	child = fork("child-mm-wrt");
	if (child == 0) {
		CHECK ((child = exec ("child-mm-wrt")) != -1, "exec \"child-mm-wrt\"");
	} else {
		CHECK (wait (child) == 0, "wait for child (should return 0)");
		quiet = false;
		
		/* Check file contents. */
		/* file 내용을 확인한다. */
		check_file ("sample.txt", sample, sizeof sample);
	} 
}
