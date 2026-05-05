#include "userprog/process.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "userprog/gdt.h"
#include "userprog/tss.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/flags.h"
#include "threads/init.h"
#include "threads/interrupt.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/mmu.h"
#include "threads/vaddr.h"
#include "intrinsic.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#ifdef VM
#include "vm/vm.h"
#endif

/* C99 표준 상 128 이상, pintos 요구사항은 없음 */
#define MAX_ARGC 128

struct fork_context {
	struct thread *parent;
	struct intr_frame parent_frame;
	struct child_state *child_state;
	struct semaphore child_ready; // 설정 완료 후에 자식이 읽도록 순서 제어
	struct semaphore fork_done;   // 자식이 공유 자원을 읽는 동안 해제하지 않도록 순서 제어
	bool success;
};

/* initd 스레드에게 child_state를 전달하기 위한 전역변수.
   process_create_initd()에서 thread_create() 호출 전에 쓰고,
   initd()에서 읽으므로 레이스 없음. */
static struct child_state *initd_child_state;

static void process_cleanup (void);
static void process_cleanup_user_memory (void);
static void process_cleanup_files (void);
static bool load (const char *cmd, struct intr_frame *if_);
static void initd (void *f_name);
static void init_fork_context (struct fork_context *, struct intr_frame *,
		struct child_state *);
static void __do_fork (void *);
static int parse_arg (char *cmd, char **arg_buf);

/* General process initializer for initd and other process. */
/* initd와 다른 프로세스에 공통으로 쓰이는 프로세스 이니셜라이저. */
static void
process_init (void) {
	struct thread *current = thread_current ();
}

/* Starts the first userland program, called "initd", loaded from FILE_NAME.
 * The new thread may be scheduled (and may even exit)
 * before process_create_initd() returns. Returns the initd's
 * thread id, or TID_ERROR if the thread cannot be created.
 * Notice that THIS SHOULD BE CALLED ONCE. */
/* FILE_NAME에서 로드한 첫 유저랜드 프로그램인 "initd"를 시작한다.
 * 새 스레드는 process_create_initd()가 리턴하기 전에 스케줄될 수 있고,
 * 심지어 종료될 수도 있다. 생성에 성공하면 initd의 thread id를,
 * 실패하면 TID_ERROR를 리턴한다.
 * 이 함수는 한 번만 호출되어야 한다는 점에 주의한다. */
tid_t
process_create_initd (const char *file_name) {
	char *fn_copy;
	char prog_name[16];
	tid_t tid;

	/* Make a copy of FILE_NAME.
	 * Otherwise there's a race between the caller and load(). */
	/* FILE_NAME의 복사본을 만든다.
	 * 그렇지 않으면 호출자와 load() 사이에 레이스가 생긴다. */
	fn_copy = palloc_get_page (0);
	if (fn_copy == NULL)
		return TID_ERROR;
	strlcpy (fn_copy, file_name, PGSIZE);

	// thread_create()는 thread name을 최대 16글자까지만 사용하며,
	// 전달된 이름은 내부 공간에 복사되어 저장된다.
	// 따라서 file_name에서 첫 공백 전까지, 최대 prog_name 크기만큼만 복사하여 념겨준다.
	size_t len = strcspn(file_name, " ");
	strlcpy(prog_name, file_name, len + 1 < sizeof prog_name ? len + 1 : sizeof prog_name);

	struct child_state *cs = malloc (sizeof *cs);
	if (cs == NULL) {
		palloc_free_page (fn_copy);
		return TID_ERROR;
	}
	init_child_state (cs);

	/* thread_create 전에 전역변수에 써둔다. initd()는 생성된 이후에만 읽으므로 레이스 없음. */
	initd_child_state = cs;

	/* Create a new thread to execute FILE_NAME. */
	/* FILE_NAME을 실행할 새 스레드를 만든다. */
	tid = thread_create (prog_name, PRI_DEFAULT, initd, fn_copy);
	if (tid == TID_ERROR) {
		initd_child_state = NULL;
		free (cs);
		palloc_free_page (fn_copy);
		return TID_ERROR;
	}

	cs->tid = tid;
	list_push_back (&thread_current ()->children, &cs->elem);

	return tid;
}

/* A thread function that launches first user process. */
/* 첫 유저 프로세스를 실행하는 스레드 함수. */
static void
initd (void *f_name) {
	thread_current ()->child_state = initd_child_state;
	initd_child_state = NULL;

#ifdef VM
	supplemental_page_table_init (&thread_current ()->spt);
#endif

	process_init ();

	if (process_exec (f_name) < 0)
		PANIC("Fail to launch initd\n");
	NOT_REACHED ();
}

/* Clones the current process as `name`. Returns the new process's thread id, or
 * TID_ERROR if the thread cannot be created. */
/* 현재 프로세스를 `name`이라는 이름으로 클론한다. 새 프로세스의 thread id를
 * 리턴하며, 스레드를 만들 수 없으면 TID_ERROR를 리턴한다. */
