#include "threads/thread.h"
#include <debug.h>
#include <stddef.h>
#include <random.h>
#include <stdio.h>
#include <string.h>
#include "threads/flags.h"
#include "threads/interrupt.h"
#include "threads/intr-stubs.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "intrinsic.h"
#ifdef USERPROG
#include "userprog/process.h"
#endif

/* Random value for struct thread's `magic' member.
   Used to detect stack overflow.  See the big comment at the top
   of thread.h for details. */
/* struct thread의 `magic` 멤버에 쓰이는 랜덤 값.
   스택 오버플로우를 감지하는 데 사용한다. 자세한 내용은 thread.h 위쪽의
   큰 주석을 참고한다. */
#define THREAD_MAGIC 0xcd6abf4b

/* Random value for basic thread
   Do not modify this value. */
/* basic thread에 쓰이는 랜덤 값.
   이 값은 수정하지 않는다. */
#define THREAD_BASIC 0xd42df210

/* List of processes in THREAD_READY state, that is, processes
   that are ready to run but not actually running. */
/* THREAD_READY 상태인 프로세스들의 리스트. 즉, 실행 준비는 되었지만 실제로는
   실행 중이 아닌 프로세스들의 리스트이다. */
static struct list ready_list;

/* 특정 타이머 tick까지 잠든 프로세스들의 리스트. */
static struct list sleep_list;

/* Idle thread. */
/* idle 스레드. */
static struct thread *idle_thread;

/* Initial thread, the thread running init.c:main(). */
/* init.c:main()을 실행하는 initial 스레드. */
static struct thread *initial_thread;

/* Lock used by allocate_tid(). */
/* allocate_tid()에서 사용하는 락. */
static struct lock tid_lock;

/* Thread destruction requests */
/* 스레드 파괴 요청들. */
static struct list destruction_req;

/* Statistics. */
/* 통계. */
static long long idle_ticks;    /* # of timer ticks spent idle. */
/* idle 상태로 보낸 타이머 tick 수. */
static long long kernel_ticks;  /* # of timer ticks in kernel threads. */
/* 커널 스레드에서 보낸 타이머 tick 수. */
static long long user_ticks;    /* # of timer ticks in user programs. */
/* 유저 프로그램에서 보낸 타이머 tick 수. */

/* Scheduling. */
/* 스케줄링. */
#define TIME_SLICE 4            /* # of timer ticks to give each thread. */
/* 각 스레드에 줄 타이머 tick 수. */
static unsigned thread_ticks;   /* # of timer ticks since last yield. */
/* 마지막 yield 이후 지난 타이머 tick 수. */

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
/* false(기본값)이면 round-robin 스케줄러를 사용한다.
   true이면 multi-level feedback queue 스케줄러를 사용한다.
   커널 커맨드라인 옵션 "-o mlfqs"로 제어한다. */
bool thread_mlfqs;

static void kernel_thread (thread_func *, void *aux);

static void idle (void *aux UNUSED);
static struct thread *next_thread_to_run (void);
static void init_thread (struct thread *, const char *name, int priority);
static void do_schedule(int status);
static void schedule (void);
static tid_t allocate_tid (void);
static bool cmp_wakeup_ticks (const struct list_elem *a,
		const struct list_elem *b, void *aux UNUSED);

/* Returns true if T appears to point to a valid thread. */
/* T가 유효한 스레드를 가리키는 것처럼 보이면 true를 리턴한다. */
#define is_thread(t) ((t) != NULL && (t)->magic == THREAD_MAGIC)

/* Returns the running thread.
 * Read the CPU's stack pointer `rsp', and then round that
 * down to the start of a page.  Since `struct thread' is
 * always at the beginning of a page and the stack pointer is
 * somewhere in the middle, this locates the curent thread. */
/* 실행 중인 스레드를 리턴한다.
 * CPU의 stack pointer `rsp`를 읽은 뒤 페이지 시작 주소로 내림한다.
 * `struct thread`는 항상 페이지의 시작 부분에 있고 stack pointer는 그 중간
 * 어딘가에 있으므로, 이 방식으로 현재 스레드를 찾을 수 있다. */
#define running_thread() ((struct thread *) (pg_round_down (rrsp ())))


// Global descriptor table for the thread_start.
// thread_start를 위한 Global Descriptor Table.
// Because the gdt will be setup after the thread_init, we should
// gdt는 thread_init 이후에 설정되므로,
// setup temporal gdt first.
// 먼저 임시 gdt를 설정해야 한다.
static uint64_t gdt[3] = { 0, 0x00af9a000000ffff, 0x00cf92000000ffff };

