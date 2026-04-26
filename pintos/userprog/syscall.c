#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"

void syscall_entry (void);
void syscall_handler (struct intr_frame *);

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
syscall_handler (struct intr_frame *f UNUSED) {
	// TODO: Your implementation goes here.
	// TODO: 여기에 구현을 작성한다.
	printf ("system call!\n");
	thread_exit ();
}
