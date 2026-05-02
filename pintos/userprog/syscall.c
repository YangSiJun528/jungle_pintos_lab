#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"
#include "threads/synch.h"

#define NO_RETURN_VAL (-1)

struct syscall_entry {
	/* system call number */
	uint64_t syscall_num;

	/* return value (optional, default: NO_RETURN_VAL) */
	/* 반환값이 필요한 경우 handle_{syscall_name} 함수에서
	   설정함 */
	uint64_t return_value;

	/* arguments */
	/* Linux x86-64 system call ABI에선 인자를 6개로 제한함 */
	uint64_t args[6];
};

void syscall_entry (void);
void syscall_handler (struct intr_frame *);
static void init_syscall_entry (struct intr_frame *, struct syscall_entry *);
static void dispatch_syscall (struct intr_frame *, struct syscall_entry *);
static void handle_halt (struct intr_frame *, struct syscall_entry *);
static void handle_exit (struct intr_frame *, struct syscall_entry *);
static void handle_fork (struct intr_frame *, struct syscall_entry *);
static void handle_exec (struct intr_frame *, struct syscall_entry *);
static void handle_wait (struct intr_frame *, struct syscall_entry *);
static void handle_create (struct intr_frame *, struct syscall_entry *);
static void handle_remove (struct intr_frame *, struct syscall_entry *);
static void handle_open (struct intr_frame *, struct syscall_entry *);
static void handle_filesize (struct intr_frame *, struct syscall_entry *);
static void handle_read (struct intr_frame *, struct syscall_entry *);
static void handle_write (struct intr_frame *, struct syscall_entry *);
static void handle_seek (struct intr_frame *, struct syscall_entry *);
static void handle_tell (struct intr_frame *, struct syscall_entry *);
static void handle_close (struct intr_frame *, struct syscall_entry *);

/* System call.
 *
 * Previously system call services was handled by the interrupt handler
 * (e.g. int 0x80 in linux). However, in x86-64, the manufacturer supplies
 * efficient path for requesting the system call, the `syscall` instruction.
 *
 * The syscall instruction works by reading the values from the the Model
 * Specific Register (MSR). For the details, see the manual. */
/* System call 설명.
 *
 * 예전에는 system call 서비스가 interrupt handler에서 처리되었다
 * (예: linux의 int 0x80). 하지만 x86-64에서는 제조사가 system call을 요청하기
 * 위한 효율적인 경로인 `syscall` instruction을 제공한다.
 *
 * syscall instruction은 Model Specific Register(MSR)의 값을 읽어 동작한다.
 * 자세한 내용은 매뉴얼을 참고한다. */

#define MSR_STAR 0xc0000081         /* Segment selector msr */
/* 세그먼트 selector msr. */
#define MSR_LSTAR 0xc0000082        /* Long mode SYSCALL target */
/* long mode SYSCALL 대상. */
#define MSR_SYSCALL_MASK 0xc0000084 /* Mask for the eflags */
/* eflags를 위한 mask. */

void
syscall_init (void) {
	write_msr(MSR_STAR, ((uint64_t)SEL_UCSEG - 0x10) << 48  |
			((uint64_t)SEL_KCSEG) << 32);
	write_msr(MSR_LSTAR, (uint64_t) syscall_entry);

	/* The interrupt service rountine should not serve any interrupts
	 * until the syscall_entry swaps the userland stack to the kernel
	 * mode stack. Therefore, we masked the FLAG_FL. */
	/* syscall_entry가 유저랜드 스택을 커널 모드 스택으로 바꾸기 전까지
	 * interrupt service routine은 어떤 인터럽트도 처리하면 안 된다.
	 * 따라서 FLAG_FL을 mask한다. */
	write_msr(MSR_SYSCALL_MASK,
			FLAG_IF | FLAG_TF | FLAG_DF | FLAG_IOPL | FLAG_AC | FLAG_NT);
}

/* The main system call interface */
/* 메인 system call 인터페이스. */
void
syscall_handler (struct intr_frame *f) {
	struct syscall_entry entry;

	init_syscall_entry (f, &entry);
	dispatch_syscall (f, &entry);

	if (entry.return_value != NO_RETURN_VAL) {
		f->R.rax = entry.return_value;
	}
}

static void
init_syscall_entry (struct intr_frame *f, struct syscall_entry *entry) {
	entry->syscall_num = f->R.rax;
	entry->return_value = NO_RETURN_VAL;
	entry->args[0] = f->R.rdi;
	entry->args[1] = f->R.rsi;
	entry->args[2] = f->R.rdx;
	entry->args[3] = f->R.r10;
	entry->args[4] = f->R.r8;
	entry->args[5] = f->R.r9;
}