/* 코드를 변환하여 스레딩 시스템을 초기화합니다.
   현재 스레드로 실행 중입니다. 이것은 작동하지 않습니다
   일반적이며 이 경우에만 가능합니다. 왜냐하면 loader.S
   스택의 맨 아래를 페이지 경계에 두도록 주의했습니다.

   또한 실행 큐와 tid 잠금을 초기화합니다.

   이 함수를 호출한 후에는 반드시 페이지를 초기화하세요.
   스레드를 생성하기 전에 할당자
   thread_create().

   It is not safe to call thread_current() until this function
   finishes. */
/* 현재 실행 중인 코드를 스레드로 변환해 스레딩 시스템을 초기화한다. 일반적으로는
   이렇게 할 수 없지만, loader.S가 스택의 바닥을 페이지 경계에 맞춰 두었기
   때문에 이 경우에는 가능하다.

   run queue와 tid 락도 초기화한다.

   이 함수를 호출한 뒤 thread_create()로 스레드를 만들기 전에 반드시 page
   allocator를 초기화해야 한다.

   이 함수가 끝나기 전에는 thread_current()를 호출해도 안전하지 않다. */
void
thread_init (void) {
	ASSERT (intr_get_level () == INTR_OFF);

	/* Reload the temporal gdt for the kernel
	 * This gdt does not include the user context.
	 * The kernel will rebuild the gdt with user context, in gdt_init (). */
	/* 커널을 위한 임시 gdt를 다시 로드한다.
	 * 이 gdt에는 유저 컨텍스트가 포함되어 있지 않다.
	 * 커널은 gdt_init()에서 유저 컨텍스트를 포함해 gdt를 다시 만든다. */
	struct desc_ptr gdt_ds = {
		.size = sizeof (gdt) - 1,
		.address = (uint64_t) gdt
	};
	lgdt (&gdt_ds);

	/* Init the globla thread context */
	/* 전역 스레드 컨텍스트를 초기화한다. */
	lock_init (&tid_lock);
	list_init (&ready_list);
	list_init (&sleep_list);
	list_init (&destruction_req);

	/* Set up a thread structure for the running thread. */
	/* 현재 실행 중인 스레드를 위한 thread 구조체를 설정한다. */
	initial_thread = running_thread ();
	init_thread (initial_thread, "main", PRI_DEFAULT);
	initial_thread->status = THREAD_RUNNING;
	initial_thread->tid = allocate_tid ();
}

/* Starts preemptive thread scheduling by enabling interrupts.
   Also creates the idle thread. */
/* 인터럽트를 활성화해 선점형 스레드 스케줄링을 시작한다.
   idle 스레드도 생성한다. */
void
thread_start (void) {
	/* Create the idle thread. */
	/* idle 스레드를 만든다. */
	struct semaphore idle_started;
	sema_init (&idle_started, 0);
	thread_create ("idle", PRI_MIN, idle, &idle_started);

	/* Start preemptive thread scheduling. */
	/* 선점형 스레드 스케줄링을 시작한다. */
	intr_enable ();

	/* Wait for the idle thread to initialize idle_thread. */
	/* idle 스레드가 idle_thread를 초기화할 때까지 기다린다. */
	sema_down (&idle_started);
}

/* Called by the timer interrupt handler at each timer tick.
   Thus, this function runs in an external interrupt context. */
/* 매 타이머 tick마다 타이머 인터럽트 핸들러가 호출한다.
   따라서 이 함수는 외부 인터럽트 컨텍스트에서 실행된다. */
void
thread_tick (void) {
	struct thread *t = thread_current ();

	/* Update statistics. */
	/* 통계를 업데이트한다. */
	if (t == idle_thread)
		idle_ticks++;
#ifdef USERPROG
	else if (t->pml4 != NULL)
		user_ticks++;
#endif
	else
		kernel_ticks++;

	/* Enforce preemption. */
	/* 선점을 강제한다. */
	if (++thread_ticks >= TIME_SLICE)
		intr_yield_on_return ();
}

/* Prints thread statistics. */
/* 스레드 통계를 출력한다. */
void
thread_print_stats (void) {
	printf ("Thread: %lld idle ticks, %lld kernel ticks, %lld user ticks\n",
			idle_ticks, kernel_ticks, user_ticks);
}

