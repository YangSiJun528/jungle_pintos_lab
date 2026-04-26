/* Reads a 128 kB file into static data and "sorts" the bytes in
   it, using counting sort, a single-pass algorithm.  The sorted
   data is written back to the same file in-place. */
/* 128 kB file을 static data로 읽고, single-pass algorithm인 counting sort를 사용해
   그 안의 byte를 "sort"한다. 정렬된 data는 같은 file에 in-place로 다시 기록된다. */

#include <debug.h>
#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

unsigned char buf[128 * 1024];
size_t histogram[256];

int
main (int argc UNUSED, char *argv[]) 
{
  test_name = "child-sort";

  int handle;
  unsigned char *p;
  size_t size;
  size_t i;

  quiet = true;

  CHECK ((handle = open (argv[1])) > 1, "open \"%s\"", argv[1]);

  size = read (handle, buf, sizeof buf);
  for (i = 0; i < size; i++)
    histogram[buf[i]]++;
  p = buf;
  for (i = 0; i < sizeof histogram / sizeof *histogram; i++) 
    {
      size_t j = histogram[i];
      while (j-- > 0)
        *p++ = i;
    }
  seek (handle, 0);
  write (handle, buf, size);
  close (handle);
  
  return 123;
}
