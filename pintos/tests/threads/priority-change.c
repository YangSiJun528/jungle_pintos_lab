/* Verifies that lowering a thread's priority so that it is no
   longer the highest-priority thread in the system causes it to
   yield immediately. */
/* thread의 priority를 낮춰 더 이상 시스템에서 가장 높은 priority thread가 아니게
   만들면 즉시 yield하는지 검증한다. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/thread.h"

static thread_func changing_thread;

void
test_priority_change (void) 
{
  /* This test does not work with the MLFQS. */
  /* 이 테스트는 MLFQS에서는 동작하지 않는다. */
  ASSERT (!thread_mlfqs);

  msg ("Creating a high-priority thread 2.");
  thread_create ("thread 2", PRI_DEFAULT + 1, changing_thread, NULL);
  msg ("Thread 2 should have just lowered its priority.");
  thread_set_priority (PRI_DEFAULT - 2);
  msg ("Thread 2 should have just exited.");
}

static void
changing_thread (void *aux UNUSED) 
{
  msg ("Thread 2 now lowering priority.");
  thread_set_priority (PRI_DEFAULT - 1);
  msg ("Thread 2 exiting.");
}