/* 주어진 초기값을 사용하여 NAME이라는 새 커널 스레드를 생성합니다.
   AUX를 인수로 전달하여 FUNCTION을 실행하는 PRIORITY,
   준비 대기열에 추가합니다. 스레드 식별자를 반환합니다.
   새 스레드의 경우, 생성이 실패하면 TID_ERROR입니다.

   thread_start()이 호출된 경우 새 스레드는 다음과 같을 수 있습니다.
   thread_create()이 반환되기 전에 예약되었습니다. 빠져나갈 수도 있다
   thread_create()이 반환되기 전에. 그에 비해 원작은
   스레드는 새 스레드가 생성되기 전까지 얼마 동안 실행될 수 있습니다.
   예정. 세마포어 또는 다른 형태의 사용
   주문을 보장해야 하는 경우 동기화.

   The code provided sets the new thread's `priority' member to
   PRIORITY, but no actual priority scheduling is implemented.
   Priority scheduling is the goal of Problem 1-3. */
/* 주어진 초기 PRIORITY로 NAME이라는 새 커널 스레드를 만들고, FUNCTION을 AUX
   인자로 실행하도록 한 뒤 ready queue에 추가한다. 성공하면 새 스레드의 thread
   identifier를 리턴하고, 실패하면 TID_ERROR를 리턴한다.

   thread_start()가 호출된 뒤라면 새 스레드는 thread_create()가 리턴하기 전에
   스케줄될 수 있다. 심지어 thread_create()가 리턴하기 전에 종료될 수도 있다.
   반대로, 원래 스레드가 새 스레드가 스케줄되기 전에 얼마든지 더 실행될 수도
   있다. 순서를 보장해야 한다면 세마포어 같은 동기화 수단을 사용한다.

   제공된 코드는 새 스레드의 `priority` 멤버를 PRIORITY로 설정하지만, 실제
   priority scheduling은 구현되어 있지 않다. Priority scheduling은 Problem
   1-3의 목표이다. */
tid_t
thread_create (const char *name, int priority,
		thread_func *function, void *aux) {
	struct thread *t;
	tid_t tid;

	ASSERT (function != NULL);

	/* Allocate thread. */
	/* 스레드를 할당한다. */
	t = palloc_get_page (PAL_ZERO);
	if (t == NULL)
		return TID_ERROR;

	/* Initialize thread. */
	/* 스레드를 초기화한다. */
	init_thread (t, name, priority);
	tid = t->tid = allocate_tid ();

	/* Call the kernel_thread if it scheduled.
	 * Note) rdi is 1st argument, and rsi is 2nd argument. */
	/* 스케줄되면 kernel_thread를 호출한다.
	 * Note) rdi는 첫 번째 인자이고, rsi는 두 번째 인자이다. */
	t->tf.rip = (uintptr_t) kernel_thread;
	t->tf.R.rdi = (uint64_t) function;
	t->tf.R.rsi = (uint64_t) aux;
	t->tf.ds = SEL_KDSEG;
	t->tf.es = SEL_KDSEG;
	t->tf.ss = SEL_KDSEG;
	t->tf.cs = SEL_KCSEG;
	t->tf.eflags = FLAG_IF;

	/* Add to run queue. */
	/* run queue에 추가한다. */
	thread_unblock (t);
	thread_yield_if_needed();

	return tid;
}

/* 현재 스레드를 절전 모드로 전환합니다. 예정되어 있지 않습니다
   thread_unblock()에 의해 깨어날 때까지 다시.

   This function must be called with interrupts turned off.  It
   is usually a better idea to use one of the synchronization
   primitives in synch.h. */
/* 현재 스레드를 sleep 상태로 둔다. thread_unblock()으로 깨우기 전까지 다시
   스케줄되지 않는다.

   이 함수는 인터럽트가 꺼진 상태에서 호출해야 한다. 보통은 synch.h의 동기화
   primitive 중 하나를 사용하는 편이 더 좋다. */
void
thread_block (void) {
	ASSERT (!intr_context ());
	ASSERT (intr_get_level () == INTR_OFF);
	thread_current ()->status = THREAD_BLOCKED;
	schedule ();
}

/* 차단된 스레드 T를 실행 준비 상태로 전환합니다.
   T가 차단되지 않은 경우 이는 오류입니다. (thread_yield()을(를) 사용하여
   실행 중인 스레드를 준비합니다.)

   This function does not preempt the running thread.  This can
   be important: if the caller had disabled interrupts itself,
   it may expect that it can atomically unblock a thread and
   update other data. */
