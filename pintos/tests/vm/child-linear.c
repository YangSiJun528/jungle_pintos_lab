/* Child process of page-parallel.
   Encrypts 1 MB of zeros, then decrypts it, and ensures that
   the zeros are back. */
/* page-parallel의 child process.
   1 MB의 zero를 encrypt한 뒤 decrypt하고, zero가 다시 돌아왔는지 확인한다. */

#include <string.h>
#include "tests/arc4.h"
#include "tests/lib.h"
#include "tests/main.h"

#define SIZE (1024 * 1024)
static char buf[SIZE];

int
main (int argc, char *argv[])
{
  test_name = "child-linear";

  const char *key = argv[argc - 1];
  struct arc4 arc4;
  size_t i;

  /* Encrypt zeros. */
  /* zero를 encrypt한다. */
  arc4_init (&arc4, key, strlen (key));
  arc4_crypt (&arc4, buf, SIZE);

  /* Decrypt back to zeros. */
  /* 다시 zero로 decrypt한다. */
  arc4_init (&arc4, key, strlen (key));
  arc4_crypt (&arc4, buf, SIZE);

  /* Check that it's all zeros. */
  /* 모두 zero인지 확인한다. */
  for (i = 0; i < SIZE; i++)
    if (buf[i] != '\0')
      fail ("byte %zu != 0", i);

  return 0x42;
}
