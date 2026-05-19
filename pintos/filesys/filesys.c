#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "filesys/fat.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "devices/disk.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/malloc.h"

#define PATH_MAX 1024
#define SYMLINK_MAX_DEPTH 8

/* The disk that contains the file system. */
/* 파일 시스템을 담고 있는 디스크. */
struct disk *filesys_disk;

struct lock filesys_lock;

static void do_format (void);
#ifdef EFILESYS
static disk_sector_t root_dir_sector (void);
static struct dir *dir_for_path_start (struct dir *, const char *);
static bool resolve_path_from (struct dir *, const char *, bool, int,
		struct inode **);
static bool resolve_path (const char *, bool, struct inode **);
static bool resolve_parent (const char *, struct dir **, char *);
static char *read_symlink_target (struct inode *);
static bool is_special_dir_name (const char *);
static bool allocate_inode_sector (disk_sector_t *);
static void remove_created_inode (disk_sector_t);
#endif

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
#ifdef EFILESYS
	struct dir *dir = NULL;
	char basename[NAME_MAX + 1];
	disk_sector_t inode_sector = 0;
	bool success = false;
	struct inode *existing = NULL;

	if (!resolve_parent (name, &dir, basename))
		return false;
	if (is_special_dir_name (basename))
		goto done;
	if (dir_lookup (dir, basename, &existing)) {
		inode_close (existing);
		goto done;
	}
	if (!allocate_inode_sector (&inode_sector))
		goto done;

	success = inode_create_typed (inode_sector, initial_size, INODE_FILE)
	          && dir_add (dir, basename, inode_sector);
	if (!success)
		remove_created_inode (inode_sector);

done:
	dir_close (dir);
	return success;
#else
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
#endif
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
#ifdef EFILESYS
	struct inode *inode = NULL;

	if (!resolve_path (name, true, &inode))
		return NULL;
	return file_open (inode);
#else
	struct dir *dir = dir_open_root ();
	struct inode *inode = NULL;

	if (dir != NULL)
		dir_lookup (dir, name, &inode);
	dir_close (dir);

	return file_open (inode);
#endif
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
#ifdef EFILESYS
	struct dir *dir = NULL;
	char basename[NAME_MAX + 1];
	struct inode *inode = NULL;
	bool success = false;

	if (!resolve_parent (name, &dir, basename))
		return false;
	if (is_special_dir_name (basename))
		goto done;
	if (!dir_lookup (dir, basename, &inode))
		goto done;
	if (inode_get_inumber (inode) == root_dir_sector ())
		goto done;
	if (inode_is_dir (inode)) {
		struct dir *target = dir_open (inode_reopen (inode));
		if (target == NULL)
			goto done;
		success = dir_is_empty (target) && inode_open_count (inode) <= 2;
		dir_close (target);
		if (!success) {
			success = false;
			goto done;
		}
	}

	success = dir_remove (dir, basename);

done:
	inode_close (inode);
	dir_close (dir);
	return success;
#else
	struct dir *dir = dir_open_root ();
	bool success = dir != NULL && dir_remove (dir, name);
	dir_close (dir);

	return success;
#endif
}

#ifdef EFILESYS
bool
filesys_chdir (const char *name) {
	struct inode *inode = NULL;
	struct dir *dir;

	if (!resolve_path (name, true, &inode))
		return false;
	if (!inode_is_dir (inode)) {
		inode_close (inode);
		return false;
	}

	dir = dir_open (inode);
	if (dir == NULL)
		return false;

	if (thread_current ()->cwd != NULL)
		dir_close (thread_current ()->cwd);
	thread_current ()->cwd = dir;
	return true;
}