/* blocked 상태의 스레드 T를 ready-to-run 상태로 전환한다. T가 blocked 상태가
   아니면 에러이다. 실행 중인 스레드를 ready 상태로 만들려면 thread_yield()를
   사용한다.

   이 함수는 실행 중인 스레드를 preempt하지 않는다. 이는 중요할 수 있다.
   호출자가 직접 인터럽트를 비활성화했다면, 스레드를 unblock하고 다른 데이터를
   업데이트하는 작업이 atomic하게 수행되기를 기대할 수 있기 때문이다. */
void
thread_unblock (struct thread *t) {
	enum intr_level old_level;

	ASSERT (is_thread (t));

	old_level = intr_disable ();
	ASSERT (t->status == THREAD_BLOCKED);
	list_insert_ordered (&ready_list, &t->elem, cmp_priority, NULL);
	t->status = THREAD_READY;
	intr_set_level (old_level);
}

/* Returns the name of the running thread. */
/* 실행 중인 스레드의 이름을 리턴한다. */
const char *
thread_name (void) {
	return thread_current ()->name;
}

/* Returns the running thread.
   This is running_thread() plus a couple of sanity checks.
   See the big comment at the top of thread.h for details. */
/* 실행 중인 스레드를 리턴한다.
   running_thread()에 몇 가지 sanity check를 더한 것이다.
   자세한 내용은 thread.h 위쪽의 큰 주석을 참고한다. */
struct thread *
thread_current (void) {
	struct thread *t = running_thread ();

	/* Make sure T is really a thread.
	   If either of these assertions fire, then your thread may
	   have overflowed its stack.  Each thread has less than 4 kB
	   of stack, so a few big automatic arrays or moderate
	   recursion can cause stack overflow. */
	/* T가 실제로 스레드인지 확인한다.
	   이 assertion 중 하나라도 실패하면 스레드가 스택을 overflow했을 수 있다.
	   각 스레드는 4 kB보다 작은 스택을 가지므로, 큰 automatic array 몇 개나
	   적당한 수준의 recursion만으로도 스택 오버플로우가 발생할 수 있다. */
	ASSERT (is_thread (t));
	ASSERT (t->status == THREAD_RUNNING);

	return t;
}

/* Returns the running thread's tid. */
/* 실행 중인 스레드의 tid를 리턴한다. */
tid_t
thread_tid (void) {
	return thread_current ()->tid;
}

/* Deschedules the current thread and destroys it.  Never
   returns to the caller. */
/* 현재 스레드를 deschedule하고 파괴한다. 호출자에게 절대 리턴하지 않는다. */
void
thread_exit (void) {
	ASSERT (!intr_context ());

#ifdef USERPROG
	process_exit ();
#endif

	/* Just set our status to dying and schedule another process.
	   We will be destroyed during the call to schedule_tail(). */
	/* 상태를 dying으로 설정하고 다른 프로세스를 스케줄한다.
	   schedule_tail() 호출 중에 현재 스레드는 파괴된다. */
	intr_disable ();
	do_schedule (THREAD_DYING);
	NOT_REACHED ();
}

/* Yields the CPU.  The current thread is not put to sleep and
   may be scheduled again immediately at the scheduler's whim. */
/* CPU를 yield한다. 현재 스레드는 sleep 상태가 되지 않으며, 스케줄러 판단에 따라
   즉시 다시 스케줄될 수도 있다. */
void
thread_yield (void) {
	struct thread *curr = thread_current ();
	enum intr_level old_level;

	ASSERT (!intr_context ());

	old_level = intr_disable ();
	if (curr != idle_thread)
		list_insert_ordered (&ready_list, &curr->elem, cmp_priority, NULL);
	do_schedule (THREAD_READY);
	intr_set_level (old_level);
}

/* 만약 ready_list 중 현재 스레드보다 우선순위가 높은게 있으면 preemption(yield 호출) 한다. */
void
thread_yield_if_needed (void) {
	if (list_empty (&ready_list))
		return;

	struct thread *peek_t =
		list_entry (list_front (&ready_list), struct thread, elem);
	bool need_preemption = peek_t->priority > thread_current ()->priority;

	if (need_preemption) {
		if (intr_context ())
			intr_yield_on_return ();
		else
			thread_yield ();
	}
}

