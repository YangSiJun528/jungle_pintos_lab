/* 이 파일은 Nachos의 소스 코드에서 파생되었습니다.
   교육용 운영체제. Nachos 저작권 고지
   아래에 전체 내용이 재현되어 있습니다. */

/* 저작권 (c) 1992-1996 캘리포니아 대학교 이사회.
   모든 권리 보유.

   이 소프트웨어를 사용, 복사, 수정 및 배포할 수 있는 권한
   어떤 목적으로든 수수료 없이 해당 문서를
   서면 동의 없이 이에 따라 승인됩니다.
   위의 저작권 표시와 다음 두 단락이 나타납니다.
   이 소프트웨어의 모든 복사본에 포함됩니다.

   어떠한 경우에도 캘리포니아 대학교는 이 소프트웨어와 문서의 사용으로
   인해 발생하는 직접, 간접, 특수, 부수 또는 결과적 손해에 대해
   책임지지 않습니다. 캘리포니아 대학교가 그러한 손해 가능성을
   통지받았더라도 마찬가지입니다.

   캘리포니아 대학교는 상품성 및 특정 목적 적합성에 대한 묵시적 보증을
   포함하되 이에 한정되지 않는 모든 보증을 명시적으로 부인합니다.
   이 소프트웨어는 "있는 그대로" 제공되며, 캘리포니아 대학교는 유지보수,
   지원, 업데이트, 개선 또는 수정 제공 의무를 지지 않습니다.
   */

#include "threads/synch.h"
#include <stdio.h>
#include <string.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

/* 세마포어 SEMA을 VALUE로 초기화합니다. 세마포어는
   음이 아닌 정수와 두 개의 원자 연산자
   조작하기:

   - down 또는 "P": 값이 양수가 될 때까지 기다린 다음 감소시킨다.

   - up or "V": increment the value (and wake up one waiting
   thread, if any). */
/* 세마포어 SEMA를 VALUE로 초기화한다. 세마포어는 음수가 아닌 정수와 이를
   조작하는 두 atomic operator로 이루어진다.

   - down 또는 "P": 값이 양수가 될 때까지 기다린 뒤 값을 감소시킨다.

   - up 또는 "V": 값을 증가시키고, 기다리는 스레드가 있으면 하나를 깨운다. */
void
sema_init (struct semaphore *sema, unsigned value) {
	ASSERT (sema != NULL);

	sema->value = value;
	list_init (&sema->waiters);
}

/* 세마포어에 대한 Down 또는 "P" 작업입니다. SEMA의 값을 기다립니다.
   양수로 변한 다음 원자적으로 감소시킵니다.

   This function may sleep, so it must not be called within an
   interrupt handler.  This function may be called with
   interrupts disabled, but if it sleeps then the next scheduled
   thread will probably turn interrupts back on. This is
   sema_down function. */
/* 세마포어에 대한 down 또는 "P" operation. SEMA의 값이 양수가 될 때까지 기다린
   뒤 atomic하게 감소시킨다.

   이 함수는 sleep할 수 있으므로 interrupt handler 안에서 호출하면 안 된다.
   인터럽트가 비활성화된 상태에서 호출할 수는 있지만, sleep하게 되면 다음에
   스케줄되는 스레드가 인터럽트를 다시 켤 가능성이 높다. 이것은 sema_down
   함수이다. */
void
sema_down (struct semaphore *sema) {
	enum intr_level old_level;

	ASSERT (sema != NULL);
	ASSERT (!intr_context ());

	old_level = intr_disable ();
	while (sema->value == 0) {
		//TODO(schd-2): waiters를 priority 기준으로 정렬되게 추가
		list_push_back (&sema->waiters, &thread_current ()->elem);
		thread_block ();
	}
	sema->value--;
	intr_set_level (old_level);
}

/* 세마포어에 대한 Down 또는 "P" 연산.
   세마포어가 아직 0이 아닙니다. 세마포어가 0인 경우 true를 반환합니다.
   감소하고, 그렇지 않으면 거짓입니다.

   This function may be called from an interrupt handler. */
/* 세마포어에 대한 down 또는 "P" operation이지만, 세마포어가 이미 0이 아닐 때만
   수행한다. 세마포어가 감소되면 true, 아니면 false를 리턴한다.

   이 함수는 interrupt handler에서 호출할 수 있다. */
