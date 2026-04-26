/* Grows a file from 0 bytes to 5,678 bytes, 1,234 bytes at a
   time. */
/* file을 0 byte에서 5,678 byte까지 한 번에 1,234 byte씩 grow시킨다. */

#define TEST_SIZE 5678
#include "tests/filesys/extended/grow-seq.inc"