/* Puts the current thread to sleep until WAKEUP_TICK. */
/* 현재 스레드를 WAKEUP_TICK까지 재우고 스케줄 대상에서 제외한다. */
// ASSERT: 인터럽트 핸들러에 의해서 호출될 수 없다.
void
thread_sleep (int64_t wakeup_tick) {
	struct thread *curr = thread_current ();
	enum intr_level old_level;

	ASSERT (!intr_context ());

	old_level = intr_disable ();
	if (curr != idle_thread) {
		curr->wakeup_ticks = wakeup_tick;
		list_insert_ordered (&sleep_list, &curr->elem, cmp_wakeup_ticks, NULL);
		thread_block ();
	}
	intr_set_level (old_level);
}

/* Wakes every sleeping thread whose wakeup tick has arrived. */
/* 깨어날 tick에 도달한 모든 sleeping thread를 ready 상태로 옮긴다. */
// ASSERT: 인터럽트 핸들러에 의해서만 호출되어야 한다.
void
threads_wakeup (int64_t ticks) {
	struct list_elem *e;

	ASSERT (intr_context ());
	ASSERT (intr_get_level () == INTR_OFF);

	e = list_begin (&sleep_list);
	while (e != list_end (&sleep_list)) {
		struct thread *t = list_entry (e, struct thread, elem);

		if (t->wakeup_ticks <= ticks) {
			e = list_remove (e);
			thread_unblock (t);
		} else {
			break;
		}
	}
	thread_yield_if_needed();
}

/* Sets the current thread's priority to NEW_PRIORITY. */
/* 현재 스레드의 priority를 NEW_PRIORITY로 설정한다. */
void
thread_set_priority (int new_priority) {
	thread_current ()->priority = new_priority;
	thread_yield_if_needed();
}

/* Returns the current thread's priority. */
/* 현재 스레드의 priority를 리턴한다. */
int
thread_get_priority (void) {
	return thread_current ()->priority;
}

/* Sets the current thread's nice value to NICE. */
/* 현재 스레드의 nice 값을 NICE로 설정한다. */
void
thread_set_nice (int nice UNUSED) {
	/* TODO: Your implementation goes here */
	/* TODO: 여기에 구현을 작성한다. */
}

/* Returns the current thread's nice value. */
/* 현재 스레드의 nice 값을 리턴한다. */
int
thread_get_nice (void) {
	/* TODO: Your implementation goes here */
	/* TODO: 여기에 구현을 작성한다. */
	return 0;
}

/* Returns 100 times the system load average. */
/* 시스템 load average의 100배를 리턴한다. */
int
thread_get_load_avg (void) {
	/* TODO: Your implementation goes here */
	/* TODO: 여기에 구현을 작성한다. */
	return 0;
}

/* Returns 100 times the current thread's recent_cpu value. */
/* 현재 스레드 recent_cpu 값의 100배를 리턴한다. */
int
thread_get_recent_cpu (void) {
	/* TODO: Your implementation goes here */
	/* TODO: 여기에 구현을 작성한다. */
	return 0;
}

/* 유휴 스레드. 실행할 준비가 된 다른 스레드가 없을 때 실행됩니다.

   The idle thread is initially put on the ready list by
   thread_start().  It will be scheduled once initially, at which
   point it initializes idle_thread, "up"s the semaphore passed
   to it to enable thread_start() to continue, and immediately
   blocks.  After that, the idle thread never appears in the
   ready list.  It is returned by next_thread_to_run() as a
   special case when the ready list is empty. */
/* 실행할 준비가 된 다른 스레드가 없을 때 idle 스레드가 실행된다.

   idle 스레드는 처음에 thread_start()에 의해 ready list에 들어간다. 처음 한 번
   스케줄되면 idle_thread를 초기화하고, 전달받은 세마포어를 "up"해서
   thread_start()가 계속 진행할 수 있게 한 뒤 즉시 block된다. 그 후 idle
   스레드는 ready list에 나타나지 않는다. ready list가 비었을 때
   next_thread_to_run()이 special case로 이 스레드를 리턴한다. */
