/* Writes out the contents of a fairly large file all at once,
   and then reads it back to make sure that it was written
   properly. */
/* 꽤 큰 file의 내용을 한 번에 write한 뒤,
   다시 읽어서 올바르게 기록되었는지 확인한다. */

#define TEST_SIZE 75678
#include "tests/filesys/base/full.inc"