static void
dispatch_syscall (struct intr_frame *f, struct syscall_entry *entry) {
	switch (entry->syscall_num) {
		case SYS_HALT:
			handle_halt (f, entry);
			break;
		case SYS_EXIT:
			handle_exit (f, entry);
			break;
		case SYS_FORK:
			handle_fork (f, entry);
			break;
		case SYS_EXEC:
			handle_exec (f, entry);
			break;
		case SYS_WAIT:
			handle_wait (f, entry);
			break;
		case SYS_CREATE:
			handle_create (f, entry);
			break;
		case SYS_REMOVE:
			handle_remove (f, entry);
			break;
		case SYS_OPEN:
			handle_open (f, entry);
			break;
		case SYS_FILESIZE:
			handle_filesize (f, entry);
			break;
		case SYS_READ:
			handle_read (f, entry);
			break;
		case SYS_WRITE:
			handle_write (f, entry);
			break;
		case SYS_SEEK:
			handle_seek (f, entry);
			break;
		case SYS_TELL:
			handle_tell (f, entry);
			break;
		case SYS_CLOSE:
			handle_close (f, entry);
			break;
		default:
			ASSERT (false); /* 현재 처리할 수 없는 syscall */
	}
}

/* TODO: 구현하면 UNUSED, ASSERT 빼기 */
static void
handle_halt (struct intr_frame *f UNUSED, struct syscall_entry *entry UNUSED) {
	barrier ();
	ASSERT (false); /* 현재 처리할 수 없는 syscall */
}

static void
handle_exit (struct intr_frame *f UNUSED, struct syscall_entry *entry UNUSED) {
	barrier ();
	ASSERT (false); /* 현재 처리할 수 없는 syscall */
}

/* TODO: 구현하면 UNUSED, ASSERT 빼기 */
static void
handle_fork (struct intr_frame *f UNUSED, struct syscall_entry *entry UNUSED) {
	barrier ();
	ASSERT (false); /* 현재 처리할 수 없는 syscall */
}

/* TODO: 구현하면 UNUSED, ASSERT 빼기 */
static void
handle_exec (struct intr_frame *f UNUSED, struct syscall_entry *entry UNUSED) {
	barrier ();
	ASSERT (false); /* 현재 처리할 수 없는 syscall */
}

/* TODO: 구현하면 UNUSED, ASSERT 빼기 */
static void
handle_wait (struct intr_frame *f UNUSED, struct syscall_entry *entry UNUSED) {
	barrier ();
	ASSERT (false); /* 현재 처리할 수 없는 syscall */
}

/* TODO: 구현하면 UNUSED, ASSERT 빼기 */
static void
handle_create (struct intr_frame *f UNUSED,
		struct syscall_entry *entry UNUSED) {
	barrier ();
	ASSERT (false); /* 현재 처리할 수 없는 syscall */
}

/* TODO: 구현하면 UNUSED, ASSERT 빼기 */
static void
handle_remove (struct intr_frame *f UNUSED,
		struct syscall_entry *entry UNUSED) {
	barrier ();
	ASSERT (false); /* 현재 처리할 수 없는 syscall */
}

/* TODO: 구현하면 UNUSED, ASSERT 빼기 */
static void
handle_open (struct intr_frame *f UNUSED, struct syscall_entry *entry UNUSED) {
	barrier ();
	ASSERT (false); /* 현재 처리할 수 없는 syscall */
}

/* TODO: 구현하면 UNUSED, ASSERT 빼기 */
static void
handle_filesize (struct intr_frame *f UNUSED,
		struct syscall_entry *entry UNUSED) {
	barrier ();
	ASSERT (false); /* 현재 처리할 수 없는 syscall */
}

/* TODO: 구현하면 UNUSED, ASSERT 빼기 */
static void
handle_read (struct intr_frame *f UNUSED, struct syscall_entry *entry UNUSED) {
	barrier ();
	ASSERT (false); /* 현재 처리할 수 없는 syscall */
}

/* TODO: 구현하면 UNUSED, ASSERT 빼기 */
static void
handle_write (struct intr_frame *f UNUSED, struct syscall_entry *entry UNUSED) {
	barrier ();
	ASSERT (false); /* 현재 처리할 수 없는 syscall */
}

/* TODO: 구현하면 UNUSED, ASSERT 빼기 */
static void
handle_seek (struct intr_frame *f UNUSED, struct syscall_entry *entry UNUSED) {
	barrier ();
	ASSERT (false); /* 현재 처리할 수 없는 syscall */
}

/* TODO: 구현하면 UNUSED, ASSERT 빼기 */
static void
handle_tell (struct intr_frame *f UNUSED, struct syscall_entry *entry UNUSED) {
	barrier ();
	ASSERT (false); /* 현재 처리할 수 없는 syscall */
}

/* TODO: 구현하면 UNUSED, ASSERT 빼기 */
static void
handle_close (struct intr_frame *f UNUSED, struct syscall_entry *entry UNUSED) {
	barrier ();
	ASSERT (false); /* 현재 처리할 수 없는 syscall */
}
