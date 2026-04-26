/* Child process for syn-read test.
   Reads the contents of a test file a byte at a time, in the
   hope that this will take long enough that we can get a
   significant amount of contention in the kernel file system
   code. */
/* syn-read 테스트용 child process.
   test file의 내용을 한 번에 1 byte씩 읽는다.
   이 작업이 충분히 오래 걸려 kernel file system code에서
   상당한 contention이 생기기를 기대한다. */

#include <random.h>
#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include "tests/lib.h"
#include "tests/filesys/base/syn-read.h"

static char buf[BUF_SIZE];

int
main (int argc, const char *argv[]) 
{
  test_name = "child-syn-read";

  int child_idx;
  int fd;
  size_t i;

  quiet = true;
  
  CHECK (argc == 2, "argc must be 2, actually %d", argc);
  child_idx = atoi (argv[1]);

  random_init (0);
  random_bytes (buf, sizeof buf);

  CHECK ((fd = open (file_name)) > 1, "open \"%s\"", file_name);
  for (i = 0; i < sizeof buf; i++) 
    {
      char c;
      CHECK (read (fd, &c, 1) > 0, "read \"%s\"", file_name);
      compare_bytes (&c, buf + i, 1, i, file_name);
    }
  close (fd);

  return child_idx;
}

