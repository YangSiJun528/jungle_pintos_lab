#ifndef FILESYS_FILE_INTERNAL_H
#define FILESYS_FILE_INTERNAL_H

/* filesys 내부 전용 비잠금 file I/O 헬퍼.
 * bitmap.c, free-map.c 등 filesys 레이어 내부에서만 사용한다.
 * 외부 모듈(userprog 등)은 include하지 않는다. */

#include "filesys/off_t.h"

struct file;

off_t file_read_at_unlocked  (struct file *, void *,       off_t size, off_t file_ofs);
off_t file_write_at_unlocked (struct file *, const void *, off_t size, off_t file_ofs);

#endif /* filesys/file_internal.h */