static void
idle (void *idle_started_ UNUSED) {
	struct semaphore *idle_started = idle_started_;

	idle_thread = thread_current ();
	sema_up (idle_started);

	for (;;) {
		/* Let someone else run. */
		/* 다른 스레드가 실행되게 한다. */
		intr_disable ();
		thread_block ();

		/* 인터럽트를 다시 활성화하고 다음 인터럽트를 기다립니다.

		   `sti' 명령어는 다음이 나올 때까지 인터럽트를 비활성화합니다.
		   다음 명령이 완료되었으므로 이 두 명령은
		   명령어는 원자적으로 실행됩니다. 이 원자성은
		   important; otherwise, an interrupt could be handled
		   인터럽트를 다시 활성화하고 다음을 기다리는 사이
		   하나가 발생하면 1시계 틱만큼 낭비됩니다.
		   시간.

		   See [IA32-v2a] "HLT", [IA32-v2b] "STI", and [IA32-v3a]
		   7.11.1 "HLT Instruction". */
		/* 인터럽트를 다시 활성화하고 다음 인터럽트를 기다린다.

		   `sti` instruction은 다음 instruction이 완료될 때까지 인터럽트를
		   비활성화하므로, 이 두 instruction은 atomic하게 실행된다. 이 atomicity는
		   중요하다. 그렇지 않으면 인터럽트를 다시 활성화한 뒤 다음 인터럽트를
		   기다리기 전에 인터럽트가 처리되어 clock tick 하나만큼의 시간이 낭비될 수
		   있다.

		   [IA32-v2a] "HLT", [IA32-v2b] "STI", [IA32-v3a] 7.11.1
		   "HLT Instruction"을 참고한다. */
		asm volatile ("sti; hlt" : : : "memory");
	}
}

/* Function used as the basis for a kernel thread. */
/* 커널 스레드의 기반으로 사용되는 함수. */
static void
kernel_thread (thread_func *function, void *aux) {
	ASSERT (function != NULL);

	intr_enable ();       /* The scheduler runs with interrupts off. */
	/* 스케줄러는 인터럽트가 꺼진 상태로 실행된다. */
	function (aux);       /* Execute the thread function. */
	/* 스레드 함수를 실행한다. */
	thread_exit ();       /* If function() returns, kill the thread. */
	/* function()이 리턴하면 스레드를 종료한다. */
}


/* T의 기본 초기화를 차단된 스레드로 수행합니다.
   NAME. */
/* T를 NAME이라는 이름의 blocked 스레드로 기본 초기화한다. */
static void
init_thread (struct thread *t, const char *name, int priority) {
	ASSERT (t != NULL);
	ASSERT (PRI_MIN <= priority && priority <= PRI_MAX);
	ASSERT (name != NULL);

	memset (t, 0, sizeof *t);
	t->status = THREAD_BLOCKED;
	strlcpy (t->name, name, sizeof t->name);
	t->tf.rsp = (uint64_t) t + PGSIZE - sizeof (void *);
	t->priority = priority;
	t->magic = THREAD_MAGIC;
}

/* 예약할 다음 스레드를 선택하고 반환합니다. 해야 한다
   실행 큐가 그렇지 않은 경우 실행 큐에서 스레드를 반환합니다.
   비어 있는. (실행 중인 스레드가 계속 실행될 수 있으면
   실행 큐에 있습니다.) 실행 큐가 비어 있으면 반환
   idle_thread. */
/* 다음에 스케줄할 스레드를 선택해 리턴한다. run queue가 비어 있지 않다면 그
   안에서 스레드를 리턴해야 한다. 실행 중인 스레드가 계속 실행될 수 있다면 그
   스레드는 run queue 안에 있다. run queue가 비어 있으면 idle_thread를
   리턴한다. */
static struct thread *
next_thread_to_run (void) {
	if (list_empty (&ready_list))
		return idle_thread;
	else
		return list_entry (list_pop_front (&ready_list), struct thread, elem);
}

/* Use iretq to launch the thread */
/* iretq를 사용해 스레드를 실행한다. */
void
do_iret (struct intr_frame *tf) {
	__asm __volatile(
			"movq %0, %%rsp\n"
			"movq 0(%%rsp),%%r15\n"
			"movq 8(%%rsp),%%r14\n"
			"movq 16(%%rsp),%%r13\n"
			"movq 24(%%rsp),%%r12\n"
			"movq 32(%%rsp),%%r11\n"
			"movq 40(%%rsp),%%r10\n"
			"movq 48(%%rsp),%%r9\n"
			"movq 56(%%rsp),%%r8\n"
			"movq 64(%%rsp),%%rsi\n"
			"movq 72(%%rsp),%%rdi\n"
			"movq 80(%%rsp),%%rbp\n"
			"movq 88(%%rsp),%%rdx\n"
			"movq 96(%%rsp),%%rcx\n"
			"movq 104(%%rsp),%%rbx\n"
			"movq 112(%%rsp),%%rax\n"
			"addq $120,%%rsp\n"
			"movw 8(%%rsp),%%ds\n"
			"movw (%%rsp),%%es\n"
			"addq $32, %%rsp\n"
			"iretq"
			: : "g" ((uint64_t) tf) : "memory");
}

