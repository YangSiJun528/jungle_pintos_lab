/* inspect.c: Testing utility for VM. */
/* inspect.c: VM 테스트 유틸리티. */
/* DO NOT MODIFY THIS FILE. */
/* 이 파일은 수정하지 않는다. */

#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/mmu.h"
#include "vm/inspect.h"

static void
inspect (struct intr_frame *f) {
	const void *va = (const void *) f->R.rax;
	f->R.rax = PTE_ADDR (pml4_get_page (thread_current ()->pml4, va));
}

/* Tool for testing vm component. Calling this function via int 0x42.
 * Input:
 *   @RAX - Virtual address to inspect
 * Output:
 *   @RAX - Physical address that mmaped to input. */
/* vm component를 테스트하기 위한 툴. 이 함수는 int 0x42로 호출한다.
 * Input:
 *   @RAX - inspect할 가상 주소
 * Output:
 *   @RAX - input에 맵핑된 물리 주소 */
void
register_inspect_intr (void) {
	intr_register_int (0x42, 3, INTR_OFF, inspect, "Inspect Virtual Memory");
}
