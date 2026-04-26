#include "userprog/exception.h"
#include <inttypes.h>
#include <stdio.h>
#include "userprog/gdt.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "intrinsic.h"

/* Number of page faults processed. */
/* 처리된 page fault 수. */
static long long page_fault_cnt;

static void kill (struct intr_frame *);
static void page_fault (struct intr_frame *);

/* Registers handlers for interrupts that can be caused by user
   programs.

   In a real Unix-like OS, most of these interrupts would be
   passed along to the user process in the form of signals, as
   described in [SV-386] 3-24 and 3-25, but we don't implement
   signals.  Instead, we'll make them simply kill the user
   process.

   Page faults are an exception.  Here they are treated the same
   way as other exceptions, but this will need to change to
   implement virtual memory.

   Refer to [IA32-v3a] section 5.15 "Exception and Interrupt
   Reference" for a description of each of these exceptions. */
/* 유저 프로그램이 일으킬 수 있는 인터럽트의 핸들러를 등록한다.

   실제 Unix-like OS에서는 이 인터럽트 대부분을 [SV-386] 3-24와 3-25에 설명된
   signal 형태로 유저 프로세스에 전달한다. 하지만 여기서는 signal을 구현하지
   않고, 단순히 유저 프로세스를 kill한다.

   Page fault는 예외이다. 여기서는 다른 exception과 같은 방식으로 처리하지만,
   virtual memory를 구현하려면 이 부분을 바꿔야 한다.

   각 exception에 대한 설명은 [IA32-v3a] section 5.15 "Exception and Interrupt
   Reference"를 참고한다. */
void
exception_init (void) {
	/* These exceptions can be raised explicitly by a user program,
	   e.g. via the INT, INT3, INTO, and BOUND instructions.  Thus,
	   we set DPL==3, meaning that user programs are allowed to
	   invoke them via these instructions. */
	/* 이 exception들은 유저 프로그램이 INT, INT3, INTO, BOUND instruction 등을
	   통해 명시적으로 발생시킬 수 있다. 그래서 DPL==3으로 설정해 유저 프로그램이
	   이러한 instruction으로 호출할 수 있게 한다. */
	intr_register_int (3, 3, INTR_ON, kill, "#BP Breakpoint Exception");
	intr_register_int (4, 3, INTR_ON, kill, "#OF Overflow Exception");
	intr_register_int (5, 3, INTR_ON, kill,
			"#BR BOUND Range Exceeded Exception");

	/* These exceptions have DPL==0, preventing user processes from
	   invoking them via the INT instruction.  They can still be
	   caused indirectly, e.g. #DE can be caused by dividing by
	   0.  */
	/* 이 exception들은 DPL==0이므로 유저 프로세스가 INT instruction으로 호출할
	   수 없다. 그래도 간접적으로는 발생할 수 있다. 예를 들어 #DE는 0으로 나누면
	   발생할 수 있다. */
	intr_register_int (0, 0, INTR_ON, kill, "#DE Divide Error");
	intr_register_int (1, 0, INTR_ON, kill, "#DB Debug Exception");
	intr_register_int (6, 0, INTR_ON, kill, "#UD Invalid Opcode Exception");
	intr_register_int (7, 0, INTR_ON, kill,
			"#NM Device Not Available Exception");
	intr_register_int (11, 0, INTR_ON, kill, "#NP Segment Not Present");
	intr_register_int (12, 0, INTR_ON, kill, "#SS Stack Fault Exception");
	intr_register_int (13, 0, INTR_ON, kill, "#GP General Protection Exception");
	intr_register_int (16, 0, INTR_ON, kill, "#MF x87 FPU Floating-Point Error");
	intr_register_int (19, 0, INTR_ON, kill,
			"#XF SIMD Floating-Point Exception");

	/* Most exceptions can be handled with interrupts turned on.
	   We need to disable interrupts for page faults because the
	   fault address is stored in CR2 and needs to be preserved. */
	/* 대부분의 exception은 인터럽트가 켜진 상태로 처리할 수 있다.
	   page fault는 fault address가 CR2에 저장되고 보존되어야 하므로 인터럽트를
	   비활성화해야 한다. */
	intr_register_int (14, 0, INTR_OFF, page_fault, "#PF Page-Fault Exception");
}


/* Prints exception statistics. */
/* exception 통계를 출력한다. */
void
exception_print_stats (void) {
	printf ("Exception: %lld page faults\n", page_fault_cnt);
}

