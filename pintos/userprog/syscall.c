#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"
#include "devices/input.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "threads/mmu.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/init.h"
#include "userprog/process.h"

typedef tid_t pid_t;

struct syscall_entry {
	/* system call number */
	uint64_t syscall_num;

	/* (default: false)
	  만약 true인 경우, return_value를 시스템 콜 반환값으로 설정한다.
	  각 핸들러(handle_{syscall_name})에서 리턴이 필요한 경우,
	  이 값을 true로 설정해야 한다. */
	bool should_return_value;

	/* return value (optional) */
	/* 반환값이 필요한 경우 핸들러(handle_{syscall_name})에서 설정함 */
	int64_t return_value;

	/* arguments */
	/* Linux x86-64 system call ABI에선 인자를 6개로 제한함 */
	uint64_t args[6];
};

void syscall_entry (void);
void syscall_handler (struct intr_frame *);
static void init_syscall_entry (struct intr_frame *, struct syscall_entry *);
static void dispatch_syscall (struct intr_frame *, struct syscall_entry *);
static void handle_halt (void);
static void handle_exit (struct syscall_entry *);
static void handle_fork (struct intr_frame *, struct syscall_entry *);
static void handle_exec (struct syscall_entry *);
static void handle_wait (struct syscall_entry *);
static void handle_create (struct syscall_entry *);
static void handle_remove (struct syscall_entry *);
static void handle_open (struct syscall_entry *);
static void handle_filesize (struct syscall_entry *);
static void handle_read (struct syscall_entry *);
static void handle_write (struct syscall_entry *);
static void handle_seek (struct syscall_entry *);
static void handle_tell (struct syscall_entry *);
static void handle_close (struct syscall_entry *);
static void _exit (int status);
static void *get_next_page_if_valid (void *);
static bool is_valid_user_buffer (void *, size_t);
static bool is_valid_user_string (char *);

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

	if (entry.should_return_value) {
		f->R.rax = entry.return_value;
	}
}

static void
init_syscall_entry (struct intr_frame *f, struct syscall_entry *entry) {
	entry->syscall_num = f->R.rax;
	entry->should_return_value = false;
	entry->return_value = 0;
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
			handle_halt ();
			break;
		case SYS_EXIT:
			handle_exit (entry);
			break;
		case SYS_FORK:
			handle_fork (f, entry);
			break;
		case SYS_EXEC:
			handle_exec (entry);
			break;
		case SYS_WAIT:
			handle_wait (entry);
			break;
		case SYS_CREATE:
			handle_create (entry);
			break;
		case SYS_REMOVE:
			handle_remove (entry);
			break;
		case SYS_OPEN:
			handle_open (entry);
			break;
		case SYS_FILESIZE:
			handle_filesize (entry);
			break;
		case SYS_READ:
			handle_read (entry);
			break;
		case SYS_WRITE:
			handle_write (entry);
			break;
		case SYS_SEEK:
			handle_seek (entry);
			break;
		case SYS_TELL:
			handle_tell (entry);
			break;
		case SYS_CLOSE:
			handle_close (entry);
			break;
		default:
			ASSERT (false); /* 현재 처리할 수 없는 syscall */
	}
}

static void
handle_halt (void) {
	//TODO: OS 자체를 강제로 끄는거 같아서, 스레드 종료나 그런거 없이 닫음 처리함. 맞는지 체크 필요
	power_off();
}

// void exit (int status);
// 인자 1개
// 리턴값 없음
static void
handle_exit (struct syscall_entry *entry) {
	int status = (int) entry->args[0];
	_exit (status);
}

// pid_t fork (const char *thread_name);
// 인자 1개
// 리턴값 pid_t - parent(현재스레드)에선 새로 생성된 pid, child는 0, 에러 시 TID_ERROR
static void
handle_fork (struct intr_frame *f, struct syscall_entry *entry) {
	entry->should_return_value = true;
	const char *thread_name = (const char *) entry->args[0];

	if (!is_valid_user_string ((char *) thread_name)) {
		_exit (-1);
	}

	// tid_t는 커널 스레드, pid_t는 사용자 프로세스 식별용,
	// 이 구현에서는 1:1로 매핑하여 동일한 값을 가짐. (typedef 사용)
	entry->should_return_value = process_fork (thread_name, f);
}

