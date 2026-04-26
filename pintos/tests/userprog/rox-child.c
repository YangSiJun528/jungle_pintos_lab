/* Ensure that the executable of a running process cannot be
   modified, even by a child process. */
/* 실행 중인 process의 executable은 child process에 의해서도 수정될 수 없음을
   확인한다. */

#define CHILD_CNT "1"
#include "tests/userprog/rox-child.inc"
