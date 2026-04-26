/* Ensure that the executable of a running process cannot be
   modified, even in the presence of multiple children. */
/* 여러 child가 있어도 실행 중인 process의 executable은 수정될 수 없음을 확인한다. */

#define CHILD_CNT "5"
#include "tests/userprog/rox-child.inc"
