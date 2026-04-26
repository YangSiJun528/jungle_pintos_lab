/* Creates N threads, each of which sleeps a different, fixed
   duration, M times.  Records the wake-up order and verifies
   that it is valid. */
/* N개의 thread를 만들고, 각 thread가 서로 다른 고정 시간 동안 M번 sleep하게 한다.
   깨어난 순서를 기록하고 그 순서가 유효한지 검증한다. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"

static void test_sleep (int thread_cnt, int iterations);

void
test_alarm_simultaneous (void) 
{
  test_sleep (3, 5);
}

/* Information about the test. */
/* 테스트에 대한 정보. */
struct sleep_test 
  {
    int64_t start;              /* Current time at start of test. */
                                /* 테스트 시작 시점의 현재 시간. */
    int iterations;             /* Number of iterations per thread. */
                                /* thread별 반복 횟수. */
    int *output_pos;            /* Current position in output buffer. */
                                /* output buffer의 현재 위치. */
  };

static void sleeper (void *);

/* Runs THREAD_CNT threads thread sleep ITERATIONS times each. */
/* THREAD_CNT개의 thread가 각각 ITERATIONS번 thread sleep을 실행한다. */
static void
test_sleep (int thread_cnt, int iterations) 
{
  struct sleep_test test;
  int *output;
  int i;

  /* This test does not work with the MLFQS. */
  /* 이 테스트는 MLFQS에서는 동작하지 않는다. */
  ASSERT (!thread_mlfqs);

  msg ("Creating %d threads to sleep %d times each.", thread_cnt, iterations);
  msg ("Each thread sleeps 10 ticks each time.");
  msg ("Within an iteration, all threads should wake up on the same tick.");

  /* Allocate memory. */
  /* 메모리를 할당한다. */
  output = malloc (sizeof *output * iterations * thread_cnt * 2);
  if (output == NULL)
    PANIC ("couldn't allocate memory for test");

  /* Initialize test. */
  /* 테스트를 초기화한다. */
  test.start = timer_ticks () + 100;
  test.iterations = iterations;
  test.output_pos = output;

  /* Start threads. */
  /* thread들을 시작한다. */
  ASSERT (output != NULL);
  for (i = 0; i < thread_cnt; i++)
    {
      char name[16];
      snprintf (name, sizeof name, "thread %d", i);
      thread_create (name, PRI_DEFAULT, sleeper, &test);
    }
  
  /* Wait long enough for all the threads to finish. */
  /* 모든 thread가 끝나기에 충분한 시간 동안 기다린다. */
  timer_sleep (100 + iterations * 10 + 100);

  /* Print completion order. */
  /* 완료 순서를 출력한다. */
  msg ("iteration 0, thread 0: woke up after %d ticks", output[0]);
  for (i = 1; i < test.output_pos - output; i++) 
    msg ("iteration %d, thread %d: woke up %d ticks later",
         i / thread_cnt, i % thread_cnt, output[i] - output[i - 1]);
  
  free (output);
}

/* Sleeper thread. */
/* sleeper thread. */
static void
sleeper (void *test_) 
{
  struct sleep_test *test = test_;
  int i;

  /* Make sure we're at the beginning of a timer tick. */
  /* timer tick의 시작 지점에 있는지 확인한다. */
  timer_sleep (1);

  for (i = 1; i <= test->iterations; i++) 
    {
      int64_t sleep_until = test->start + i * 10;
      timer_sleep (sleep_until - timer_ticks ());
      *test->output_pos++ = timer_ticks () - test->start;
      thread_yield ();
    }
}
