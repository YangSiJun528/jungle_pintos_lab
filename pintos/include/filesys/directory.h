#ifndef FILESYS_DIRECTORY_H
#define FILESYS_DIRECTORY_H

#include <stdbool.h>
#include <stddef.h>
#include "devices/disk.h"

/* Maximum length of a file name component.
 * This is the traditional UNIX maximum length.
 * After directories are implemented, this maximum length may be
 * retained, but much longer full path names must be allowed. */
/* 파일 이름 component의 최대 길이.
 * 전통적인 UNIX 최대 길이이다.
 * 디렉터리를 구현한 뒤에도 이 최대 길이는 유지할 수 있지만,
 * 훨씬 긴 full path name은 허용해야 한다. */
#define NAME_MAX 14

struct inode;

/* Opening and closing directories. */
/* 디렉터리 열기와 닫기. */
bool dir_create (disk_sector_t sector, size_t entry_cnt);
bool dir_create_with_parent (disk_sector_t sector, disk_sector_t parent_sector);
struct dir *dir_open (struct inode *);
struct dir *dir_open_root (void);
struct dir *dir_reopen (struct dir *);
void dir_close (struct dir *);
struct inode *dir_get_inode (struct dir *);

/* Reading and writing. */
/* 읽기와 쓰기. */
bool dir_lookup (const struct dir *, const char *name, struct inode **);
bool dir_add (struct dir *, const char *name, disk_sector_t);
bool dir_remove (struct dir *, const char *name);
bool dir_readdir (struct dir *, char name[NAME_MAX + 1]);
bool dir_is_empty (struct dir *);

#endif /* filesys/directory.h */