// int exec (const char *cmd_line);
// 인자 1개
// 리턴값 int - 성공 시 프로그램이 전환되므로 리턴 없음. 실패 시 -1 반환
static void
handle_exec (struct syscall_entry *entry) {
	entry->should_return_value = true;
	//TODO: 이거 보니까 argument passing 처리할 때, 이름이 cmd_line을 하는게 일관성 있는듯? 나중에 바꾸기
	const char *cmd_line = (const char *) entry->args[0];

	if (!is_valid_user_string (cmd_line)) {
		_exit (-1);
	}

	entry->return_value = process_exec (cmd_line);
	if (entry->return_value == -1) {
		_exit (-1);
	}
}

// int wait (pid_t pid);
// 인자 1개
// 리턴값 int - 해당 child의 exit status 값, 예외 상황에선 -1
static void
handle_wait (struct syscall_entry *entry) {
	entry->should_return_value = true;
	pid_t pid = (pid_t) entry->args[0];

	entry->return_value = process_wait (pid);
}

// bool create (const char *file, unsigned initial_size);
// 인자 2개
// 리턴값 bool - 성공하면 true, 실패하면 false
static void
handle_create (struct syscall_entry *entry) {
	entry->should_return_value = true;
	const char *filename = (const char *) entry->args[0];
	size_t initial_size = entry->args[1];

	if (!is_valid_user_string (filename)) {
		_exit (-1);
	}

	entry->return_value = filesys_create (filename, initial_size);
}

// bool remove (const char *file);
// 인자 2개
// 리턴값 bool - 성공하면 true, 실패하면 false
static void
handle_remove (struct syscall_entry *entry) {
	entry->should_return_value = true;
	const char *filename = (const char *) entry->args[0];

	if (!is_valid_user_string (filename)) {
		_exit (-1);
	}

	entry->return_value = filesys_remove (filename);
}

// int open (const char *file);
// 인자 1개
// 리턴값 int - fd 번호, 실패하면 -1
static void
handle_open (struct syscall_entry *entry) {
	entry->should_return_value = true;
	const char *filename = (const char *) entry->args[0];

	if (!is_valid_user_string (filename)) {
		_exit (-1);
	}

	// 파일 열기, 실패하면 -1을 리턴값으로 설정하고 종료
	struct file *file = filesys_open (filename);
	if (file == NULL) {
		entry->return_value = -1;
		return;
	}

	// thread의 fds에 새로운 fd 할당, 실패하면 -1을 리턴값으로 설정하고 종료
	int fd = fd_alloc (file);
	if (fd == -1) {
		file_close (file);
		entry->return_value = -1;
		return;
	}

	entry->return_value = fd;
}

// int filesize (int fd);
// 인자 1개
// 리턴값 int - 파일의 크기(바이트 단위)
static void
handle_filesize (struct syscall_entry *entry) {
	entry->should_return_value = true;
	int fd = (int) entry->args[0];

	// STD OUT/IN은 잘못된 요청임
	if (fd == STDOUT_FILENO || fd == STDIN_FILENO) {
		_exit (-1);
	}

	struct file *file = fd_lookup (fd);
	if (file == NULL) {
		_exit (-1);
	}

	entry->return_value = file_length (file);
}

// int read (int fd, void *buffer, unsigned size);
// 인자 3개
// 리턴값 int - 읽은 바이트 수, 읽을 수 없으면 0, 예외 상황은 -1
static void
handle_read (struct syscall_entry *entry) {
	entry->should_return_value = true;
	int fd = (int) entry->args[0];
	uint8_t *buffer = (void *) entry->args[1];
	size_t size = entry->args[2];

	if (!is_valid_user_buffer (buffer, size)) {
		_exit (-1);
	}

	// STDOUT_FILENO은 지원하지 않음, 종료
	if (fd == STDOUT_FILENO) {
		_exit (-1);
	}

	// STDIN_FILENO이면 키보드의 입력을 받기
	if (fd == STDIN_FILENO) {
		//TODO: 파일이 아니라 끝까지 무조건 버퍼 다 채울때까지 대기하는듯?
		// input_getc 자체에서 wait 한다고 하니까 락이나 sleep을 고려할 필요는 없어보임
		size_t idx = 0;
		while (idx == (size - 1)) {
			buffer[idx] = input_getc();
			idx++;
		}
		entry->return_value = size;
		return;
	}

	struct file *file = fd_lookup (fd);
	if (file == NULL) {
		_exit (-1);
	}

	// TODO: 이게 커서가 end면 0 반환해야하는데, 테스트 실패하면 추가 분기처리 필요
	entry->return_value = file_read (file, buffer, size);
}

