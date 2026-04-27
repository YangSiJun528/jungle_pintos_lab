#ifndef THREADS_SYNCH_H
#define THREADS_SYNCH_H

#include <list.h>
#include <stdbool.h>

/* A counting semaphore. */
/* counting 세마포어. */
struct semaphore {
	unsigned value;             /* Current value. */
	/* 현재 값. */
	struct list waiters;        /* List of waiting threads. */
	/* 기다리는 스레드들의 리스트. */
};

void sema_init (struct semaphore *, unsigned value);
void sema_down (struct semaphore *);
bool sema_try_down (struct semaphore *);
void sema_up (struct semaphore *);
void sema_self_test (void);

/* 락. */
/* Lock. */
struct lock {
	struct thread *holder;      /* Thread holding lock (for debugging). */
	/* lock을 들고 있는 스레드(디버깅용). */
	struct semaphore semaphore; /* Binary semaphore controlling access. */
	/* 접근을 제어하는 binary 세마포어. */
};

void lock_init (struct lock *);
void lock_acquire (struct lock *);
bool lock_try_acquire (struct lock *);
void lock_release (struct lock *);
bool lock_held_by_current_thread (const struct lock *);

/* Condition variable. */
/* 컨디션 variable. */
struct condition {
	struct list waiters;        /* List of waiting threads. */
	/* 기다리는 스레드들의 리스트. */
};

void cond_init (struct condition *);
void cond_wait (struct condition *, struct lock *);
void cond_signal (struct condition *, struct lock *);
void cond_broadcast (struct condition *, struct lock *);

/* Optimization barrier.
 *
 * The compiler will not reorder operations across an
 * optimization barrier.  See "Optimization Barriers" in the
 * reference guide for more information.*/
/* 최적화 barrier.
 *
 * 컴파일러는 optimization barrier를 가로질러 operation을 reorder하지 않는다.
 * 자세한 내용은 reference guide의 "Optimization Barriers"를 참고한다. */
#define barrier() asm volatile ("" : : : "memory")

#endif /* threads/synch.h */
