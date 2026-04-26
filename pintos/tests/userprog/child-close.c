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
#include <syscall.h>
#include "tests/userprog/sample.inc"
#include "tests/lib.h"

int
main (int argc UNUSED, char *argv[]) 
{
  test_name = "child-close";

  msg ("begin");
  
  if (!isdigit (*argv[1]))
    fail ("bad command-line arguments");
  
  int handle = atoi (argv[1]);
  check_file_handle (handle, "sample.txt", sample, sizeof sample - 1);

  close (handle);
  msg ("end");

  return 0;
}
