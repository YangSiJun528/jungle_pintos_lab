/* The main thread set its priority to PRI_MIN and creates 7 threads 
   (thread 1..7) with priorities PRI_MIN + 3, 6, 9, 12, ...
   The main thread initializes 8 locks: lock 0..7 and acquires lock 0.

   When thread[i] starts, it first acquires lock[i] (unless i == 7.)
   Subsequently, thread[i] attempts to acquire lock[i-1], which is held by
   thread[i-1], except for lock[0], which is held by the main thread.
   Because the lock is held, thread[i] donates its priority to thread[i-1],
   which donates to thread[i-2], and so on until the main thread
   receives the donation.

   After threads[1..7] have been created and are blocked on locks[0..7],
   the main thread releases lock[0], unblocking thread[1], and being
   preempted by it.
   Thread[1] then completes acquiring lock[0], then releases lock[0],
   then releases lock[1], unblocking thread[2], etc.
   Thread[7] finally acquires & releases lock[7] and exits, allowing 
   thread[6], then thread[5] etc. to run and exit until finally the 
   main thread exits.

   In addition, interloper threads are created at priority levels
   p = PRI_MIN + 2, 5, 8, 11, ... which should not be run until the 
   corresponding thread with priority p + 1 has finished.
  
   Written by Godmar Back <gback@cs.vt.edu> */ 
/* main thread는 자신의 priority를 PRI_MIN으로 설정하고, PRI_MIN + 3, 6, 9,
   12, ... priority를 가진 7개 thread(thread 1..7)를 만든다. main thread는
   lock 0..7, 총 8개의 lock을 초기화하고 lock 0을 획득한다.

   thread[i]가 시작하면 먼저 lock[i]를 획득한다(i == 7인 경우 제외). 이후
   thread[i]는 thread[i-1]이 보유한 lock[i-1] 획득을 시도한다. lock[0]은 main
   thread가 보유한다. lock이 이미 보유 중이므로 thread[i]는 자신의 priority를
   thread[i-1]에 donation하고, thread[i-1]은 thread[i-2]에 donation하는 식으로
   main thread가 donation을 받을 때까지 이어진다.

   threads[1..7]이 생성되어 locks[0..7]에서 block된 뒤, main thread는 lock[0]을
   release하여 thread[1]을 unblock하고 thread[1]에 의해 preempt된다. thread[1]은
   lock[0] 획득을 완료한 뒤 lock[0]과 lock[1]을 release하여 thread[2]를 unblock하는
   식으로 진행한다. thread[7]은 마지막으로 lock[7]을 획득하고 release한 뒤 종료하며,
   이후 thread[6], thread[5] 등이 차례로 실행되고 종료되어 마지막으로 main thread가
   종료된다.

   추가로 p = PRI_MIN + 2, 5, 8, 11, ... priority를 가진 interloper thread들이
   생성되며, 이들은 대응되는 priority p + 1의 thread가 끝날 때까지 실행되면 안 된다.

   Godmar Back <gback@cs.vt.edu>가 작성했다. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/synch.h"
#include "threads/thread.h"

#define NESTING_DEPTH 8

struct lock_pair
  {
    struct lock *second;
    struct lock *first;
  };

static thread_func donor_thread_func;
static thread_func interloper_thread_func;

void
test_priority_donate_chain (void) 
{
  int i;  
  struct lock locks[NESTING_DEPTH - 1];
  struct lock_pair lock_pairs[NESTING_DEPTH];

  /* This test does not work with the MLFQS. */
  /* 이 테스트는 MLFQS에서는 동작하지 않는다. */
  ASSERT (!thread_mlfqs);

  thread_set_priority (PRI_MIN);

  for (i = 0; i < NESTING_DEPTH - 1; i++)
    lock_init (&locks[i]);

  lock_acquire (&locks[0]);
  msg ("%s got lock.", thread_name ());

  for (i = 1; i < NESTING_DEPTH; i++)
    {
      char name[16];
      int thread_priority;

      snprintf (name, sizeof name, "thread %d", i);
      thread_priority = PRI_MIN + i * 3;
      lock_pairs[i].first = i < NESTING_DEPTH - 1 ? locks + i: NULL;
      lock_pairs[i].second = locks + i - 1;

      thread_create (name, thread_priority, donor_thread_func, lock_pairs + i);
      msg ("%s should have priority %d.  Actual priority: %d.",
          thread_name (), thread_priority, thread_get_priority ());

      snprintf (name, sizeof name, "interloper %d", i);
      thread_create (name, thread_priority - 1, interloper_thread_func, NULL);
    }

  lock_release (&locks[0]);
  msg ("%s finishing with priority %d.", thread_name (),
                                         thread_get_priority ());
}

static void
donor_thread_func (void *locks_) 
{
  struct lock_pair *locks = locks_;

  if (locks->first)
    lock_acquire (locks->first);

  lock_acquire (locks->second);
  msg ("%s got lock", thread_name ());

  lock_release (locks->second);
  msg ("%s should have priority %d. Actual priority: %d", 
        thread_name (), (NESTING_DEPTH - 1) * 3,
        thread_get_priority ());

  if (locks->first)
    lock_release (locks->first);

  msg ("%s finishing with priority %d.", thread_name (),
                                         thread_get_priority ());
}

static void
interloper_thread_func (void *arg_ UNUSED)
{
  msg ("%s finished.", thread_name ());
}

// vim: sw=2
// vim 설정: shift width를 2로 둔다.
