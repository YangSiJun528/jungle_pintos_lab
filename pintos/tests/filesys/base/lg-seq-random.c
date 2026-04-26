/* Writes out a fairly large file sequentially, one random-sized
   block at a time, then reads it back to verify that it was
   written properly. */
/* 꽤 큰 file을 random-sized block 단위로 순차적으로 write한 뒤,
   다시 읽어서 올바르게 기록되었는지 검증한다. */

#define TEST_SIZE 75678
#include "tests/filesys/base/seq-random.inc"
