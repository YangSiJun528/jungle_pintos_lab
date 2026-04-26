/* Writes out the content of a fairly large file in random order,
   then reads it back in random order to verify that it was
   written properly. */
/* 꽤 큰 file의 내용을 random order로 write한 뒤,
   random order로 다시 읽어서 올바르게 기록되었는지 검증한다. */

#define BLOCK_SIZE 512
#define TEST_SIZE (512 * 150)
#include "tests/filesys/base/random.inc"