tid_t
process_fork (const char *name, struct intr_frame *if_) {
	tid_t tid = TID_ERROR; // 성공하면 덮어씌워짐 아니면 실패 상태
	struct thread *parent = thread_current();
	struct fork_context fork_ctx;
	struct child_state *child_state;

	child_state = malloc(sizeof *child_state);
	if (child_state == NULL) {
		goto out;
	}

	init_child_state(child_state);
	init_fork_context(&fork_ctx, if_, child_state);

	list_push_back(&parent->children, &child_state->elem);

	// 자식 스레드를 생성, 스케줄러에 따라 부모보다 먼저 실행될 수 있으므로 순서 제어가 필요
	tid = thread_create(name, PRI_DEFAULT, __do_fork, &fork_ctx);
	if (tid == TID_ERROR) {
		// 자식 스레드가 아예 생성되지 않아서 child 측 참조가 없음 → 직접 free
		list_remove(&child_state->elem);
		free(child_state);
		goto out;
	}

	child_state->tid = tid;

	sema_up(&fork_ctx.child_ready); // parent가 child에게 처리 가능하다고 알림
	sema_down(&fork_ctx.fork_done); // child가 parent에게 처리가 완료되었음을 알림

	if (!fork_ctx.success) {
		// 자식이 child_state를 참조한 채 thread_exit() 예정이므로 release로 처리
		list_remove(&child_state->elem);
		child_state_release(child_state); // parent 측 참조 해제
		goto out;
	}

	return tid;

out:
	return tid;
}

static void
init_fork_context (struct fork_context *ctx, struct intr_frame *parent_if,
		struct child_state *child_state) {
	ASSERT (ctx != NULL);
	ASSERT (parent_if != NULL);
	ASSERT (child_state != NULL);

	ctx->parent = thread_current ();
	ctx->parent_frame = *parent_if;
	ctx->child_state = child_state;
	ctx->success = false;

	sema_init (&ctx->child_ready, 0); /* parent가 up -> child가 처리 시작 */
	sema_init (&ctx->fork_done, 0);   /* child가 up -> parent에서 후처리 */
}


#ifndef VM
/* Duplicate the parent's address space by passing this function to the
 * pml4_for_each. This is only for the project 2. */
/* 이 함수를 pml4_for_each에 넘겨 부모의 주소 공간을 복제한다.
 * 이 코드는 project 2에서만 사용된다. */
static bool
duplicate_pte (uint64_t *pte, void *va, void *aux) {
	struct thread *current = thread_current ();
	struct thread *parent = (struct thread *) aux;
	void *parent_page;
	void *newpage;
	bool writable;

	/* 1. If the parent_page is kernel page, then return immediately. */
	if (is_kernel_vaddr (va))
		return true;

	/* 2. Resolve VA from the parent's page map level 4. */
	parent_page = pml4_get_page (parent->pml4, va);

	/* 3. Allocate new PAL_USER page for the child and set result to NEWPAGE. */
	newpage = palloc_get_page (PAL_USER);
	if (newpage == NULL)
		return false;

	/* 4. Duplicate parent's page to the new page and check whether parent's
	 *    page is writable or not (set WRITABLE according to the result). */
	memcpy (newpage, parent_page, PGSIZE);
	writable = is_writable (pte);

	/* 5. Add new page to child's page table at address VA with WRITABLE
	 *    permission. */
	if (!pml4_set_page (current->pml4, va, newpage, writable)) {
		/* 6. if fail to insert page, do error handling. */
		palloc_free_page (newpage);
		return false;
	}
	return true;
}
#endif

/* A thread function that copies parent's execution context.
 * Hint) parent->tf does not hold the userland context of the process.
 *       That is, you are required to pass second argument of process_fork to
 *       this function. */
/* 부모의 실행 컨텍스트를 복사하는 스레드 함수.
 * Hint) parent->tf는 프로세스의 유저랜드 컨텍스트를 들고 있지 않다.
 *       따라서 process_fork의 두 번째 인자를 이 함수에 전달해야 한다. */
