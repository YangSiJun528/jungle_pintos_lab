/* Ensures that a high-priority thread really preempts.

   Based on a test originally submitted for Stanford's CS 140 in
   winter 1999 by by Matt Franklin
   <startled@leland.stanford.edu>, Greg Hutchins
   <gmh@leland.stanford.edu>, Yu Ping Hu <yph@cs.stanford.edu>.
   Modified by arens. */
/* 높은 priority의 thread가 실제로 preempt하는지 확인한다.

   Stanford CS 140에 1999년 겨울 Matt Franklin
   <startled@leland.stanford.edu>, Greg Hutchins
   <gmh@leland.stanford.edu>, Yu Ping Hu <yph@cs.stanford.edu>가 제출한
   테스트를 바탕으로 한다. arens가 수정했다. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/synch.h"
#include "threads/thread.h"

static thread_func simple_thread_func;

void
test_priority_preempt (void) 
{
  /* This test does not work with the MLFQS. */
  /* 이 테스트는 MLFQS에서는 동작하지 않는다. */
  ASSERT (!thread_mlfqs);

  /* Make sure our priority is the default. */
  /* 현재 priority가 기본값인지 확인한다. */
  ASSERT (thread_get_priority () == PRI_DEFAULT);

  thread_create ("high-priority", PRI_DEFAULT + 1, simple_thread_func, NULL);
  msg ("The high-priority thread should have already completed.");
}

static void 
simple_thread_func (void *aux UNUSED) 
{
  int i;
  
  for (i = 0; i < 5; i++) 
    {
      msg ("Thread %s iteration %d", thread_name (), i);
      thread_yield ();
    }
  msg ("Thread %s done!", thread_name ());
}
