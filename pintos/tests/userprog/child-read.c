/* Child process run by multi-child-fd test.

   Attempts to close the file descriptor passed as the first
   command-line argument. Since KAIST new Pintos inherits opened
   files descriptors over exec() for fork() system calls, 
   this should work well */
/* multi-child-fd 테스트가 실행하는 child process.

   첫 번째 command-line argument로 전달된 file descriptor를 close하려고 시도한다.
   KAIST new Pintos는 fork() system call을 위해 exec()를 거쳐 열린 file descriptor를
   상속하므로, 이 동작은 잘 처리되어야 한다. */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syscall.h>
#include "tests/userprog/boundary.h"
#include "tests/userprog/sample.inc"
#include "tests/lib.h"

int
main (int argc UNUSED, char *argv[]) 
{
  test_name = "child-read";

  int handle1, handle2;
  int byte_cnt;
  char *buffer;
  msg ("begin");
 
  if (!isdigit (*argv[1]))
    fail ("bad command-line arguments");
  
  handle1 = atoi (argv[1]);
  CHECK ((handle2 = open ("sample.txt")) > 1 && handle2 != handle1,
         "open \"sample.txt\"");

  buffer = get_boundary_area () - sizeof sample / 2;
  CHECK ((byte_cnt = read (handle2, buffer, 20)) == 20,
         "read \"sample.txt\" first 20 bytes");
  CHECK ((byte_cnt = read (handle1, buffer + 20, sizeof sample - 21))
                      == sizeof sample -21,
         "read \"sample.txt\" remainders");
  
  if (strcmp (sample, buffer)){
    msg ("expected text:\n%s", sample);
    msg ("text actually read:\n%s", buffer);
    fail ("expected text differs from actual");
  } else {
    msg ("Child success");
  }

  close (handle1);
  close (handle2);
  msg ("end");

  return 0;
}