static void
__do_fork (void *aux) {
	struct fork_context *fork_ctx = aux;
	struct intr_frame if_;
	struct thread *parent;
	struct thread *current = thread_current ();

	// parent가 child를 깨우면 진행
	sema_down (&fork_ctx->child_ready);

	parent = fork_ctx->parent;
	current->child_state = fork_ctx->child_state;

	/* 1. 부모의 컨텍스트를 로컬 스택으로 읽어 온다. */
	memcpy (&if_, &fork_ctx->parent_frame, sizeof if_);
	if_.R.rax = 0;

	/* 2. PT를 복제한다. */
	current->pml4 = pml4_create ();
	if (current->pml4 == NULL)
		goto error;

	process_activate (current);
#ifdef VM
	supplemental_page_table_init (&current->spt);
	if (!supplemental_page_table_copy (&current->spt, &parent->spt))
		goto error;
#else
	if (!pml4_for_each (parent->pml4, duplicate_pte, parent))
		goto error;
#endif

	/* 부모의 파일 디스크립터를 복제 */
	struct list_elem *e = list_begin (&parent->file_descriptors);
	while (e != list_end (&parent->file_descriptors)) {
		struct file_descriptor *parent_fde =
			list_entry (e, struct file_descriptor, elem);
		struct file *dup_file = file_duplicate (parent_fde->file);
		if (dup_file == NULL)
			goto error;
		if (fd_alloc (dup_file) == -1) {
			file_close (dup_file);
			goto error;
		}
		e = list_next (e);
	}

	process_init ();

	fork_ctx->success = true;
	sema_up (&fork_ctx->fork_done); // child가 parent를 꺠움

	/* 마지막으로 새로 생성한 프로세스로 전환한다. */
	do_iret (&if_);
error:
	fork_ctx->success = false;
	sema_up (&fork_ctx->fork_done);
	thread_exit ();
}

/* Switch the current execution context to the f_name.
 * Returns -1 on fail. */
/* 현재 실행 컨텍스트를 f_name으로 전환한다.
 * 실패하면 -1을 리턴한다. */
int
process_exec (void *f_name) {
	bool success;

	/* We cannot use the intr_frame in the thread structure.
	 * This is because when current thread rescheduled,
	 * it stores the execution information to the member. */
	/* 스레드 구조체 안의 intr_frame은 사용할 수 없다.
	 * 현재 스레드가 다시 스케줄될 때 실행 정보를 그 멤버에 저장하기 때문이다. */
	struct intr_frame _if;
	_if.ds = _if.es = _if.ss = SEL_UDSEG;
	_if.cs = SEL_UCSEG;
	_if.eflags = FLAG_IF | FLAG_MBS;

	/* exec는 현재 스레드/프로세스의 커널 상태는 유지하고,
       유저 프로그램 실행 이미지(code/data/stack)만 새 프로그램으로 교체한다.
	   따라서 fd table은 닫으면 안 되고, 기존 유저 주소공간(pml4)만 제거한다. */
	process_cleanup_user_memory ();

	/* And then load the binary */
	/* 그 다음 바이너리를 로드한다. */
	success = load (f_name, &_if);

	/* If load failed, quit. */
	/* 로드에 실패하면 종료한다. */
	palloc_free_page (f_name);
	if (!success)
		return -1;

	/* Start switched process. */
	/* 전환된 프로세스를 시작한다. */
	do_iret (&_if);
	NOT_REACHED ();
}


/* Waits for thread TID to die and returns its exit status.  If
 * it was terminated by the kernel (i.e. killed due to an
 * exception), returns -1.  If TID is invalid or if it was not a
 * child of the calling process, or if process_wait() has already
 * been successfully called for the given TID, returns -1
 * immediately, without waiting.
 *
 * This function will be implemented in problem 2-2.  For now, it
 * does nothing. */
/* thread TID가 종료될 때까지 기다렸다가 exit status를 리턴한다. 커널에 의해
 * 종료된 경우, 즉 exception 때문에 kill된 경우에는 -1을 리턴한다. TID가
 * 올바르지 않거나 호출 프로세스의 자식이 아니거나, 같은 TID에 대해
 * process_wait()가 이미 성공적으로 호출된 적이 있으면 기다리지 않고 즉시
 * -1을 리턴한다.
 *
 * 이 함수는 problem 2-2에서 구현한다. 지금은 아무 일도 하지 않는다. */
int
process_wait (tid_t child_tid) {
	struct child_state *cs = child_lookup (child_tid);

	/* 자식이 없거나 이미 wait한 경우 */
	if (cs == NULL || cs->waited)
		return -1;

	cs->waited = true;

	if (!cs->exited)
		sema_down (&cs->wait_sema);

	int status = cs->status;
	list_remove (&cs->elem);
	child_state_release (cs); // parent 측 참조 해제

	return status;
}

/* Exit the process. This function is called by thread_exit (). */
/* 프로세스를 종료한다. 이 함수는 thread_exit()에서 호출된다. */
void
process_exit (void) {
	struct thread *curr = thread_current ();
	printf ("%s: exit(%d)\n", curr->name, curr->exit_status);

	/* 자식에게 exit status를 전달하고 대기 중인 부모를 깨운다. */
	if (curr->child_state != NULL) {
		curr->child_state->status = curr->exit_status;
		curr->child_state->exited = true;
		sema_up (&curr->child_state->wait_sema);
		child_state_release (curr->child_state); // child 측 참조 해제
		curr->child_state = NULL;
	}

	/* 내 자식 목록에 남은 child_state의 parent 측 참조를 해제한다. */
	while (!list_empty (&curr->children)) {
		struct list_elem *e = list_pop_front (&curr->children);
		struct child_state *cs = list_entry (e, struct child_state, elem);
		child_state_release (cs); // parent 측 참조 해제
	}

	process_cleanup ();
}

