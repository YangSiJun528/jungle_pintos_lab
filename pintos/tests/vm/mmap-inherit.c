/* Maps a file into memory and runs child-inherit to verify that
   mappings are not inherited. */
/* file을 memory에 mapping하고 child-inherit를 실행하여
   mapping이 상속되지 않는지 검증한다. */

#include <string.h>
#include <syscall.h>
#include "tests/vm/sample.inc"
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void)
{
  char *actual = (char *) 0x54321000;
  int handle;
  pid_t child;

  /* Open file, map, verify data. */
  /* file을 열고 mapping한 뒤 data를 검증한다. */
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
  CHECK (mmap (actual, 4096, 0, handle, 0) != MAP_FAILED, "mmap \"sample.txt\"");
  if (memcmp (actual, sample, strlen (sample)))
    fail ("read of mmap'd file reported bad data");

	/* Spawn child and wait. */
	/* child를 생성하고 기다린다. */
	child = fork("child-inherit");
	if (child == 0) {
		CHECK (exec ("child-inherit") != -1, "exec \"child-inherit\"");
	}	else {
		quiet = true;
		CHECK (wait (child) == -1, "wait for child (should return -1)");
		quiet = false;
	}

  /* Verify data again. */
  /* data를 다시 검증한다. */
  CHECK (!memcmp (actual, sample, strlen (sample)),
         "checking that mmap'd file still has same data");
}
