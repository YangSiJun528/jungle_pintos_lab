/* Child process for syn-rw.
   Reads from a file created by our parent process, which is
   growing it.  We loop until we've read the whole file
   successfully.  Many iterations through the loop will return 0
   bytes, because the file has not grown in the meantime.  That
   is, we are "busy waiting" for the file to grow.
   (This test could be improved by adding a "yield" system call
   and calling yield whenever we receive a 0-byte read.) */
/* syn-rw용 child process.
   parent process가 만들고 grow시키는 file에서 읽는다.
   전체 file을 성공적으로 읽을 때까지 loop를 돈다.
   그동안 file이 grow하지 않았기 때문에 loop의 많은 iteration은 0 byte를 반환한다.
   즉 file이 grow하기를 "busy waiting"한다.
   ("yield" system call을 추가하고 0-byte read를 받을 때마다 yield를 호출하면
   이 테스트를 개선할 수 있다.) */

#include <random.h>
#include <stdlib.h>
#include <syscall.h>
#include "tests/filesys/extended/syn-rw.h"
#include "tests/lib.h"

static char buf1[BUF_SIZE];
static char buf2[BUF_SIZE];

int
main (int argc, const char *argv[]) 
{
  test_name = "child-syn-rw";

  int child_idx;
  int fd;
  size_t ofs;

  quiet = true;
  
  CHECK (argc == 2, "argc must be 2, actually %d", argc);
  child_idx = atoi (argv[1]);

  random_init (0);
  random_bytes (buf1, sizeof buf1);

  CHECK ((fd = open (file_name)) > 1, "open \"%s\"", file_name);
  ofs = 0;
  while (ofs < sizeof buf2)
    {
      int bytes_read = read (fd, buf2 + ofs, sizeof buf2 - ofs);
      CHECK (bytes_read >= -1 && bytes_read <= (int) (sizeof buf2 - ofs),
             "%zu-byte read on \"%s\" returned invalid value of %d",
             sizeof buf2 - ofs, file_name, bytes_read);
      if (bytes_read > 0) 
        {
          compare_bytes (buf2 + ofs, buf1 + ofs, bytes_read, ofs, file_name);
          ofs += bytes_read;
        }
    }
  close (fd);

  return child_idx;
}