/* Free the current process's resources. */
/* 현재 프로세스의 리소스를 해제한다. */
static void
process_cleanup (void) {
	process_cleanup_files ();
	process_cleanup_user_memory ();
}

static void
process_cleanup_files (void) {
	struct thread *curr = thread_current ();

	while (!list_empty (&curr->file_descriptors)) {
		struct list_elem *e = list_pop_front (&curr->file_descriptors);
		struct file_descriptor *fde = list_entry (e, struct file_descriptor, elem);
		file_close (fde->file);
		free (fde);
	}
}

static void
process_cleanup_user_memory (void) {
	struct thread *curr = thread_current ();

#ifdef VM
	supplemental_page_table_kill (&curr->spt);
#endif

	uint64_t *pml4;
	/* Destroy the current process's page directory and switch back
	 * to the kernel-only page directory. */
	/* 현재 프로세스의 페이지 디렉터리를 파괴하고 커널 전용 페이지 디렉터리로
	 * 다시 전환한다. */
	pml4 = curr->pml4;
	if (pml4 != NULL) {
		/* Correct ordering here is crucial.  We must set
		 * cur->pagedir to NULL before switching page directories,
		 * so that a timer interrupt can't switch back to the
		 * process page directory.  We must activate the base page
		 * directory before destroying the process's page
		 * directory, or our active page directory will be one
		 * that's been freed (and cleared). */
		/* 여기서는 순서가 매우 중요하다. 페이지 디렉터리를 전환하기 전에
		 * cur->pagedir를 NULL로 설정해야 한다. 그래야 타이머 인터럽트가 다시
		 * 프로세스 페이지 디렉터리로 전환하지 않는다. 또한 프로세스의 페이지
		 * 디렉터리를 파괴하기 전에 base 페이지 디렉터리를 activate해야 한다.
		 * 그렇지 않으면 이미 해제되고 클리어된 페이지 디렉터리가 active 상태가
		 * 될 수 있다. */
		curr->pml4 = NULL;
		pml4_activate (NULL);
		pml4_destroy (pml4);
	}
}

/* Sets up the CPU for running user code in the nest thread.
 * This function is called on every context switch. */
/* 다음 스레드에서 유저 코드를 실행할 수 있도록 CPU를 설정한다.
 * 이 함수는 매 context switch마다 호출된다. */
void
process_activate (struct thread *next) {
	/* Activate thread's page tables. */
	/* 스레드의 페이지 테이블을 activate한다. */
	pml4_activate (next->pml4);

	/* Set thread's kernel stack for use in processing interrupts. */
	/* 인터럽트 처리에 사용할 스레드의 커널 스택을 설정한다. */
	tss_update (next);
}

/* We load ELF binaries.  The following definitions are taken
 * from the ELF specification, [ELF1], more-or-less verbatim.  */
/* ELF 바이너리를 로드한다. 아래 정의들은 ELF specification [ELF1]에서 거의
 * 그대로 가져온 것이다. */

/* ELF types.  See [ELF1] 1-2. */
/* ELF 타입. [ELF1] 1-2를 참고한다. */
#define EI_NIDENT 16

#define PT_NULL    0            /* Ignore. */
#define PT_LOAD    1            /* Loadable segment. */
#define PT_DYNAMIC 2            /* Dynamic linking info. */
#define PT_INTERP  3            /* Name of dynamic loader. */
#define PT_NOTE    4            /* Auxiliary info. */
#define PT_SHLIB   5            /* Reserved. */
#define PT_PHDR    6            /* Program header table. */
#define PT_STACK   0x6474e551   /* Stack segment. */

#define PF_X 1          /* Executable. */
#define PF_W 2          /* Writable. */
#define PF_R 4          /* Readable. */

/* Executable header.  See [ELF1] 1-4 to 1-8.
 * This appears at the very beginning of an ELF binary. */
/* Executable 헤더. [ELF1] 1-4부터 1-8을 참고한다.
 * ELF 바이너리의 가장 앞부분에 위치한다. */
struct ELF64_hdr {
	unsigned char e_ident[EI_NIDENT];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint64_t e_entry;
	uint64_t e_phoff;
	uint64_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
};

struct ELF64_PHDR {
	uint32_t p_type;
	uint32_t p_flags;
	uint64_t p_offset;
	uint64_t p_vaddr;
	uint64_t p_paddr;
	uint64_t p_filesz;
	uint64_t p_memsz;
	uint64_t p_align;
};

/* Abbreviations */
/* 축약 이름. */
#define ELF ELF64_hdr
#define Phdr ELF64_PHDR

static bool setup_stack (struct intr_frame *if_);
static bool validate_segment (const struct Phdr *, struct file *);
static bool load_segment (struct file *file, off_t ofs, uint8_t *upage,
		uint32_t read_bytes, uint32_t zero_bytes,
		bool writable);

