/* The main thread acquires locks A and B, then it creates three
   higher-priority threads.  The first two of these threads block
   acquiring one of the locks and thus donate their priority to
   the main thread.  The main thread releases the locks in turn
   and relinquishes its donated priorities, allowing the third thread
   to run.

   In this test, the main thread releases the locks in a different
   order compared to priority-donate-multiple.c.
   
   Written by Godmar Back <gback@cs.vt.edu>. 
   Based on a test originally submitted for Stanford's CS 140 in
   winter 1999 by Matt Franklin <startled@leland.stanford.edu>,
   Greg Hutchins <gmh@leland.stanford.edu>, Yu Ping Hu
   <yph@cs.stanford.edu>.  Modified by arens. */
/* main thread가 lock A와 B를 획득한 뒤 더 높은 priority의 thread 세 개를 만든다.
   이 중 앞의 두 thread는 lock 중 하나를 획득하려다 block되며 main thread에
   priority를 donation한다. main thread는 lock을 차례로 release하고 donation된
   priority를 내려놓아 세 번째 thread가 실행될 수 있게 한다.

   이 테스트에서는 priority-donate-multiple.c와 비교해 main thread가 lock을 다른
   순서로 release한다.

   Godmar Back <gback@cs.vt.edu>가 작성했다.
   Stanford CS 140에 1999년 겨울 Matt Franklin <startled@leland.stanford.edu>,
   Greg Hutchins <gmh@leland.stanford.edu>, Yu Ping Hu <yph@cs.stanford.edu>가
   제출한 테스트를 바탕으로 한다. arens가 수정했다. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/synch.h"
#include "threads/thread.h"

static thread_func a_thread_func;
static thread_func b_thread_func;
static thread_func c_thread_func;

void
test_priority_donate_multiple2 (void) 
{
  struct lock a, b;

  /* This test does not work with the MLFQS. */
  /* 이 테스트는 MLFQS에서는 동작하지 않는다. */
  ASSERT (!thread_mlfqs);

  /* Make sure our priority is the default. */
  /* 현재 priority가 기본값인지 확인한다. */
  ASSERT (thread_get_priority () == PRI_DEFAULT);

  lock_init (&a);
  lock_init (&b);

  lock_acquire (&a);
  lock_acquire (&b);

  thread_create ("a", PRI_DEFAULT + 3, a_thread_func, &a);
  msg ("Main thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT + 3, thread_get_priority ());

  thread_create ("c", PRI_DEFAULT + 1, c_thread_func, NULL);

  thread_create ("b", PRI_DEFAULT + 5, b_thread_func, &b);
  msg ("Main thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT + 5, thread_get_priority ());

  lock_release (&a);
  msg ("Main thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT + 5, thread_get_priority ());

  lock_release (&b);
  msg ("Threads b, a, c should have just finished, in that order.");
  msg ("Main thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT, thread_get_priority ());
}

static void
a_thread_func (void *lock_) 
{
  struct lock *lock = lock_;

  lock_acquire (lock);
  msg ("Thread a acquired lock a.");
  lock_release (lock);
  msg ("Thread a finished.");
}

static void
b_thread_func (void *lock_) 
{
  struct lock *lock = lock_;

  lock_acquire (lock);
  msg ("Thread b acquired lock b.");
  lock_release (lock);
  msg ("Thread b finished.");
}

static void
c_thread_func (void *a_ UNUSED) 
{
  msg ("Thread c finished.");
}
