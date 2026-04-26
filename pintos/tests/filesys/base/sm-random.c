/* Writes out the content of a fairly small file in random order,
   then reads it back in random order to verify that it was
   written properly. */
/* 꽤 작은 file의 내용을 random order로 write한 뒤,
   random order로 다시 읽어서 올바르게 기록되었는지 검증한다. */

#define BLOCK_SIZE 13
#define TEST_SIZE (13 * 123)
#include "tests/filesys/base/random.inc"