bool
sema_try_down (struct semaphore *sema) {
	enum intr_level old_level;
	bool success;

	ASSERT (sema != NULL);

	old_level = intr_disable ();
	if (sema->value > 0)
	{
		sema->value--;
		success = true;
	}
	else
		success = false;
	intr_set_level (old_level);

	return success;
}

/* 세마포어에 대한 Up 또는 "V" 작업입니다. SEMA의 값을 증가시킵니다.
   SEMA 을 기다리는 스레드 중 하나가 있으면 깨웁니다.

   This function may be called from an interrupt handler. */
/* 세마포어에 대한 up 또는 "V" operation. SEMA의 값을 증가시키고, SEMA를 기다리는
   스레드가 있으면 그중 하나를 깨운다.

   이 함수는 interrupt handler에서 호출할 수 있다. */
void
sema_up (struct semaphore *sema) {
	enum intr_level old_level;

	ASSERT (sema != NULL);

	old_level = intr_disable ();
	if (!list_empty (&sema->waiters))
		//TODO(schd-2): 정렬되어있으므로 수정 필요 없음
		thread_unblock (list_entry (list_pop_front (&sema->waiters),
					struct thread, elem));
	sema->value++;
	//TODO(schd-1): thread_yield_if_needed 호출
	intr_set_level (old_level);
}

static void sema_test_helper (void *sema_);

/* Self-test for semaphores that makes control "ping-pong"
   between a pair of threads.  Insert calls to printf() to see
   what's going on. */
/* 두 스레드 사이에서 제어가 "ping-pong"되게 하는 세마포어 self-test.
   진행 상황을 보려면 printf() 호출을 넣어 본다. */
void
sema_self_test (void) {
	struct semaphore sema[2];
	int i;

	printf ("Testing semaphores...");
	sema_init (&sema[0], 0);
	sema_init (&sema[1], 0);
	thread_create ("sema-test", PRI_DEFAULT, sema_test_helper, &sema);
	for (i = 0; i < 10; i++)
	{
		sema_up (&sema[0]);
		sema_down (&sema[1]);
	}
	printf ("done.\n");
}

/* Thread function used by sema_self_test(). */
/* sema_self_test()에서 사용하는 스레드 함수. */
static void
sema_test_helper (void *sema_) {
	struct semaphore *sema = sema_;
	int i;

	for (i = 0; i < 10; i++)
	{
		sema_down (&sema[0]);
		sema_up (&sema[1]);
	}
}

/* LOCK을 초기화합니다. 잠금은 최대 한 명이 보유할 수 있습니다.
   스레드를 언제든지 사용할 수 있습니다. 우리의 잠금은 "재귀적"이 아닙니다.
   즉, 현재 잠금을 보유하고 있는 스레드에 대한 오류입니다.
   그 자물쇠를 얻으려고 노력하십시오.

   A lock is a specialization of a semaphore with an initial
   value of 1.  The difference between a lock and such a
   semaphore is twofold.  First, a semaphore can have a value
   greater than 1, but a lock can only be owned by a single
   thread at a time.  Second, a semaphore does not have an owner,
   meaning that one thread can "down" the semaphore and then
   another one "up" it, but with a lock the same thread must both
   acquire and release it.  When these restrictions prove
   onerous, it's a good sign that a semaphore should be used,
   instead of a lock. */
/* LOCK을 초기화한다. lock은 어떤 시점에도 최대 하나의 스레드만 보유할 수 있다.
   여기의 lock은 "recursive"가 아니다. 즉, 현재 lock을 들고 있는 스레드가 같은
   lock을 다시 acquire하려고 하면 에러이다.

   lock은 초기값이 1인 세마포어의 특수한 형태이다. lock과 그런 세마포어의 차이는
   두 가지이다. 첫째, 세마포어는 1보다 큰 값을 가질 수 있지만 lock은 한 번에 한
   스레드만 소유할 수 있다. 둘째, 세마포어에는 owner가 없다. 한 스레드가
   세마포어를 "down"하고 다른 스레드가 "up"할 수 있지만, lock은 같은 스레드가
   acquire와 release를 모두 해야 한다. 이러한 제약이 부담스럽다면 lock 대신
   세마포어를 써야 한다는 신호이다. */
void
lock_init (struct lock *lock) {
	ASSERT (lock != NULL);

	lock->holder = NULL;
	sema_init (&lock->semaphore, 1);
}