/* Handler for an exception (probably) caused by a user process. */
/* 유저 프로세스가 일으켰을 가능성이 높은 exception을 처리하는 핸들러. */
static void
kill (struct intr_frame *f) {
	/* This interrupt is one (probably) caused by a user process.
	   For example, the process might have tried to access unmapped
	   virtual memory (a page fault).  For now, we simply kill the
	   user process.  Later, we'll want to handle page faults in
	   the kernel.  Real Unix-like operating systems pass most
	   exceptions back to the process via signals, but we don't
	   implement them. */
	/* 이 인터럽트는 유저 프로세스가 일으켰을 가능성이 높다. 예를 들어 프로세스가
	   맵핑되지 않은 가상 메모리에 접근하려고 했을 수 있다(page fault). 지금은
	   단순히 유저 프로세스를 kill한다. 나중에는 커널에서 page fault를 처리하게
	   될 것이다. 실제 Unix-like 운영체제는 대부분의 exception을 signal로
	   프로세스에 돌려주지만, 여기서는 구현하지 않는다. */

	/* The interrupt frame's code segment value tells us where the
	   exception originated. */
	/* interrupt frame의 code segment 값은 exception이 어디서 시작됐는지 알려준다. */
	switch (f->cs) {
		case SEL_UCSEG:
			/* User's code segment, so it's a user exception, as we
			   expected.  Kill the user process.  */
			/* 유저 code segment이므로 예상대로 유저 exception이다.
			   유저 프로세스를 kill한다. */
			printf ("%s: dying due to interrupt %#04llx (%s).\n",
					thread_name (), f->vec_no, intr_name (f->vec_no));
			intr_dump_frame (f);
			thread_exit ();

		case SEL_KCSEG:
			/* Kernel's code segment, which indicates a kernel bug.
			   Kernel code shouldn't throw exceptions.  (Page faults
			   may cause kernel exceptions--but they shouldn't arrive
			   here.)  Panic the kernel to make the point.  */
			/* 커널 code segment이며, 이는 커널 버그를 의미한다. 커널 코드는
			   exception을 던지면 안 된다. page fault가 커널 exception을 일으킬 수는
			   있지만 여기로 오면 안 된다. 이를 명확히 하기 위해 커널 panic을 낸다. */
			intr_dump_frame (f);
			PANIC ("Kernel bug - unexpected interrupt in kernel");

		default:
			/* Some other code segment?  Shouldn't happen.  Panic the
			   kernel. */
			/* 다른 code segment라면 발생하면 안 되는 상황이다. 커널 panic을 낸다. */
			printf ("Interrupt %#04llx (%s) in unknown segment %04x\n",
					f->vec_no, intr_name (f->vec_no), f->cs);
			thread_exit ();
	}
}

/* Page fault handler.  This is a skeleton that must be filled in
   to implement virtual memory.  Some solutions to project 2 may
   also require modifying this code.

   At entry, the address that faulted is in CR2 (Control Register
   2) and information about the fault, formatted as described in
   the PF_* macros in exception.h, is in F's error_code member.  The
   example code here shows how to parse that information.  You
   can find more information about both of these in the
   description of "Interrupt 14--Page Fault Exception (#PF)" in
   [IA32-v3a] section 5.15 "Exception and Interrupt Reference". */
/* Page fault 핸들러. virtual memory를 구현하려면 채워야 하는 스켈레톤이다.
   project 2의 일부 솔루션도 이 코드를 수정해야 할 수 있다.

   진입 시점에는 fault가 발생한 주소가 CR2(Control Register 2)에 있고, fault에
   대한 정보는 exception.h의 PF_* 매크로 설명 형식으로 F의 error_code 멤버에
   들어 있다. 아래 예시 코드는 이 정보를 파싱하는 방법을 보여준다. 이 둘에 대한
   자세한 내용은 [IA32-v3a] section 5.15 "Exception and Interrupt Reference"의
   "Interrupt 14--Page Fault Exception (#PF)" 설명을 참고한다. */
static void
page_fault (struct intr_frame *f) {
	bool not_present;  /* True: not-present page, false: writing r/o page. */
	/* True: not-present page, false: read-only 페이지에 쓰기. */
	bool write;        /* True: access was write, false: access was read. */
	/* True: write 접근, false: read 접근. */
	bool user;         /* True: access by user, false: access by kernel. */
	/* True: 유저 접근, false: 커널 접근. */
	void *fault_addr;  /* Fault address. */
	/* fault가 발생한 주소. */

	/* Obtain faulting address, the virtual address that was
	   accessed to cause the fault.  It may point to code or to
	   data.  It is not necessarily the address of the instruction
	   that caused the fault (that's f->rip). */
	/* fault를 일으킨 주소, 즉 접근 때문에 fault가 발생한 가상 주소를 얻는다.
	   이 주소는 코드나 데이터를 가리킬 수 있다. fault를 일으킨 instruction의
	   주소일 필요는 없다. instruction 주소는 f->rip이다. */

	fault_addr = (void *) rcr2();

	/* Turn interrupts back on (they were only off so that we could
	   be assured of reading CR2 before it changed). */
	/* 인터럽트를 다시 켠다. CR2가 바뀌기 전에 확실히 읽기 위해서만 꺼 두었다. */
	intr_enable ();


	/* Determine cause. */
	/* 원인을 판단한다. */
	not_present = (f->error_code & PF_P) == 0;
	write = (f->error_code & PF_W) != 0;
	user = (f->error_code & PF_U) != 0;

#ifdef VM
	/* For project 3 and later. */
	/* project 3 이후용. */
	if (vm_try_handle_fault (f, fault_addr, user, write, not_present))
		return;
#endif

	/* Count page faults. */
	/* page fault를 카운트한다. */
	page_fault_cnt++;

	/* If the fault is true fault, show info and exit. */
	/* 진짜 fault라면 정보를 출력하고 종료한다. */
	printf ("Page fault at %p: %s error %s page in %s context.\n",
			fault_addr,
			not_present ? "not present" : "rights violation",
			write ? "writing" : "reading",
			user ? "user" : "kernel");
	kill (f);
}