/* Loads an ELF executable from FILE_NAME into the current thread.
 * Stores the executable's entry point into *RIP
 * and its initial stack pointer into *RSP.
 * Returns true if successful, false otherwise. */
/* FILE_NAME의 ELF executable을 현재 스레드에 로드한다.
 * executable의 entry point를 *RIP에 저장하고,
 * 초기 stack pointer를 *RSP에 저장한다.
 * 성공하면 true, 아니면 false를 리턴한다. */
static bool
load (const char *cmd, struct intr_frame *if_) {
	struct thread *t = thread_current ();
	struct ELF ehdr;
	struct file *file = NULL;
	off_t file_ofs;
	bool success = false;
	int i;

	char *arg_buf[MAX_ARGC];
	int argc = parse_arg (cmd, arg_buf);
	/* 파싱 실패 시 전체 작업 실패. */
	if (argc == -1)
		goto done;

	char *file_name = arg_buf[0];

	/* Allocate and activate page directory. */
	/* 페이지 디렉터리를 할당하고 activate한다. */
	t->pml4 = pml4_create ();
	if (t->pml4 == NULL)
		goto done;
	process_activate (thread_current ());

	/* Open executable file. */
	/* executable 파일을 연다. */
	file = filesys_open (file_name);
	if (file == NULL) {
		printf ("load: %s: open failed\n", file_name);
		goto done;
	}

	/* Read and verify executable header. */
	/* executable 헤더를 읽고 검증한다. */
	if (file_read (file, &ehdr, sizeof ehdr) != sizeof ehdr
			|| memcmp (ehdr.e_ident, "\177ELF\2\1\1", 7)
			|| ehdr.e_type != 2
			|| ehdr.e_machine != 0x3E // amd64
			|| ehdr.e_version != 1
			|| ehdr.e_phentsize != sizeof (struct Phdr)
			|| ehdr.e_phnum > 1024) {
		printf ("load: %s: error loading executable\n", file_name);
		goto done;
	}

	/* Read program headers. */
	/* 프로그램 헤더들을 읽는다. */
	file_ofs = ehdr.e_phoff;
	for (i = 0; i < ehdr.e_phnum; i++) {
		struct Phdr phdr;

		if (file_ofs < 0 || file_ofs > file_length (file))
			goto done;
		file_seek (file, file_ofs);

		if (file_read (file, &phdr, sizeof phdr) != sizeof phdr)
			goto done;
		file_ofs += sizeof phdr;
		switch (phdr.p_type) {
			case PT_NULL:
			case PT_NOTE:
			case PT_PHDR:
			case PT_STACK:
			default:
				/* Ignore this segment. */
				/* 이 세그먼트는 무시한다. */
				break;
			case PT_DYNAMIC:
			case PT_INTERP:
			case PT_SHLIB:
				goto done;
			case PT_LOAD:
				if (validate_segment (&phdr, file)) {
					bool writable = (phdr.p_flags & PF_W) != 0;
					uint64_t file_page = phdr.p_offset & ~PGMASK;
					uint64_t mem_page = phdr.p_vaddr & ~PGMASK;
					uint64_t page_offset = phdr.p_vaddr & PGMASK;
					uint32_t read_bytes, zero_bytes;
					if (phdr.p_filesz > 0) {
						/* Normal segment.
						 * Read initial part from disk and zero the rest. */
						/* 일반 세그먼트.
						 * 앞부분은 디스크에서 읽고 나머지는 zero로 채운다. */
						read_bytes = page_offset + phdr.p_filesz;
						zero_bytes = (ROUND_UP (page_offset + phdr.p_memsz, PGSIZE)
								- read_bytes);
					} else {
						/* Entirely zero.
						 * Don't read anything from disk. */
						/* 전체가 zero인 세그먼트.
						 * 디스크에서 아무것도 읽지 않는다. */
						read_bytes = 0;
						zero_bytes = ROUND_UP (page_offset + phdr.p_memsz, PGSIZE);
					}
					if (!load_segment (file, file_page, (void *) mem_page,
								read_bytes, zero_bytes, writable))
						goto done;
				}
				else
					goto done;
				break;
		}
	}

	/* Set up stack. */
	/* 스택을 설정한다. */
	if (!setup_stack (if_))
		goto done;

	// TODO: 이거 좀 별로인듯? 외부 함수로 적절하게 빼던가 하기.
	/* Start address. */
	/* 시작 주소. */
	if_->rip = ehdr.e_entry;
	if_->rsp = USER_STACK;

	for (int i = 0; i < argc; i++) {
		char *arg = arg_buf[argc - 1 - i];
		size_t arg_size = strlen (arg) + 1;
		/* stack은 커질 때 값이 내려가니까 먼저 내리기. */
		if_->rsp -= arg_size;
		strlcpy ((void *) if_->rsp, arg, arg_size);
		arg_buf[argc - 1 - i] = (char *) if_->rsp;
	}

	/* 비트 연산으로 8의 배수로 내림. */
	if_->rsp = if_->rsp & ~7;

	/* 스택은 바이트 단위로 이동. */
	if_->rsp -= sizeof (char *);
	*(char **) if_->rsp = NULL;

	for (int i = 0; i < argc; i++) {
		if_->rsp -= sizeof (uintptr_t);
		/* 역순으로 추가. */
		*(uintptr_t *) if_->rsp = (uintptr_t) arg_buf[argc - i - 1];
	}

	/* argv[0], argc의 위치를 저장 */
	if_->R.rsi = if_->rsp;
	if_->R.rdi = argc;

	if_->rsp -= sizeof (char *);
	/* fake return address. */
	*(uintptr_t *) if_->rsp = 0;

	/* rsp를 최종 위치로 이동. */
	if_->rsp -= sizeof (char *);

	success = true;

done:
	/* We arrive here whether the load is successful or not. */
	/* 로드 성공 여부와 관계없이 이 지점에 도달한다. */
	file_close (file);
	return success;
}