/* 새 스레드의 페이지를 활성화하여 스레드 전환
   테이블을 삭제하고, 이전 스레드가 죽어가고 있으면 이를 삭제합니다.

   이 함수를 호출하면 방금 스레드에서 전환했습니다.
   PREV, 새 스레드가 이미 실행 중이고 인터럽트가 발생했습니다.
   여전히 비활성화되어 있습니다.

   It's not safe to call printf() until the thread switch is
   complete.  In practice that means that printf()s should be
   added at the end of the function. */
/* 새 스레드의 페이지 테이블을 activate해서 스레드를 전환하고, 이전 스레드가
   dying 상태라면 파괴한다.

   이 함수가 호출될 때는 방금 PREV 스레드에서 전환된 상태이며, 새 스레드는 이미
   실행 중이고 인터럽트는 여전히 비활성화되어 있다.

   스레드 전환이 끝나기 전에는 printf()를 호출해도 안전하지 않다. 실제로는
   printf()를 함수 끝부분에 추가해야 한다는 뜻이다. */
static void
thread_launch (struct thread *th) {
	uint64_t tf_cur = (uint64_t) &running_thread ()->tf;
	uint64_t tf = (uint64_t) &th->tf;
	ASSERT (intr_get_level () == INTR_OFF);

	/* The main switching logic.
	 * We first restore the whole execution context into the intr_frame
	 * and then switching to the next thread by calling do_iret.
	 * Note that, we SHOULD NOT use any stack from here
	 * until switching is done. */
	/* 핵심 switching 로직.
	 * 먼저 전체 실행 컨텍스트를 intr_frame에 복원하고, do_iret를 호출해 다음
	 * 스레드로 전환한다.
	 * 주의: 여기부터 switching이 끝날 때까지 어떤 스택도 사용하면 안 된다. */
	__asm __volatile (
			/* Store registers that will be used. */
			/* 사용할 레지스터를 저장한다. */
			"push %%rax\n"
			"push %%rbx\n"
			"push %%rcx\n"
			/* Fetch input once */
			/* 입력을 한 번 가져온다. */
			"movq %0, %%rax\n"
			"movq %1, %%rcx\n"
			"movq %%r15, 0(%%rax)\n"
			"movq %%r14, 8(%%rax)\n"
			"movq %%r13, 16(%%rax)\n"
			"movq %%r12, 24(%%rax)\n"
			"movq %%r11, 32(%%rax)\n"
			"movq %%r10, 40(%%rax)\n"
			"movq %%r9, 48(%%rax)\n"
			"movq %%r8, 56(%%rax)\n"
			"movq %%rsi, 64(%%rax)\n"
			"movq %%rdi, 72(%%rax)\n"
			"movq %%rbp, 80(%%rax)\n"
			"movq %%rdx, 88(%%rax)\n"
			"pop %%rbx\n"              // rcx를 저장했습니다.
			"movq %%rbx, 96(%%rax)\n"
			"pop %%rbx\n"              // 저장된 rbx
			"movq %%rbx, 104(%%rax)\n"
			"pop %%rbx\n"              // 저장된 랙스
			"movq %%rbx, 112(%%rax)\n"
			"addq $120, %%rax\n"
			"movw %%es, (%%rax)\n"
			"movw %%ds, 8(%%rax)\n"
			"addq $32, %%rax\n"
			"call __next\n"         // 현재 찢어짐을 읽으십시오.
			"__next:\n"
			"pop %%rbx\n"
			"addq $(out_iret -  __next), %%rbx\n"
			"movq %%rbx, 0(%%rax)\n" // 찢다
			"movw %%cs, 8(%%rax)\n"  // CS
			"pushfq\n"
			"popq %%rbx\n"
			"mov %%rbx, 16(%%rax)\n" // 깃발
			"mov %%rsp, 24(%%rax)\n" // RSP
			"movw %%ss, 32(%%rax)\n"
			"mov %%rcx, %%rdi\n"
			"call do_iret\n"
			"out_iret:\n"
			: : "g"(tf_cur), "g" (tf) : "memory"
			);
}

