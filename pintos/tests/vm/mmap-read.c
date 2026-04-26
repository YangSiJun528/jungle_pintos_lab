/* Uses a memory mapping to read a file. */
/* memory mapping을 사용해 file을 읽는다. */

#include <string.h>
#include <syscall.h>
#include "tests/vm/sample.inc"
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void)
{
  char *actual = (char *) 0x10000000;
  int handle;
  void *map;
  size_t i;

  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
  CHECK ((map = mmap (actual, 4096, 0, handle, 0)) != MAP_FAILED, "mmap \"sample.txt\"");

  /* Check that data is correct. */
  /* data가 올바른지 확인한다. */
  if (memcmp (actual, sample, strlen (sample)))
    fail ("read of mmap'd file reported bad data");

  /* Verify that data is followed by zeros. */
  /* data 뒤에 zero가 이어지는지 검증한다. */
  for (i = strlen (sample); i < 4096; i++)
    if (actual[i] != 0)
      fail ("byte %zu of mmap'd region has value %02hhx (should be 0)",
            i, actual[i]);

  munmap (map);
  close (handle);
}