/* Checks whether PHDR describes a valid, loadable segment in
 * FILE and returns true if so, false otherwise. */
/* PHDR이 FILE 안의 유효하고 로드 가능한 세그먼트를 설명하는지 확인한다.
 * 그렇다면 true, 아니면 false를 리턴한다. */
static bool
validate_segment (const struct Phdr *phdr, struct file *file) {
	/* p_offset and p_vaddr must have the same page offset. */
	/* p_offset과 p_vaddr은 같은 페이지 offset을 가져야 한다. */
	if ((phdr->p_offset & PGMASK) != (phdr->p_vaddr & PGMASK))
		return false;

	/* p_offset must point within FILE. */
	/* p_offset은 FILE 내부를 가리켜야 한다. */
	if (phdr->p_offset > (uint64_t) file_length (file))
		return false;

	/* p_memsz must be at least as big as p_filesz. */
	/* p_memsz는 p_filesz 이상이어야 한다. */
	if (phdr->p_memsz < phdr->p_filesz)
		return false;

	/* The segment must not be empty. */
	/* 세그먼트는 비어 있으면 안 된다. */
	if (phdr->p_memsz == 0)
		return false;

	/* The virtual memory region must both start and end within the
	   user address space range. */
	/* 가상 메모리 영역의 시작과 끝은 모두 유저 주소 공간 범위 안에 있어야 한다. */
	if (!is_user_vaddr ((void *) phdr->p_vaddr))
		return false;
	if (!is_user_vaddr ((void *) (phdr->p_vaddr + phdr->p_memsz)))
		return false;

	/* The region cannot "wrap around" across the kernel virtual
	   address space. */
	/* 이 영역은 커널 가상 주소 공간을 가로질러 "wrap around"되면 안 된다. */
	if (phdr->p_vaddr + phdr->p_memsz < phdr->p_vaddr)
		return false;

	/* Disallow mapping page 0.
	   Not only is it a bad idea to map page 0, but if we allowed
	   it then user code that passed a null pointer to system calls
	   could quite likely panic the kernel by way of null pointer
	   assertions in memcpy(), etc. */
	/* page 0 맵핑을 금지한다.
	   page 0을 맵핑하는 것은 좋지 않을 뿐 아니라, 허용할 경우 시스템 콜에 null
	   pointer를 넘긴 유저 코드가 memcpy() 등의 null pointer assertion을 통해
	   커널 panic을 일으킬 가능성이 높다. */
	if (phdr->p_vaddr < PGSIZE)
		return false;

	/* It's okay. */
	/* 유효하다. */
	return true;
}

#ifndef VM
/* Codes of this block will be ONLY USED DURING project 2.
 * If you want to implement the function for whole project 2, implement it
 * outside of #ifndef macro. */
/* 이 블록의 코드는 project 2에서만 사용된다.
 * project 2 전체에서 사용할 함수를 구현하려면 #ifndef 매크로 바깥에 구현한다. */

/* load() helpers. */
/* load() 헬퍼. */
static bool install_page (void *upage, void *kpage, bool writable);

/* Loads a segment starting at offset OFS in FILE at address
 * UPAGE.  In total, READ_BYTES + ZERO_BYTES bytes of virtual
 * memory are initialized, as follows:
 *
 * - READ_BYTES bytes at UPAGE must be read from FILE
 * starting at offset OFS.
 *
 * - ZERO_BYTES bytes at UPAGE + READ_BYTES must be zeroed.
 *
 * The pages initialized by this function must be writable by the
 * user process if WRITABLE is true, read-only otherwise.
 *
 * Return true if successful, false if a memory allocation error
 * or disk read error occurs. */