/* LOCK을 획득하고, 다음과 같은 경우 사용할 수 있을 때까지 잠자기합니다.
   필요한. 현재 잠금이 이미 보유되어 있지 않아야 합니다.
   실.

   This function may sleep, so it must not be called within an
   interrupt handler.  This function may be called with
   interrupts disabled, but interrupts will be turned back on if
   we need to sleep. */
/* LOCK을 acquire한다. 필요하면 사용 가능해질 때까지 sleep한다. lock은 현재
   스레드가 이미 들고 있으면 안 된다.

   이 함수는 sleep할 수 있으므로 interrupt handler 안에서 호출하면 안 된다.
   인터럽트가 비활성화된 상태에서 호출할 수는 있지만, sleep해야 하면 인터럽트가
   다시 켜진다. */
void
lock_acquire (struct lock *lock) {
	ASSERT (lock != NULL);
	ASSERT (!intr_context ());
	ASSERT (!lock_held_by_current_thread (lock));

	sema_down (&lock->semaphore);
	lock->holder = thread_current ();
}

/* LOCK 획득을 시도하고 성공하거나 거짓인 경우 true를 반환합니다.
   실패시. 현재 잠금이 이미 보유되어 있지 않아야 합니다.
   실.

   This function will not sleep, so it may be called within an
   interrupt handler. */
/* LOCK acquire를 시도하고 성공하면 true, 실패하면 false를 리턴한다. lock은 현재
   스레드가 이미 들고 있으면 안 된다.

   이 함수는 sleep하지 않으므로 interrupt handler 안에서 호출할 수 있다. */
bool
lock_try_acquire (struct lock *lock) {
	bool success;

	ASSERT (lock != NULL);
	ASSERT (!lock_held_by_current_thread (lock));

	success = sema_try_down (&lock->semaphore);
	if (success)
		lock->holder = thread_current ();
	return success;
}

/* 현재 스레드가 소유해야 하는 LOCK 을 해제합니다.
   lock_release 함수입니다.

   An interrupt handler cannot acquire a lock, so it does not
   make sense to try to release a lock within an interrupt
   handler. */
/* 현재 스레드가 소유해야 하는 LOCK을 release한다.
   이것은 lock_release 함수이다.

   interrupt handler는 lock을 acquire할 수 없으므로, interrupt handler 안에서
   lock을 release하려고 하는 것은 의미가 없다. */
void
lock_release (struct lock *lock) {
	ASSERT (lock != NULL);
	ASSERT (lock_held_by_current_thread (lock));

	lock->holder = NULL;
	sema_up (&lock->semaphore);
}

/* Returns true if the current thread holds LOCK, false
   otherwise.  (Note that testing whether some other thread holds
   a lock would be racy.) */
/* 현재 스레드가 LOCK을 들고 있으면 true, 아니면 false를 리턴한다.
   다른 스레드가 lock을 들고 있는지 검사하는 것은 racy하다는 점에 주의한다. */
bool
lock_held_by_current_thread (const struct lock *lock) {
	ASSERT (lock != NULL);

	return lock->holder == thread_current ();
}

/* One semaphore in a list. */
/* 리스트 안의 세마포어 하나. */
struct semaphore_elem {
	struct list_elem elem;              /* List element. */
	/* 리스트 element. */
	struct semaphore semaphore;         /* This semaphore. */
	/* 이 세마포어. */
};

/* Initializes condition variable COND.  A condition variable
   allows one piece of code to signal a condition and cooperating
   code to receive the signal and act upon it. */
/* condition variable COND를 초기화한다. condition variable은 한 코드 조각이
   condition을 signal하고, 협력하는 코드가 signal을 받아 동작할 수 있게 한다. */
void
cond_init (struct condition *cond) {
	ASSERT (cond != NULL);

	list_init (&cond->waiters);
}

/* LOCK을 원자적으로 해제하고 COND이 신호를 받을 때까지 기다립니다.
   다른 코드 조각. COND이 신호를 받은 후 LOCK은(는)
   돌아오기 전에 다시 획득했습니다. 호출하기 전에 LOCK을(를) 누르고 있어야 합니다.
   이 기능.

   이 기능으로 구현된 모니터는 "Mesa" 스타일이 아닌 "Mesa" 스타일입니다.
   "Hoare" 스타일, 즉 신호를 보내고 받는 것은
   원자 연산. 따라서 일반적으로 호출자는 다시 확인해야 합니다.
   대기가 완료된 후의 조건 및 필요한 경우 대기
   다시.

   주어진 조건 변수는 단일 조건과만 연관됩니다.
   그러나 하나의 잠금은 여러 개의 잠금과 연관될 수 있습니다.
   조건변수. 즉, 일대다 매핑이 ​​있습니다.
   잠금에서 조건 변수까지.

   This function may sleep, so it must not be called within an
   interrupt handler.  This function may be called with
   interrupts disabled, but interrupts will be turned back on if
   we need to sleep. */
