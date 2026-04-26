#include "tests/vm/qsort.h"
#include <stdbool.h>
#include <debug.h>
#include <random.h>

/* Picks a pivot for the quicksort from the SIZE bytes in BUF. */
/* BUF의 SIZE byte에서 quicksort용 pivot을 고른다. */
static unsigned char
pick_pivot (unsigned char *buf, size_t size) 
{
  ASSERT (size >= 1);
  return buf[random_ulong () % size];
}

/* Checks whether the SIZE bytes in ARRAY are divided into an
   initial LEFT_SIZE elements all less than PIVOT followed by
   SIZE - LEFT_SIZE elements all greater than or equal to
   PIVOT. */
/* ARRAY의 SIZE byte가 PIVOT보다 작은 initial LEFT_SIZE element와
   PIVOT 이상인 SIZE - LEFT_SIZE element로 나뉘어 있는지 확인한다. */
static bool
is_partitioned (const unsigned char *array, size_t size,
                unsigned char pivot, size_t left_size) 
{
  size_t i;
  
  for (i = 0; i < left_size; i++)
    if (array[i] >= pivot)
      return false;

  for (; i < size; i++)
    if (array[i] < pivot)
      return false;

  return true;
}

/* Swaps the bytes at *A and *B. */
/* *A와 *B의 byte를 swap한다. */
static void
swap (unsigned char *a, unsigned char *b) 
{
  unsigned char t = *a;
  *a = *b;
  *b = t;
}

/* Partitions ARRAY in-place in an initial run of bytes all less
   than PIVOT, followed by a run of bytes all greater than or
   equal to PIVOT.  Returns the length of the initial run. */
/* ARRAY를 PIVOT보다 작은 byte들의 initial run과 PIVOT 이상인 byte들의 run으로
   in-place partition한다. initial run의 길이를 반환한다. */
static size_t
partition (unsigned char *array, size_t size, int pivot) 
{
  size_t left_size = size;
  unsigned char *first = array;
  unsigned char *last = first + left_size;

  for (;;)
    {
      /* Move FIRST forward to point to first element greater than
         PIVOT. */
      /* FIRST를 앞으로 이동시켜 PIVOT보다 큰 첫 element를 가리키게 한다. */
      for (;;)
        {
          if (first == last)
            {
              ASSERT (is_partitioned (array, size, pivot, left_size));
              return left_size;
            }
          else if (*first >= pivot)
            break;

          first++;
        }
      left_size--;

      /* Move LAST backward to point to last element no bigger
         than PIVOT. */
      /* LAST를 뒤로 이동시켜 PIVOT보다 크지 않은 마지막 element를 가리키게 한다. */
      for (;;)
        {
          last--;

          if (first == last)
            {
              ASSERT (is_partitioned (array, size, pivot, left_size));
              return left_size;
            }
          else if (*last < pivot)
            break;
          else
            left_size--;
        }

      /* By swapping FIRST and LAST we extend the starting and
         ending sequences that pass and fail, respectively,
         PREDICATE. */
      /* FIRST와 LAST를 swap하여 PREDICATE를 각각 pass/fail하는
         시작 sequence와 끝 sequence를 확장한다. */
      swap (first, last);
      first++;
    }
}

/* Returns true if the SIZE bytes in BUF are in nondecreasing
   order, false otherwise. */
/* BUF의 SIZE byte가 nondecreasing order이면 true를, 아니면 false를 반환한다. */
static bool
is_sorted (const unsigned char *buf, size_t size) 
{
  size_t i;

  for (i = 1; i < size; i++)
    if (buf[i - 1] > buf[i])
      return false;

  return true;
}

/* Sorts the SIZE bytes in BUF into nondecreasing order, using
   the quick-sort algorithm. */
/* quick-sort algorithm을 사용해 BUF의 SIZE byte를 nondecreasing order로 sort한다. */
void
qsort_bytes (unsigned char *buf, size_t size) 
{
  if (!is_sorted (buf, size)) 
    {
      int pivot = pick_pivot (buf, size);

      unsigned char *left_half = buf;
      size_t left_size = partition (buf, size, pivot);
      unsigned char *right_half = left_half + left_size;
      size_t right_size = size - left_size;
  
      if (left_size <= right_size) 
        {
          qsort_bytes (left_half, left_size);
          qsort_bytes (right_half, right_size); 
        }
      else
        {
          qsort_bytes (right_half, right_size); 
          qsort_bytes (left_half, left_size);
        }
    } 
}
