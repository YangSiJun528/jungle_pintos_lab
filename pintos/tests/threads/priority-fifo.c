/* Creates several threads all at the same priority and ensures
   that they consistently run in the same round-robin order.

   Based on a test originally submitted for Stanford's CS 140 in
   winter 1999 by by Matt Franklin
   <startled@leland.stanford.edu>, Greg Hutchins
   <gmh@leland.stanford.edu>, Yu Ping Hu <yph@cs.stanford.edu>.
   Modified by arens. */
/* 같은 priority를 가진 여러 thread를 만들고, 이들이 항상 같은 round-robin 순서로
   실행되는지 확인한다.

   Stanford CS 140에 1999년 겨울 Matt Franklin
   <startled@leland.stanford.edu>, Greg Hutchins
   <gmh@leland.stanford.edu>, Yu Ping Hu <yph@cs.stanford.edu>가 제출한
   테스트를 바탕으로 한다. arens가 수정했다. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "devices/timer.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

struct simple_thread_data 
  {
    int id;                     /* Sleeper ID. */
                                /* sleeper ID. */
    int iterations;             /* Iterations so far. */
                                /* 지금까지의 반복 횟수. */
    struct lock *lock;          /* Lock on output. */
                                /* output에 대한 lock. */
    int **op;                   /* Output buffer position. */
                                /* output buffer 위치. */
  };

#define THREAD_CNT 16
#define ITER_CNT 16

static thread_func simple_thread_func;

void
test_priority_fifo (void) 
{
  struct simple_thread_data data[THREAD_CNT];
  struct lock lock;
  int *output, *op;
  int i, cnt;

  /* This test does not work with the MLFQS. */
  /* 이 테스트는 MLFQS에서는 동작하지 않는다. */
  ASSERT (!thread_mlfqs);

  /* Make sure our priority is the default. */
  /* 현재 priority가 기본값인지 확인한다. */
  ASSERT (thread_get_priority () == PRI_DEFAULT);

  msg ("%d threads will iterate %d times in the same order each time.",
       THREAD_CNT, ITER_CNT);
  msg ("If the order varies then there is a bug.");

  output = op = malloc (sizeof *output * THREAD_CNT * ITER_CNT * 2);
  ASSERT (output != NULL);
  lock_init (&lock);

  thread_set_priority (PRI_DEFAULT + 2);
  for (i = 0; i < THREAD_CNT; i++) 
    {
      char name[16];
      struct simple_thread_data *d = data + i;
      snprintf (name, sizeof name, "%d", i);
      d->id = i;
      d->iterations = 0;
      d->lock = &lock;
      d->op = &op;
      thread_create (name, PRI_DEFAULT + 1, simple_thread_func, d);
    }

  thread_set_priority (PRI_DEFAULT);
  /* All the other threads now run to termination here. */
  /* 이제 다른 모든 thread가 여기서 종료될 때까지 실행된다. */
  ASSERT (lock.holder == NULL);

  cnt = 0;
  for (; output < op; output++) 
    {
      struct simple_thread_data *d;

      ASSERT (*output >= 0 && *output < THREAD_CNT);
      d = data + *output;
      if (cnt % THREAD_CNT == 0)
        printf ("(priority-fifo) iteration:");
      printf (" %d", d->id);
      if (++cnt % THREAD_CNT == 0)
        printf ("\n");
      d->iterations++;
    }
}

static void 
simple_thread_func (void *data_) 
{
  struct simple_thread_data *data = data_;
  int i;
  
  for (i = 0; i < ITER_CNT; i++) 
    {
      lock_acquire (data->lock);
      *(*data->op)++ = data->id;
      lock_release (data->lock);
      thread_yield ();
    }
}
