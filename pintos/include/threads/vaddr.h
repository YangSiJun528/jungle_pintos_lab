#ifndef THREADS_VADDR_H
#define THREADS_VADDR_H

#include <debug.h>
#include <stdint.h>
#include <stdbool.h>

#include "threads/loader.h"

/* Functions and macros for working with virtual addresses.
 *
 * See pte.h for functions and macros specifically for x86
 * hardware page tables. */
/* 가상 주소를 다루기 위한 함수와 매크로.
 *
 * x86 하드웨어 page table 전용 함수와 매크로는 pte.h를 참고한다. */

#define BITMASK(SHIFT, CNT) (((1ul << (CNT)) - 1) << (SHIFT))

/* Page offset (bits 0:12). */
/* 페이지 offset(bits 0:12). */
#define PGSHIFT 0                          /* Index of first offset bit. */
/* 첫 offset bit의 index. */
#define PGBITS  12                         /* Number of offset bits. */
/* offset bit 수. */
#define PGSIZE  (1 << PGBITS)              /* Bytes in a page. */
/* page 안의 byte 수. */
#define PGMASK  BITMASK(PGSHIFT, PGBITS)   /* Page offset bits (0:12). */
/* 페이지 offset bit(0:12). */

/* Offset within a page. */
/* page 안의 offset. */
#define pg_ofs(va) ((uint64_t) (va) & PGMASK)

#define pg_no(va) ((uint64_t) (va) >> PGBITS)

/* Round up to nearest page boundary. */
/* 가장 가까운 page boundary로 올림한다. */
#define pg_round_up(va) ((void *) (((uint64_t) (va) + PGSIZE - 1) & ~PGMASK))

/* Round down to nearest page boundary. */
/* 가장 가까운 page boundary로 내림한다. */
#define pg_round_down(va) (void *) ((uint64_t) (va) & ~PGMASK)

/* Kernel virtual address start */
/* 커널 가상 주소 시작 지점. */
#define KERN_BASE LOADER_KERN_BASE

/* User stack start */
/* 유저 스택 시작 지점. */
#define USER_STACK 0x47480000

/* Returns true if VADDR is a user virtual address. */
/* VADDR이 유저 가상 주소이면 true를 리턴한다. */
#define is_user_vaddr(vaddr) (!is_kernel_vaddr((vaddr)))

/* Returns true if VADDR is a kernel virtual address. */
/* VADDR이 커널 가상 주소이면 true를 리턴한다. */
#define is_kernel_vaddr(vaddr) ((uint64_t)(vaddr) >= KERN_BASE)

// FIXME: add checking
// FIXME: checking을 추가한다.
/* Returns kernel virtual address at which physical address PADDR
 *  is mapped. */
/* 물리 주소 PADDR이 맵핑된 커널 가상 주소를 리턴한다. */
#define ptov(paddr) ((void *) (((uint64_t) paddr) + KERN_BASE))

/* Returns physical address at which kernel virtual address VADDR
 * is mapped. */
/* 커널 가상 주소 VADDR이 맵핑된 물리 주소를 리턴한다. */
#define vtop(vaddr) \
({ \
	ASSERT(is_kernel_vaddr(vaddr)); \
	((uint64_t) (vaddr) - (uint64_t) KERN_BASE);\
})

#endif /* threads/vaddr.h */
