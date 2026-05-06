#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "devices/disk.h"
#include "threads/synch.h"

/* The disk that contains the file system. */
/* 파일 시스템을 담고 있는 디스크. */
struct disk *filesys_disk;

/* 한 번에 하나의 프로세스만 파일시스템 코드를 실행하도록 직렬화하는 락.
 * filesys 레이어 내부에서만 정의·사용한다. 외부 모듈은 직접 접근하지 않는다. */
struct lock filesys_lock;

static void do_format (void);

/* ── internal _unlocked helpers ──────────────────────────────
 * 아래 함수들은 filesys_lock이 이미 잡혀 있다고 가정한다.
 * ─────────────────────────────────────────────────────────── */

static bool
filesys_create_unlocked (const char *name, off_t initial_size) {
	disk_sector_t inode_sector = 0;
	struct dir *dir = dir_open_root ();
	bool success = (dir != NULL
			&& free_map_allocate (1, &inode_sector)
			&& inode_create (inode_sector, initial_size)
			&& dir_add (dir, name, inode_sector));
	if (!success && inode_sector != 0)
		free_map_release (inode_sector, 1);
	dir_close (dir);
	return success;
}

static struct file *
filesys_open_unlocked (const char *name) {
	struct dir *dir = dir_open_root ();
	struct inode *inode = NULL;
	if (dir != NULL)
		dir_lookup (dir, name, &inode);
	dir_close (dir);
	return file_open (inode);
}

static bool
filesys_remove_unlocked (const char *name) {
	struct dir *dir = dir_open_root ();
	bool success = dir != NULL && dir_remove (dir, name);
	dir_close (dir);
	return success;
}

/* ── public API ──────────────────────────────────────────────
 * 아래 함수들은 filesys_lock을 잡고 _unlocked 헬퍼를 호출한다.
 * ─────────────────────────────────────────────────────────── */

/* Initializes the file system module.
 * If FORMAT is true, reformats the file system. */
/* 파일 시스템 모듈을 초기화한다.
 * FORMAT이 true이면 파일 시스템을 다시 format한다. */
void
filesys_init (bool format) {
	filesys_disk = disk_get (0, 1);
	if (filesys_disk == NULL)
		PANIC ("hd0:1 (hdb) not present, file system initialization failed");

	lock_init (&filesys_lock);
	inode_init ();

#ifdef EFILESYS
	fat_init ();

	if (format)
		do_format ();

	fat_open ();
#else
	/* Original FS */
	/* 원래 FS. */
	free_map_init ();

	if (format)
		do_format ();

	free_map_open ();
#endif
}

/* Shuts down the file system module, writing any unwritten data
 * to disk. */
/* 파일 시스템 모듈을 종료하면서 아직 쓰지 않은 데이터를 디스크에 쓴다. */
void
filesys_done (void) {
	/* Original FS */
	/* 원래 FS. */
#ifdef EFILESYS
	fat_close ();
#else
	free_map_close ();
#endif
}

/* Creates a file named NAME with the given INITIAL_SIZE.
 * Returns true if successful, false otherwise.
 * Fails if a file named NAME already exists,
 * or if internal memory allocation fails. */
/* 주어진 INITIAL_SIZE로 NAME이라는 파일을 만든다.
 * 성공하면 true, 아니면 false를 리턴한다.
 * NAME이라는 파일이 이미 있거나 내부 메모리 할당이 실패하면 실패한다. */
bool
filesys_create (const char *name, off_t initial_size) {
	lock_acquire (&filesys_lock);
	bool result = filesys_create_unlocked (name, initial_size);
	lock_release (&filesys_lock);
	return result;
}

/* Opens the file with the given NAME.
 * Returns the new file if successful or a null pointer
 * otherwise.
 * Fails if no file named NAME exists,
 * or if an internal memory allocation fails. */
/* 주어진 NAME의 파일을 연다.
 * 성공하면 새 파일을 리턴하고, 아니면 null pointer를 리턴한다.
 * NAME이라는 파일이 없거나 내부 메모리 할당이 실패하면 실패한다. */
struct file *
filesys_open (const char *name) {
	lock_acquire (&filesys_lock);
	struct file *result = filesys_open_unlocked (name);
	lock_release (&filesys_lock);
	return result;
}

/* Deletes the file named NAME.
 * Returns true if successful, false on failure.
 * Fails if no file named NAME exists,
 * or if an internal memory allocation fails. */
/* NAME이라는 파일을 삭제한다.
 * 성공하면 true, 실패하면 false를 리턴한다.
 * NAME이라는 파일이 없거나 내부 메모리 할당이 실패하면 실패한다. */
bool
filesys_remove (const char *name) {
	lock_acquire (&filesys_lock);
	bool result = filesys_remove_unlocked (name);
	lock_release (&filesys_lock);
	return result;
}

/* Formats the file system. */
/* 파일 시스템을 format한다. */
static void
do_format (void) {
	printf ("Formatting file system...");

#ifdef EFILESYS
	/* Create FAT and save it to the disk. */
	/* FAT를 만들고 디스크에 저장한다. */
	fat_create ();
	fat_close ();
#else
	free_map_create ();
	if (!dir_create (ROOT_DIR_SECTOR, 16))
		PANIC ("root directory creation failed");
	free_map_close ();
#endif

	printf ("done.\n");
}
