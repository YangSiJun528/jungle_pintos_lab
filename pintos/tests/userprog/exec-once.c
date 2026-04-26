/* Executes and waits for a single child process. */
/* 단일 child process를 실행하고 기다린다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  msg ("I'm your father");
  exec ("child-simple");
}
