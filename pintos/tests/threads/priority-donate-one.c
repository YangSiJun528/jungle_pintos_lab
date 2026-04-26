/* The main thread acquires a lock.  Then it creates two
   higher-priority threads that block acquiring the lock, causing
   them to donate their priorities to the main thread.  When the
   main thread releases the lock, the other threads should
   acquire it in priority order.

   Based on a test originally submitted for Stanford's CS 140 in
   winter 1999 by Matt Franklin <startled@leland.stanford.edu>,
   Greg Hutchins <gmh@leland.stanford.edu>, Yu Ping Hu
   <yph@cs.stanford.edu>.  Modified by arens. */
/* main thread가 lock을 획득한다. 그런 다음 lock 획득을 기다리며 block되는 더 높은
   priority의 thread 두 개를 만들어, 이들이 main thread에 priority를 donation하게
   한다. main thread가 lock을 release하면 다른 thread들이 priority 순서대로 lock을
   획득해야 한다.

   Stanford CS 140에 1999년 겨울 Matt Franklin <startled@leland.stanford.edu>,
   Greg Hutchins <gmh@leland.stanford.edu>, Yu Ping Hu <yph@cs.stanford.edu>가
   제출한 테스트를 바탕으로 한다. arens가 수정했다. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/synch.h"
#include "threads/thread.h"

static thread_func acquire1_thread_func;
static thread_func acquire2_thread_func;

void
test_priority_donate_one (void) 
{
  struct lock lock;

  /* This test does not work with the MLFQS. */
  /* 이 테스트는 MLFQS에서는 동작하지 않는다. */
  ASSERT (!thread_mlfqs);

  /* Make sure our priority is the default. */
  /* 현재 priority가 기본값인지 확인한다. */
  ASSERT (thread_get_priority () == PRI_DEFAULT);

  lock_init (&lock);
  lock_acquire (&lock);
  thread_create ("acquire1", PRI_DEFAULT + 1, acquire1_thread_func, &lock);
  msg ("This thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT + 1, thread_get_priority ());
  thread_create ("acquire2", PRI_DEFAULT + 2, acquire2_thread_func, &lock);
  msg ("This thread should have priority %d.  Actual priority: %d.",
       PRI_DEFAULT + 2, thread_get_priority ());
  lock_release (&lock);
  msg ("acquire2, acquire1 must already have finished, in that order.");
  msg ("This should be the last line before finishing this test.");
}

static void
acquire1_thread_func (void *lock_) 
{
  struct lock *lock = lock_;

  lock_acquire (lock);
  msg ("acquire1: got the lock");
  lock_release (lock);
  msg ("acquire1: done");
}

static void
acquire2_thread_func (void *lock_) 
{
  struct lock *lock = lock_;

  lock_acquire (lock);
  msg ("acquire2: got the lock");
  lock_release (lock);
  msg ("acquire2: done");
}
