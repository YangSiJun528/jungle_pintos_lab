/* Opens a file and then runs a subprocess that tries to close
   the file.  (Pintos does not have inheritance of file handles,
   so this must fail.)  The parent process then attempts to use
   the file handle, which must succeed. */
/* file을 열고, 그 file을 close하려고 시도하는 subprocess를 실행한다. Pintos는 file
   handle 상속이 없으므로 이 close는 실패해야 한다. 그 다음 parent process는 해당
   file handle을 사용하려고 시도하며, 이는 성공해야 한다. */

#include <stdio.h>
#include <syscall.h>
#include "tests/userprog/sample.inc"
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  char child_cmd[128];
  int handle;

  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");

  snprintf (child_cmd, sizeof child_cmd, "child-close %d", handle);
  
  pid_t pid;
  if (!(pid = fork("child-close"))){
    exec (child_cmd);
  }
  msg ("wait(exec()) = %d", wait (pid));

  check_file_handle (handle, "sample.txt", sample, sizeof sample - 1);
}