bool
filesys_mkdir (const char *name) {
	struct dir *dir = NULL;
	char basename[NAME_MAX + 1];
	disk_sector_t inode_sector = 0;
	bool success = false;
	struct inode *existing = NULL;

	if (!resolve_parent (name, &dir, basename))
		return false;
	if (is_special_dir_name (basename))
		goto done;
	if (dir_lookup (dir, basename, &existing)) {
		inode_close (existing);
		goto done;
	}
	if (!allocate_inode_sector (&inode_sector))
		goto done;

	success = dir_create_with_parent (inode_sector,
			inode_get_inumber (dir_get_inode (dir)))
	          && dir_add (dir, basename, inode_sector);
	if (!success)
		remove_created_inode (inode_sector);

done:
	dir_close (dir);
	return success;
}

int
filesys_symlink (const char *target, const char *linkpath) {
	struct dir *dir = NULL;
	char basename[NAME_MAX + 1];
	disk_sector_t inode_sector = 0;
	struct inode *inode = NULL;
	struct inode *existing = NULL;
	size_t target_len;
	bool success = false;

	if (target == NULL || target[0] == '\0')
		return -1;
	if (!resolve_parent (linkpath, &dir, basename))
		return -1;
	if (is_special_dir_name (basename))
		goto done;
	if (dir_lookup (dir, basename, &existing)) {
		inode_close (existing);
		goto done;
	}
	if (!allocate_inode_sector (&inode_sector))
		goto done;

	target_len = strlen (target) + 1;
	if (!inode_create_typed (inode_sector, target_len, INODE_SYMLINK)) {
		remove_created_inode (inode_sector);
		goto done;
	}
	inode = inode_open (inode_sector);
	success = inode != NULL
	          && inode_write_at (inode, target, target_len, 0) == (off_t) target_len
	          && dir_add (dir, basename, inode_sector);
	if (!success)
		remove_created_inode (inode_sector);

done:
	inode_close (inode);
	dir_close (dir);
	return success ? 0 : -1;
}
#else
bool
filesys_chdir (const char *name UNUSED) {
	return false;
}

bool
filesys_mkdir (const char *name UNUSED) {
	return false;
}

int
filesys_symlink (const char *target UNUSED, const char *linkpath UNUSED) {
	return -1;
}
#endif

#ifdef EFILESYS
static disk_sector_t
root_dir_sector (void) {
	return cluster_to_sector (ROOT_DIR_CLUSTER);
}

static struct dir *
dir_for_path_start (struct dir *base, const char *path) {
	if (path[0] == '/')
		return dir_open_root ();
	if (base != NULL)
		return dir_reopen (base);
	if (thread_current ()->cwd != NULL)
		return dir_reopen (thread_current ()->cwd);
	return dir_open_root ();
}

static char *
read_symlink_target (struct inode *inode) {
	off_t length = inode_length (inode);
	char *target;

	if (length <= 0 || length > PATH_MAX)
		return NULL;

	target = malloc (length + 1);
	if (target == NULL)
		return NULL;
	if (inode_read_at (inode, target, length, 0) != length) {
		free (target);
		return NULL;
	}
	target[length] = '\0';
	return target;
}

