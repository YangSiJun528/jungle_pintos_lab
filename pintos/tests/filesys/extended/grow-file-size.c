/* Grows a file from 0 bytes to 2,134 bytes, 37 bytes at a time,
   and checks that the file's size is reported correctly at each
   step. */
/* file을 0 byte에서 2,134 byte까지 한 번에 37 byte씩 grow시키고,
   각 단계에서 file size가 올바르게 보고되는지 확인한다. */

#include <syscall.h>
#include "tests/filesys/seq-test.h"
#include "tests/lib.h"
#include "tests/main.h"

static char buf[2134];

static size_t
return_block_size (void) 
{
  return 37;
}

static void
check_file_size (int fd, long ofs) 
{
  long size = filesize (fd);
  if (size != ofs)
    fail ("filesize not updated properly: should be %ld, actually %ld",
          ofs, size);
}

void
test_main (void) 
{
  seq_test ("testfile",
            buf, sizeof buf, 0,
            return_block_size, check_file_size);
}