/* FILE의 offset OFS에서 시작하는 세그먼트를 UPAGE 주소에 로드한다.
 * 총 READ_BYTES + ZERO_BYTES 바이트의 가상 메모리를 다음 방식으로 초기화한다.
 *
 * - READ_BYTES 바이트는 FILE의 offset OFS부터 읽어 UPAGE에 채워야 한다.
 *
 * - ZERO_BYTES 바이트는 UPAGE + READ_BYTES부터 zero로 채워야 한다.
 *
 * 이 함수가 초기화하는 페이지들은 WRITABLE이 true이면 유저 프로세스가 쓸 수
 * 있어야 하고, 그렇지 않으면 read-only여야 한다.
 *
 * 성공하면 true를 리턴하고, 메모리 할당 에러나 디스크 read 에러가 발생하면
 * false를 리턴한다. */
static bool
load_segment (struct file *file, off_t ofs, uint8_t *upage,
		uint32_t read_bytes, uint32_t zero_bytes, bool writable) {
	ASSERT ((read_bytes + zero_bytes) % PGSIZE == 0);
	ASSERT (pg_ofs (upage) == 0);
	ASSERT (ofs % PGSIZE == 0);

	file_seek (file, ofs);
	while (read_bytes > 0 || zero_bytes > 0) {
		/* Do calculate how to fill this page.
		 * We will read PAGE_READ_BYTES bytes from FILE
		 * and zero the final PAGE_ZERO_BYTES bytes. */
		/* 이 페이지를 어떻게 채울지 계산한다.
		 * FILE에서 PAGE_READ_BYTES 바이트를 읽고,
		 * 마지막 PAGE_ZERO_BYTES 바이트는 zero로 채운다. */
		size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
		size_t page_zero_bytes = PGSIZE - page_read_bytes;

		/* Get a page of memory. */
		/* 메모리 페이지를 하나 얻는다. */
		uint8_t *kpage = palloc_get_page (PAL_USER);
		if (kpage == NULL)
			return false;

		/* Load this page. */
		/* 이 페이지를 로드한다. */
		if (file_read (file, kpage, page_read_bytes) != (int) page_read_bytes) {
			palloc_free_page (kpage);
			return false;
		}
		memset (kpage + page_read_bytes, 0, page_zero_bytes);

		/* Add the page to the process's address space. */
		/* 페이지를 프로세스의 주소 공간에 추가한다. */
		if (!install_page (upage, kpage, writable)) {
			printf("fail\n");
			palloc_free_page (kpage);
			return false;
		}

		/* Advance. */
		/* 다음 페이지로 진행한다. */
		read_bytes -= page_read_bytes;
		zero_bytes -= page_zero_bytes;
		upage += PGSIZE;
	}
	return true;
}

/* Create a minimal stack by mapping a zeroed page at the USER_STACK */
/* USER_STACK에 zero로 채운 페이지를 맵핑해 최소 스택을 만든다. */
static bool
setup_stack (struct intr_frame *if_) {
	uint8_t *kpage;
	bool success = false;

	kpage = palloc_get_page (PAL_USER | PAL_ZERO);
	if (kpage != NULL) {
		success = install_page (((uint8_t *) USER_STACK) - PGSIZE, kpage, true);
		if (success)
			if_->rsp = USER_STACK;
		else
			palloc_free_page (kpage);
	}
	return success;
}

/* Adds a mapping from user virtual address UPAGE to kernel
 * virtual address KPAGE to the page table.
 * If WRITABLE is true, the user process may modify the page;
 * otherwise, it is read-only.
 * UPAGE must not already be mapped.
 * KPAGE should probably be a page obtained from the user pool
 * with palloc_get_page().
 * Returns true on success, false if UPAGE is already mapped or
 * if memory allocation fails. */
/* 유저 가상 주소 UPAGE에서 커널 가상 주소 KPAGE로 가는 맵핑을 페이지 테이블에
 * 추가한다.
 * WRITABLE이 true이면 유저 프로세스가 페이지를 수정할 수 있고, 아니면
 * read-only이다.
 * UPAGE는 이미 맵핑되어 있으면 안 된다.
 * KPAGE는 보통 palloc_get_page()로 유저 풀에서 얻은 페이지여야 한다.
 * 성공하면 true, UPAGE가 이미 맵핑되어 있거나 메모리 할당이 실패하면 false를
 * 리턴한다. */
static bool
install_page (void *upage, void *kpage, bool writable) {
	struct thread *t = thread_current ();

	/* Verify that there's not already a page at that virtual
	 * address, then map our page there. */
	/* 해당 가상 주소에 이미 페이지가 없는지 확인한 뒤, 그 위치에 페이지를
	 * 맵핑한다. */
	return (pml4_get_page (t->pml4, upage) == NULL
			&& pml4_set_page (t->pml4, upage, kpage, writable));
}
#else
/* From here, codes will be used after project 3.
 * If you want to implement the function for only project 2, implement it on the
 * upper block. */
/* 여기부터의 코드는 project 3 이후에 사용된다.
 * project 2에서만 사용할 함수를 구현하려면 위쪽 블록에 구현한다. */

