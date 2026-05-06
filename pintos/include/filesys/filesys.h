#ifndef FILESYS_FILESYS_H
#define FILESYS_FILESYS_H

#include <stdbool.h>
#include "filesys/off_t.h"
#include "threads/synch.h"

/* Sectors of system file inodes. */
/* 시스템 파일 inode의 sector들. */
#define FREE_MAP_SECTOR 0       /* Free map file inode sector. */
/* free map 파일 inode sector. */
#define ROOT_DIR_SECTOR 1       /* Root directory file inode sector. */
/* root 디렉터리 파일 inode sector. */

/* Disk used for file system. */
/* 파일 시스템에서 사용하는 디스크. */
extern struct disk *filesys_disk;

/* 한 번에 하나의 프로세스만 파일시스템 코드를 실행하도록 직렬화하는 락. */
extern struct lock filesys_lock;

void filesys_init (bool format);
void filesys_done (void);
bool filesys_create (const char *name, off_t initial_size);
struct file *filesys_open (const char *name);
bool filesys_remove (const char *name);

#endif /* filesys/filesys.h */
