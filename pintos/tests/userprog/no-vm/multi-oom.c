/* Recursively forks until the child fails to fork.
   We expect that at least 28 copies can run.
   
   We count how many children your kernel was able to execute
   before it fails to start a new process.  We require that,
   if a process doesn't actually get to start, exec() must
   return -1, not a valid PID.

   We repeat this process 10 times, checking that your kernel
   allows for the same level of depth every time.

   In addition, some processes will spawn children that terminate
   abnormally after allocating some resources.

   We set EXPECTED_DEPTH_TO_PASS heuristically by
   giving *large* margin on the value from our implementation.
   If you seriously think there is no memory leak in your code
   but it fails with EXPECTED_DEPTH_TO_PASS,
   please manipulate it and report us the actual output.
   
   Orignally written by Godmar Back <godmar@gmail.com>
   Modified by Minkyu Jung, Jinyoung Oh <cs330_ta@casys.kaist.ac.kr>
*/
/* child가 fork에 실패할 때까지 재귀적으로 fork한다.
   최소 28개의 복사본이 실행될 수 있다고 기대한다.

   새 process 시작에 실패하기 전까지 kernel이 몇 개의 child를 실행할 수 있었는지 센다.
   process가 실제로 시작되지 못했다면 exec()는 유효한 PID가 아니라 -1을 반환해야 한다.

   이 과정을 10번 반복하여 kernel이 매번 같은 수준의 깊이를 허용하는지 확인한다.

   추가로 일부 process는 resource를 할당한 뒤 비정상 종료되는 child를 생성한다.

   EXPECTED_DEPTH_TO_PASS는 우리 구현의 값에 큰 margin을 주어 heuristic하게 설정했다.
   코드에 memory leak이 없다고 확신하는데 EXPECTED_DEPTH_TO_PASS에서 실패한다면,
   값을 조정하고 실제 출력을 보고하라.

   원래 Godmar Back <godmar@gmail.com>이 작성했다.
   Minkyu Jung, Jinyoung Oh <cs330_ta@casys.kaist.ac.kr>가 수정했다. */

#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <syscall.h>
#include <random.h>
#include "tests/lib.h"

static const int EXPECTED_DEPTH_TO_PASS = 10;
static const int EXPECTED_REPETITIONS = 10;

int make_children (void);

/* Open a number of files (and fail to close them).
   The kernel must free any kernel resources associated
   with these file descriptors. */
/* 여러 file을 열고 닫지 않는다.
   kernel은 이 file descriptor들과 관련된 모든 kernel resource를 해제해야 한다. */
static void
consume_some_resources (void)
{
  int fd, fdmax = 126;

  /* Open as many files as we can, up to fdmax.
	 Depending on how file descriptors are allocated inside
	 the kernel, open() may fail if the kernel is low on memory.
	 A low-memory condition in open() should not lead to the
	 termination of the process.  */
  /* fdmax까지 가능한 한 많은 file을 연다.
     kernel 내부에서 file descriptor가 할당되는 방식에 따라,
     kernel memory가 부족하면 open()이 실패할 수 있다.
     open()의 low-memory condition이 process 종료로 이어지면 안 된다. */
  for (fd = 0; fd < fdmax; fd++) {
#ifdef EXTRA2
	  if (fd != 0 && (random_ulong () & 1)) {
		if (dup2(random_ulong () % fd, fd+fdmax) == -1)
			break;
		else
			if (open (test_name) == -1)
			  break;
	  }
#else
		if (open (test_name) == -1)
		  break;
#endif
  }
}

/* Consume some resources, then terminate this process
   in some abnormal way.  */
/* 일부 resource를 소비한 뒤 이 process를 비정상적인 방식으로 종료한다. */
static int NO_INLINE
consume_some_resources_and_die (void)
{
  consume_some_resources ();
  int *KERN_BASE = (int *)0x8004000000;

  switch (random_ulong () % 5) {
	case 0:
	  *(int *) NULL = 42;
    break;

	case 1:
	  return *(int *) NULL;

	case 2:
	  return *KERN_BASE;

	case 3:
	  *KERN_BASE = 42;
    break;

	case 4:
	  open ((char *)KERN_BASE);
	  exit (-1);
    break;

	default:
	  NOT_REACHED ();
  }
  return 0;
}

int
make_children (void) {
  int i = 0;
  int pid;
  char child_name[128];
  for (; ; random_init (i), i++) {
    if (i > EXPECTED_DEPTH_TO_PASS/2) {
      snprintf (child_name, sizeof child_name, "%s_%d_%s", "child", i, "X");
      pid = fork(child_name);
      if (pid > 0 && wait (pid) != -1) {
        fail ("crashed child should return -1.");
      } else if (pid == 0) {
        consume_some_resources_and_die();
        fail ("Unreachable");
      }
    }

    snprintf (child_name, sizeof child_name, "%s_%d_%s", "child", i, "O");
    pid = fork(child_name);
    if (pid < 0) {
      exit (i);
    } else if (pid == 0) {
      consume_some_resources();
    } else {
      break;
    }
  }

  int depth = wait (pid);
  if (depth < 0)
	  fail ("Should return > 0.");

  if (i == 0)
	  return depth;
  else
	  exit (depth);
}

int
main (int argc UNUSED, char *argv[] UNUSED) {
  test_name = "multi-oom";

  msg ("begin");

  int first_run_depth = make_children ();
  CHECK (first_run_depth >= EXPECTED_DEPTH_TO_PASS, "Spawned at least %d children.", EXPECTED_DEPTH_TO_PASS);

  for (int i = 0; i < EXPECTED_REPETITIONS; i++) {
    int current_run_depth = make_children();
    if (current_run_depth < first_run_depth) {
      fail ("should have forked at least %d times, but %d times forked", 
              first_run_depth, current_run_depth);
    }
  }

  msg ("success. Program forked %d iterations.", EXPECTED_REPETITIONS);
  msg ("end");
}