static bool
resolve_path_from (struct dir *base, const char *path, bool follow_final,
		int depth, struct inode **inodep) {
	struct dir *dir;
	const char *p;

	if (path == NULL || inodep == NULL || depth > SYMLINK_MAX_DEPTH)
		return false;

	dir = dir_for_path_start (base, path);
	if (dir == NULL)
		return false;

	p = path;
	while (*p == '/')
		p++;
	if (*p == '\0') {
		*inodep = inode_reopen (dir_get_inode (dir));
		dir_close (dir);
		return *inodep != NULL;
	}

	for (;;) {
		char name[NAME_MAX + 1];
		const char *start;
		size_t len;
		bool last;
		struct inode *inode = NULL;

		while (*p == '/')
			p++;
		if (*p == '\0') {
			*inodep = inode_reopen (dir_get_inode (dir));
			dir_close (dir);
			return *inodep != NULL;
		}

		start = p;
		while (*p != '\0' && *p != '/')
			p++;
		len = p - start;
		if (len == 0 || len > NAME_MAX) {
			dir_close (dir);
			return false;
		}
		memcpy (name, start, len);
		name[len] = '\0';
		while (*p == '/')
			p++;
		last = *p == '\0';

		if (!dir_lookup (dir, name, &inode)) {
			dir_close (dir);
			return false;
		}

		if (inode_is_symlink (inode) && (!last || follow_final)) {
			char *target = read_symlink_target (inode);
			char *combined = NULL;
			struct dir *link_base = dir_reopen (dir);
			bool ok;

			inode_close (inode);
			dir_close (dir);
			if (target == NULL || link_base == NULL) {
				free (target);
				dir_close (link_base);
				return false;
			}
			if (!last) {
				size_t combined_len = strlen (target) + 1 + strlen (p) + 1;
				combined = malloc (combined_len);
				if (combined == NULL) {
					free (target);
					dir_close (link_base);
					return false;
				}
				snprintf (combined, combined_len, "%s/%s", target, p);
			}
			ok = resolve_path_from (link_base, last ? target : combined,
					follow_final, depth + 1, inodep);
			free (combined);
			free (target);
			dir_close (link_base);
			return ok;
		}

		if (last) {
			*inodep = inode;
			dir_close (dir);
			return true;
		}

		if (!inode_is_dir (inode)) {
			inode_close (inode);
			dir_close (dir);
			return false;
		}

		dir_close (dir);
		dir = dir_open (inode);
		if (dir == NULL)
			return false;
	}
}

static bool
resolve_path (const char *path, bool follow_final, struct inode **inodep) {
	if (path == NULL || path[0] == '\0')
		return false;
	return resolve_path_from (NULL, path, follow_final, 0, inodep);
}

static bool
resolve_parent (const char *path, struct dir **dirp, char *basename) {
	const char *end;
	const char *last;
	size_t len;
	char *parent_path = NULL;
	bool ok = false;

	if (path == NULL || path[0] == '\0' || dirp == NULL || basename == NULL)
		return false;

	end = path + strlen (path);
	while (end > path && end[-1] == '/')
		end--;
	if (end == path)
		return false;

	last = end;
	while (last > path && last[-1] != '/')
		last--;
	len = end - last;
	if (len == 0 || len > NAME_MAX)
		return false;

	memcpy (basename, last, len);
	basename[len] = '\0';

	if (last == path) {
		*dirp = dir_for_path_start (NULL, path);
		return *dirp != NULL;
	}

	len = last - path;
	parent_path = malloc (len + 1);
	if (parent_path == NULL)
		return false;
	memcpy (parent_path, path, len);
	parent_path[len] = '\0';

	struct inode *parent_inode = NULL;
	if (resolve_path (parent_path, true, &parent_inode)
			&& inode_is_dir (parent_inode)) {
		*dirp = dir_open (parent_inode);
		ok = *dirp != NULL;
	} else {
		inode_close (parent_inode);
	}

	free (parent_path);
	return ok;
}

static bool
is_special_dir_name (const char *name) {
	return !strcmp (name, ".") || !strcmp (name, "..");
}

static bool
allocate_inode_sector (disk_sector_t *sectorp) {
	cluster_t clst = fat_create_chain (0);
	if (clst == 0)
		return false;
	*sectorp = cluster_to_sector (clst);
	return true;
}

static void
remove_created_inode (disk_sector_t sector) {
	struct inode *inode = inode_open (sector);
	if (inode != NULL) {
		inode_remove (inode);
		inode_close (inode);
	} else {
		fat_remove_chain (sector_to_cluster (sector), 0);
	}
}
#endif

/* Formats the file system. */
/* 파일 시스템을 format한다. */
static void
do_format (void) {
	printf ("Formatting file system...");

#ifdef EFILESYS
	/* Create FAT and save it to the disk. */
	/* FAT를 만들고 디스크에 저장한다. */
	fat_create ();
	if (!dir_create_with_parent (root_dir_sector (), root_dir_sector ()))
		PANIC ("root directory creation failed");
	fat_close ();
#else
	free_map_create ();
	if (!dir_create (ROOT_DIR_SECTOR, 16))
		PANIC ("root directory creation failed");
	free_map_close ();
#endif

	printf ("done.\n");
}