static bool
lazy_load_segment (struct page *page, void *aux) {
	/* TODO: Load the segment from the file */
	/* TODO: 파일에서 세그먼트를 로드한다. */
	/* TODO: This called when the first page fault occurs on address VA. */
	/* TODO: 주소 VA에서 첫 page fault가 발생했을 때 호출된다. */
	/* TODO: VA is available when calling this function. */
	/* TODO: 이 함수를 호출할 때 VA를 사용할 수 있다. */
}

/* Loads a segment starting at offset OFS in FILE at address
 * UPAGE.  In total, READ_BYTES + ZERO_BYTES bytes of virtual
 * memory are initialized, as follows:
 *
 * - READ_BYTES bytes at UPAGE must be read from FILE
 * starting at offset OFS.
 *
 * - ZERO_BYTES bytes at UPAGE + READ_BYTES must be zeroed.
 *
 * The pages initialized by this function must be writable by the
 * user process if WRITABLE is true, read-only otherwise.
 *
 * Return true if successful, false if a memory allocation error
 * or disk read error occurs. */
/* FILE의 offset OFS에서 시작하는 세그먼트를 UPAGE 주소에 로드한다.
 * 총 READ_BYTES + ZERO_BYTES 바이트의 가상 메모리를 다음 방식으로 초기화한다.
 *
 * - READ_BYTES 바이트는 FILE의 offset OFS부터 읽어 UPAGE에 채워야 한다.
 *
 * - ZERO_BYTES 바이트는 UPAGE + READ_BYTES부터 zero로 채워야 한다.
 *
 * 이 함수가 초기화하는 페이지들은 WRITABLE이 true이면 유저 프로세스가 쓸 수
 * 있어야 하고, 그렇지 않으면 read-only여야 한다.
 *
 * 성공하면 true를 리턴하고, 메모리 할당 에러나 디스크 read 에러가 발생하면
 * false를 리턴한다. */
static bool
load_segment (struct file *file, off_t ofs, uint8_t *upage,
		uint32_t read_bytes, uint32_t zero_bytes, bool writable) {
	ASSERT ((read_bytes + zero_bytes) % PGSIZE == 0);
	ASSERT (pg_ofs (upage) == 0);
	ASSERT (ofs % PGSIZE == 0);

	while (read_bytes > 0 || zero_bytes > 0) {
		/* Do calculate how to fill this page.
		 * We will read PAGE_READ_BYTES bytes from FILE
		 * and zero the final PAGE_ZERO_BYTES bytes. */
		/* 이 페이지를 어떻게 채울지 계산한다.
		 * FILE에서 PAGE_READ_BYTES 바이트를 읽고,
		 * 마지막 PAGE_ZERO_BYTES 바이트는 zero로 채운다. */
		size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
		size_t page_zero_bytes = PGSIZE - page_read_bytes;

		/* TODO: Set up aux to pass information to the lazy_load_segment. */
		/* TODO: lazy_load_segment에 정보를 전달할 수 있도록 aux를 설정한다. */
		void *aux = NULL;
		if (!vm_alloc_page_with_initializer (VM_ANON, upage,
					writable, lazy_load_segment, aux))
			return false;

		/* Advance. */
		/* 다음 페이지로 진행한다. */
		read_bytes -= page_read_bytes;
		zero_bytes -= page_zero_bytes;
		upage += PGSIZE;
	}
	return true;
}

/* Create a PAGE of stack at the USER_STACK. Return true on success. */
/* USER_STACK에 스택 PAGE를 만든다. 성공하면 true를 리턴한다. */
static bool
setup_stack (struct intr_frame *if_) {
	bool success = false;
	void *stack_bottom = (void *) (((uint8_t *) USER_STACK) - PGSIZE);

	/* TODO: Map the stack on stack_bottom and claim the page immediately.
	 * TODO: If success, set the rsp accordingly.
	 * TODO: You should mark the page is stack. */
	/* TODO: stack_bottom에 스택을 맵핑하고 즉시 페이지를 claim한다.
	 * TODO: 성공하면 그에 맞게 rsp를 설정한다.
	 * TODO: 해당 페이지를 스택으로 표시해야 한다. */
	/* TODO: Your code goes here */
	/* TODO: 여기에 코드를 작성한다. */

	return success;
}
#endif /* VM */

/* 들어온 인자 파싱.
 * 반환값은 파싱된 수(argc), 에러나면 -1 반환. */
static int
parse_arg (char *cmd, char **arg_buf) {
	thread_current ();

	char *save_ptr, *token;
	/* 구분자, delimiter. */
	char *delim = " ";
	int argc = 0;

	/* 처음에는 처리할 문자열를 넘겨줘야 함. strtok_r() 참고. */
	for (token = strtok_r (cmd, delim, &save_ptr); token != NULL;
			token = strtok_r (NULL, delim, &save_ptr)) {
		if (argc >= MAX_ARGC) {
			/* 사이즈 제한 넘어가면 실패. */
			return -1;
		}
		arg_buf[argc++] = token;
	}
	arg_buf[argc] = NULL;

	return argc;
}
