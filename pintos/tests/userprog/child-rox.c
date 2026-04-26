/* Child process run by rox-child and rox-multichild tests.
   Opens and tries to write to its own executable, verifying that
   that is disallowed.
   Then recursively executes itself to the depth indicated by the
   first command-line argument. */
/* rox-child와 rox-multichild 테스트가 실행하는 child process.
   자신의 executable을 열고 write를 시도하여 그 동작이 허용되지 않는지 검증한다.
   이후 첫 번째 command-line argument가 나타내는 깊이까지 자기 자신을 재귀적으로
   실행한다. */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include "tests/lib.h"

static void
try_write (void) 
{
  int handle;
  char buffer[19];

  quiet = true;
  CHECK ((handle = open ("child-rox")) > 1, "open \"child-rox\"");
  quiet = false;

  CHECK (write (handle, buffer, sizeof buffer) == 0,
         "try to write \"child-rox\"");
  
  close (handle);
}

int
main (int argc UNUSED, char *argv[]) 
{
  test_name = "child-rox";

  msg ("begin");
  try_write ();

  if (!isdigit (*argv[1]))
    fail ("bad command-line arguments");
  if (atoi (argv[1]) > 1) 
    {
      char cmd[128];
      int child;
      
      snprintf (cmd, sizeof cmd, "child-rox %d", atoi (argv[1]) - 1);
      msg ("exec \"%s\"", cmd);
      if (!(child = fork ("child-rox"))){
        exec (cmd);
      }

      if (child < 0)
        fail ("fork() returned %d", child);
      quiet = true;
      CHECK (wait (child) == 12, "wait for \"child-rox\"");
      quiet = false;
    }

  try_write ();
  msg ("end");

  return 12;
}
