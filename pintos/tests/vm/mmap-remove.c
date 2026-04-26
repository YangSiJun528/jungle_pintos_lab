/* Deletes and closes file that is mapped into memory
   and verifies that it can still be read through the mapping. */
/* memory에 mapping된 file을 delete하고 close한 뒤에도
   mapping을 통해 계속 읽을 수 있는지 검증한다. */

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

  /* Map file. */
  /* file을 mapping한다. */
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
  CHECK ((map = mmap (actual, 4096, 0, handle, 0)) != MAP_FAILED, "mmap \"sample.txt\"");

  /* Close file and delete it. */
  /* file을 close하고 delete한다. */
  close (handle);
  CHECK (remove ("sample.txt"), "remove \"sample.txt\"");
  CHECK (open ("sample.txt") == -1, "try to open \"sample.txt\"");

  /* Create a new file in hopes of overwriting data from the old
     one, in case the file system has incorrectly freed the
     file's data. */
  /* file system이 file의 data를 잘못 free했을 경우를 대비해,
     기존 data를 overwrite하기를 기대하며 새 file을 만든다. */
  CHECK (create ("another", 4096 * 10), "create \"another\"");

  /* Check that mapped data is correct. */
  /* mapped data가 올바른지 확인한다. */
  if (memcmp (actual, sample, strlen (sample)))
    fail ("read of mmap'd file reported bad data");

  /* Verify that data is followed by zeros. */
  /* data 뒤에 zero가 이어지는지 검증한다. */
  for (i = strlen (sample); i < 4096; i++)
    if (actual[i] != 0)
      fail ("byte %zu of mmap'd region has value %02hhx (should be 0)",
            i, actual[i]);

  munmap (map);
}
