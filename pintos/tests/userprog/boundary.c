/* Utility function for tests that try to break system calls by
   passing them data that crosses from one virtual page to
   another. */
/* 한 virtual page에서 다른 virtual page로 넘어가는 data를 넘겨 system call을 깨뜨리려는
   테스트를 위한 utility function. */

#include <inttypes.h>
#include <round.h>
#include <string.h>
#include "tests/userprog/boundary.h"

static char dst[8192];

/* Returns the beginning of a page.  There are at least 2048
   modifiable bytes on either side of the pointer returned. */
/* page의 시작 지점을 반환한다. 반환된 pointer의 양쪽에는 수정 가능한 byte가 최소
   2048개씩 있다. */
void *
get_boundary_area (void) 
{
  char *p = (char *) ROUND_UP ((uintptr_t) dst, 4096);
  if (p - dst < 2048)
    p += 4096;
  return p;
}

/* Returns a copy of SRC split across the boundary between two
   pages. */
/* 두 page 사이의 boundary에 걸치도록 나뉜 SRC의 사본을 반환한다. */
char *
copy_string_across_boundary (const char *src) 
{
  char *p = get_boundary_area ();
  p -= strlen (src) < 4096 ? strlen (src) / 2 : 4096;
  strlcpy (p, src, 4096);
  return p;
}