// int write (int fd, const void *buffer, unsigned size);
// 인자 3개
// 리턴값 int - 쓴 바이트 수, 쓸 수 없으면 0, 예외 상황은 -1
static void
handle_write (struct syscall_entry *entry) {
	entry->should_return_value = true;
	int fd = (int) entry->args[0];
	const void *buffer = (const void *) entry->args[1];
	size_t size = entry->args[2];

	if (!is_valid_user_buffer ((void *) buffer, size)) {
		_exit (-1);
	}

	// STDOUT_FILENO 이면 콘솔에 쓰기
	if (fd == STDOUT_FILENO) {
		putbuf (buffer, size);
		entry->return_value = size;
		return;
	}

	// STDIN_FILENO은 지원하지 않음, 종료
	if (fd == STDIN_FILENO) {
		_exit (-1);
	}

	struct file *file = fd_lookup (fd);
	if (file == NULL) {
		_exit (-1);
	}

	entry->return_value = file_write (file, buffer, size);
}

// void seek (int fd, unsigned position);
// 인자 2개
// 리턴값 없음
static void
handle_seek (struct syscall_entry *entry) {
	int fd = (int) entry->args[0];
	off_t position = (off_t) entry->args[1];

	// STD OUT/IN은 잘못된 요청임
	if (fd == STDOUT_FILENO || fd == STDIN_FILENO) {
		_exit (-1);
	}

	struct file *file = fd_lookup (fd);
	if (file == NULL) {
		_exit (-1);
	}

	file_seek (file, position);
}

// unsigned tell (int fd);
// 인자 2개
// 리턴값 off_t, 다음 바이트의 위치
static void
handle_tell (struct syscall_entry *entry) {
	entry->should_return_value = true;
	int fd = (int) entry->args[0];

	// STD OUT/IN은 잘못된 요청임
	if (fd == STDOUT_FILENO || fd == STDIN_FILENO) {
		_exit (-1);
	}

	struct file *file = fd_lookup (fd);
	if (file == NULL) {
		_exit (-1);
	}

	entry->return_value = file_tell (file);
}

// void close (int fd);
// 인자 1개
// 리턴값 없음
static void
handle_close (struct syscall_entry *entry) {
	int fd = (int) entry->args[0];

	if (fd == STDIN_FILENO || fd == STDOUT_FILENO || !fd_close (fd)) {
		_exit (-1);
	}
}

// 프로세스 종료 시 사용
static void
_exit (int status) {
	thread_current ()->exit_status = status;
	thread_exit (); // 내부적으로 process_exit() 호출
}

static bool
is_valid_user_buffer (void *buf, size_t size) {
	void *p = buf;
	/* 산술 연산 시에는 uintptr_t 변환이 안전해보임. */
	void *buf_end = (void *) ((uintptr_t) p + size);

	if (size <= 0) {
		return get_next_page_if_valid (p) != NULL;
	}

	while (p < buf_end) {
		/* 페이지가 유효하면 다음 페이지, 아니면 NULL 반환. */
		p = get_next_page_if_valid (p);
		if (p == NULL) {
			return false;
		}
	}
	return true;
}

static bool
is_valid_user_string (char *str) {
	char *p = str;

	while (true) {
		/* 페이지가 유효하면 다음 페이지, 아니면 NULL 반환. */
		char *next_p = get_next_page_if_valid (p);
		if (next_p == NULL) {
			return false;
		}

		/* 현재 페이지 내부를 순회하며 문자열의 끝이 있는지 검사. */
		while (p < next_p) {
			if (*p == '\0') {
				return true;
			}
			p++;
		}

		/* 다음 페이지 검사 진행. */
		p = next_p;
	}
}
/* 해당 ptr의 페이지가 유효한지 확인하고,
   유효 시 다음 페이지 주소를 반환, 그렇지 않으면 NULL을 반환한다.

   구현의 편의를 위해 분리한 함수라서, is_valid_user_* 에서만 사용 추천
   리턴하는 다음 페이지 주소가 유효하지 않을 수 있음을 주의하기
   */
static void *
get_next_page_if_valid (void *ptr) {
	if (ptr == NULL) {
		return NULL;
	}

	/* 커널 영역 위인가? */
	if (!is_user_vaddr (ptr)) {
		return NULL;
	}

	/* thread가 가지는 유저 가상 주소(pml4 필드)가 unmapped 상태인가? */
	if (pml4_get_page (thread_current ()->pml4, ptr) == NULL) {
		return NULL;
	}

	return pg_next (ptr);
}

