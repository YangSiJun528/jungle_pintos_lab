/* Writes out a fairly large file sequentially, one fixed-size
   block at a time, then reads it back to verify that it was
   written properly. */
/* 꽤 큰 file을 fixed-size block 단위로 순차적으로 write한 뒤,
   다시 읽어서 올바르게 기록되었는지 검증한다. */

#define TEST_SIZE 75678
#define BLOCK_SIZE 513
#include "tests/filesys/base/seq-block.inc"