/* Schedules a new process. At entry, interrupts must be off.
 * This function modify current thread's status to status and then
 * finds another thread to run and switches to it.
 * It's not safe to call printf() in the schedule(). */
/* 새 프로세스를 스케줄한다. 진입 시점에는 인터럽트가 꺼져 있어야 한다.
 * 이 함수는 현재 스레드의 status를 status로 바꾼 뒤 실행할 다른 스레드를 찾아
 * 그 스레드로 전환한다.
 * schedule() 안에서 printf()를 호출하는 것은 안전하지 않다. */
static void
do_schedule(int status) {
	ASSERT (intr_get_level () == INTR_OFF);
	ASSERT (thread_current()->status == THREAD_RUNNING);
	while (!list_empty (&destruction_req)) {
		struct thread *victim =
			list_entry (list_pop_front (&destruction_req), struct thread, elem);
		palloc_free_page(victim);
	}
	thread_current ()->status = status;
	schedule ();
}

static void
schedule (void) {
	struct thread *curr = running_thread ();
	struct thread *next = next_thread_to_run ();

	ASSERT (intr_get_level () == INTR_OFF);
	ASSERT (curr->status != THREAD_RUNNING);
	ASSERT (is_thread (next));
	/* Mark us as running. */
	/* 우리를 running 상태로 표시한다. */
	next->status = THREAD_RUNNING;

	/* Start new time slice. */
	/* 새 time slice를 시작한다. */
	thread_ticks = 0;

#ifdef USERPROG
	/* Activate the new address space. */
	/* 새 주소 공간을 activate한다. */
	process_activate (next);
#endif

	if (curr != next) {
		/* 전환한 스레드가 죽어가고 있으면 해당 스레드를 파괴하세요.
		   실. thread_exit()이(가) 발생하지 않도록 이 작업이 늦게 발생해야 합니다.
		   깔개를 꺼내십시오.
		   페이지가 다음과 같기 때문에 여기에 페이지 무료 요청을 대기열에 추가합니다.
		   현재 스택에서 사용됩니다.
		   실제 파괴 로직은 시작 부분에서 호출됩니다.
		   schedule(). */
		/* 전환되기 전의 스레드가 dying 상태라면 그 struct thread를 파괴한다.
		   thread_exit()이 자기 자신이 기대고 있는 스택을 너무 일찍 없애지 않게
		   하려면 이 작업은 늦게 수행되어야 한다.
		   현재 페이지가 스택으로 사용 중이므로 여기서는 page free request만
		   큐에 넣는다.
		   실제 파괴 로직은 schedule() 시작 부분에서 호출된다. */
		if (curr && curr->status == THREAD_DYING && curr != initial_thread) {
			ASSERT (curr != next);
			list_push_back (&destruction_req, &curr->elem);
		}

		/* Before switching the thread, we first save the information
		 * of current running. */
		/* 스레드를 전환하기 전에 현재 실행 상태 정보를 먼저 저장한다. */
		thread_launch (next);
	}
}

/* Returns a tid to use for a new thread. */
/* 새 스레드에 사용할 tid를 리턴한다. */
static tid_t
allocate_tid (void) {
	static tid_t next_tid = 1;
	tid_t tid;

	lock_acquire (&tid_lock);
	tid = next_tid++;
	lock_release (&tid_lock);

	return tid;
}

static bool
cmp_wakeup_ticks (const struct list_elem *a, const struct list_elem *b,
		void *aux UNUSED) {
	const struct thread *ta = list_entry (a, struct thread, elem);
	const struct thread *tb = list_entry (b, struct thread, elem);

	if (ta->wakeup_ticks == tb->wakeup_ticks)
		return ta->priority > tb->priority;

	return ta->wakeup_ticks < tb->wakeup_ticks;
}

// 햇갈릴 수 있는데, 큰게 앞에 위치하도록 조건을 명세의 반대로 한다.
bool
cmp_priority(const struct list_elem* a, const struct list_elem* b,
		void *aux UNUSED) {
	const struct thread *ta = list_entry (a, struct thread, elem);
	const struct thread *tb = list_entry (b, struct thread, elem);

	//같은 priority는 false를 반환해서 기존 항목 뒤에 간다.
	return ta->priority > tb->priority;
}
