/* Forks a thread whose name spans the boundary between two pages.
   This is valid, so it must succeed. */
/* 이름이 두 page 사이의 boundary에 걸쳐 있는 thread를 fork한다.
   이는 유효하므로 성공해야 한다. */

#include <syscall.h>
#include "tests/userprog/boundary.h"
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  pid_t pid = fork (copy_string_across_boundary ("child-simple"));
  if (pid == 0){
    msg ("child run");
    exit(54);
  } else {
    int exit_val = wait(pid);
    CHECK (pid > 0, "fork");
    CHECK (exit_val == 54, "wait");
  }
}
