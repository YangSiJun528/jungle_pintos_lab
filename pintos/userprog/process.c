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

/* C99 표준 상 128까지는 필요하다지만,
   그러면 포인터 메모리로만 1kb되버리고, 실제로 그렇게까지 필요한 케이스도 적을꺼라
   임의로 더 낮은 값으로 설정 */
#define MAX_ARGC 32

static void process_cleanup (void);
static bool load (const char *cmd, struct intr_frame *if_);
static void initd (void *f_name);
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

	/* Create a new thread to execute FILE_NAME. */
	/* FILE_NAME을 실행할 새 스레드를 만든다. */
	tid = thread_create (prog_name, PRI_DEFAULT, initd, fn_copy);

	if (tid == TID_ERROR)
		palloc_free_page (fn_copy);
	return tid;
}

/* A thread function that launches first user process. */
/* 첫 유저 프로세스를 실행하는 스레드 함수. */
static void
initd (void *f_name) {
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
process_fork (const char *name, struct intr_frame *if_ UNUSED) {
	/* Clone current thread to new thread.*/
	/* 현재 스레드를 새 스레드로 클론한다. */
	return thread_create (name,
			PRI_DEFAULT, __do_fork, thread_current ());
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

	/* 1. TODO: If the parent_page is kernel page, then return immediately. */
	/* 1. TODO: parent_page가 커널 페이지라면 즉시 리턴한다. */

	/* 2. Resolve VA from the parent's page map level 4. */
	/* 2. 부모의 page map level 4에서 VA를 해석한다. */
	parent_page = pml4_get_page (parent->pml4, va);

	/* 3. TODO: Allocate new PAL_USER page for the child and set result to
	 *    TODO: NEWPAGE. */
	/* 3. TODO: 자식 프로세스를 위한 새 PAL_USER 페이지를 할당하고 결과를
	 *    TODO: NEWPAGE에 설정한다. */

	/* 4. TODO: Duplicate parent's page to the new page and
	 *    TODO: check whether parent's page is writable or not (set WRITABLE
	 *    TODO: according to the result). */
	/* 4. TODO: 부모 페이지를 새 페이지에 복제하고, 부모 페이지가 writable인지
	 *    TODO: 확인한 뒤 그 결과에 따라 WRITABLE을 설정한다. */

	/* 5. Add new page to child's page table at address VA with WRITABLE
	 *    permission. */
	/* 5. 자식의 페이지 테이블에 VA 주소로 새 페이지를 추가하고 WRITABLE
	 *    퍼미션을 적용한다. */
	if (!pml4_set_page (current->pml4, va, newpage, writable)) {
		/* 6. TODO: if fail to insert page, do error handling. */
		/* 6. TODO: 페이지 삽입에 실패하면 에러 핸들링을 수행한다. */
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
	struct intr_frame if_;
	struct thread *parent = (struct thread *) aux;
	struct thread *current = thread_current ();
	/* TODO: somehow pass the parent_if. (i.e. process_fork()'s if_) */
	/* TODO: 어떤 방식으로든 parent_if를 전달한다. 즉, process_fork()의 if_이다. */
	struct intr_frame *parent_if;
	bool succ = true;

	/* 1. Read the cpu context to local stack. */
	/* 1. CPU 컨텍스트를 로컬 스택으로 읽어 온다. */
	memcpy (&if_, parent_if, sizeof (struct intr_frame));

	/* 2. Duplicate PT */
	/* 2. PT를 복제한다. */
	current->pml4 = pml4_create();
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

	/* TODO: Your code goes here.
	 * TODO: Hint) To duplicate the file object, use `file_duplicate`
	 * TODO:       in include/filesys/file.h. Note that parent should not return
	 * TODO:       from the fork() until this function successfully duplicates
	 * TODO:       the resources of parent.*/
	/* TODO: 여기에 코드를 작성한다.
	 * TODO: Hint) 파일 오브젝트를 복제하려면 include/filesys/file.h의
	 * TODO:       `file_duplicate`를 사용한다. 이 함수가 부모의 리소스를
	 * TODO:       성공적으로 복제하기 전까지 부모는 fork()에서 리턴하면 안 된다. */

	process_init ();

	/* Finally, switch to the newly created process. */
	/* 마지막으로 새로 생성한 프로세스로 전환한다. */
	if (succ)
		do_iret (&if_);
error:
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

	/* We first kill the current context */
	/* 먼저 현재 컨텍스트를 정리한다. */
	process_cleanup ();

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
process_wait (tid_t child_tid UNUSED) {
	/* XXX: Hint) The pintos exit if process_wait (initd), we recommend you
	 * XXX:       to add infinite loop here before
	 * XXX:       implementing the process_wait. */
	/* XXX: Hint) process_wait(initd)에서 Pintos가 종료되므로, process_wait를
	 * XXX:       구현하기 전에는 여기에 무한 루프를 넣는 것을 권장한다. */

	for (int i = 10000; i >= 0; i--) {
		thread_yield ();
	}
	return -1;
}

/* Exit the process. This function is called by thread_exit (). */
/* 프로세스를 종료한다. 이 함수는 thread_exit()에서 호출된다. */
void
process_exit (void) {
	struct thread *curr = thread_current ();

	printf ("%s: exit(%d)\n", curr->name, curr->exit_status);
	process_cleanup ();
}

/* Free the current process's resources. */
/* 현재 프로세스의 리소스를 해제한다. */
static void
process_cleanup (void) {
	struct thread *curr = thread_current ();

	while (!list_empty (&curr->file_descriptors)) {
		struct list_elem *e = list_pop_front (&curr->file_descriptors);
		struct file_descriptor *fde = list_entry (e, struct file_descriptor, elem);
		file_close (fde->file);
		free (fde);
	}

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

	char *arg_buf[128];
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
		if (argc >= 128) {
			/* 사이즈 제한 넘어가면 실패. */
			return -1;
		}
		arg_buf[argc++] = token;
	}
	arg_buf[argc] = NULL;

	return argc;
}