/* LOCK을 atomic하게 release하고 다른 코드가 COND를 signal할 때까지 기다린다.
   COND가 signal되면 리턴하기 전에 LOCK을 다시 acquire한다. 이 함수를 호출하기
   전에 LOCK을 들고 있어야 한다.

   이 함수가 구현하는 monitor는 "Hoare" style이 아니라 "Mesa" style이다. 즉,
   signal을 보내는 것과 받는 것이 atomic operation이 아니다. 따라서 일반적으로
   호출자는 wait가 끝난 뒤 condition을 다시 검사하고, 필요하면 다시 wait해야
   한다.

   주어진 condition variable은 하나의 lock에만 연결되지만, 하나의 lock은 여러
   condition variable에 연결될 수 있다. 즉, lock에서 condition variable로
   one-to-many 맵핑이 있다.

   이 함수는 sleep할 수 있으므로 interrupt handler 안에서 호출하면 안 된다.
   인터럽트가 비활성화된 상태에서 호출할 수는 있지만, sleep해야 하면 인터럽트가
   다시 켜진다. */
void
cond_wait (struct condition *cond, struct lock *lock) {
	struct semaphore_elem waiter;

	ASSERT (cond != NULL);
	ASSERT (lock != NULL);
	ASSERT (!intr_context ());
	ASSERT (lock_held_by_current_thread (lock));

	sema_init (&waiter.semaphore, 0);
	//TODO(schd-2): waiters를 priority 기준으로 정렬되게 추가
	list_push_back (&cond->waiters, &waiter.elem);
	lock_release (lock);
	sema_down (&waiter.semaphore);
	lock_acquire (lock);
}

/* COND(LOCK 로 보호됨)에서 대기 중인 스레드가 있으면
   이 함수는 그 중 하나에게 대기 상태에서 깨어나도록 신호를 보냅니다.
   이 함수를 호출하기 전에 LOCK을 보유해야 합니다.

   An interrupt handler cannot acquire a lock, so it does not
   make sense to try to signal a condition variable within an
   interrupt handler. */
/* LOCK으로 보호되는 COND를 기다리는 스레드가 있다면, 이 함수는 그중 하나에
   signal을 보내 wait에서 깨운다. 이 함수를 호출하기 전에 LOCK을 들고 있어야
   한다.

   interrupt handler는 lock을 acquire할 수 없으므로, interrupt handler 안에서
   condition variable에 signal하려고 하는 것은 의미가 없다. */
void
cond_signal (struct condition *cond, struct lock *lock UNUSED) {
	ASSERT (cond != NULL);
	ASSERT (lock != NULL);
	ASSERT (!intr_context ());
	ASSERT (lock_held_by_current_thread (lock));

	if (!list_empty (&cond->waiters))
		//TODO(schd-2): 정렬되어 있어서 수정 필요 없음
		sema_up (&list_entry (list_pop_front (&cond->waiters),
					struct semaphore_elem, elem)->semaphore);
}

/* COND을(를 통해 보호되는) 대기 중인 모든 스레드를 깨웁니다.
   LOCK). 이 함수를 호출하기 전에 LOCK을 보유해야 합니다.

   An interrupt handler cannot acquire a lock, so it does not
   make sense to try to signal a condition variable within an
   interrupt handler. */
/* LOCK으로 보호되는 COND를 기다리는 모든 스레드를 깨운다. 이 함수를 호출하기
   전에 LOCK을 들고 있어야 한다.

   interrupt handler는 lock을 acquire할 수 없으므로, interrupt handler 안에서
   condition variable에 signal하려고 하는 것은 의미가 없다. */
void
cond_broadcast (struct condition *cond, struct lock *lock) {
	ASSERT (cond != NULL);
	ASSERT (lock != NULL);

	//TODO(schd-2): cond_signal()이 순서 보장한다 가정하므로 수정 없음
	while (!list_empty (&cond->waiters))
		cond_signal (cond, lock);
}
