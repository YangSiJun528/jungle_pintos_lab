#include "filesys/directory.h"
#include <stdio.h>
#include <string.h>
#include <list.h>
#include "filesys/filesys.h"
#include "filesys/fat.h"
#include "filesys/inode.h"
#include "threads/malloc.h"

/* A directory. */
/* 디렉터리. */
struct dir {
	struct inode *inode;                /* Backing store. */
	/* 데이터가 저장되는 backing store. */
	off_t pos;                          /* Current position. */
	/* 현재 위치. */
};

/* A single directory entry. */
/* 단일 디렉터리 엔트리. */
struct dir_entry {
	disk_sector_t inode_sector;         /* Sector number of header. */
	/* 헤더의 sector 번호. */
	char name[NAME_MAX + 1];            /* Null terminated file name. */
	/* null로 끝나는 파일 이름. */
	bool in_use;                        /* In use or free? */
	/* 사용 중인지 free 상태인지. */
};

/* Creates a directory with space for ENTRY_CNT entries in the
 * given SECTOR.  Returns true if successful, false on failure. */
/* 주어진 SECTOR에 ENTRY_CNT개의 엔트리를 담을 공간이 있는 디렉터리를 만든다.
 * 성공하면 true, 실패하면 false를 리턴한다. */
bool
dir_create (disk_sector_t sector, size_t entry_cnt) {
#ifdef EFILESYS
	(void) entry_cnt;
	return dir_create_with_parent (sector, sector);
#else
	return inode_create (sector, entry_cnt * sizeof (struct dir_entry));
#endif
}

bool
dir_create_with_parent (disk_sector_t sector, disk_sector_t parent_sector) {
	struct dir *dir;
	bool success;

	if (!inode_create_typed (sector, 0, INODE_DIR))
		return false;

	dir = dir_open (inode_open (sector));
	if (dir == NULL)
		return false;

	success = dir_add (dir, ".", sector)
	          && dir_add (dir, "..", parent_sector);
	dir_close (dir);
	return success;
}

/* Opens and returns the directory for the given INODE, of which
 * it takes ownership.  Returns a null pointer on failure. */
/* 주어진 INODE에 대한 디렉터리를 열고 리턴하며, 그 inode의 ownership을 가져간다.
 * 실패하면 null pointer를 리턴한다. */
struct dir *
dir_open (struct inode *inode) {
	struct dir *dir = calloc (1, sizeof *dir);
	if (inode != NULL && dir != NULL) {
		dir->inode = inode;
		dir->pos = 0;
		return dir;
	} else {
		inode_close (inode);
		free (dir);
		return NULL;
	}
}

/* Opens the root directory and returns a directory for it.
 * Return true if successful, false on failure. */
/* root 디렉터리를 열고 그 디렉터리를 리턴한다.
 * 성공하면 true, 실패하면 false를 리턴한다. */
struct dir *
dir_open_root (void) {
#ifdef EFILESYS
	return dir_open (inode_open (cluster_to_sector (ROOT_DIR_CLUSTER)));
#else
	return dir_open (inode_open (ROOT_DIR_SECTOR));
#endif
}

/* Opens and returns a new directory for the same inode as DIR.
 * Returns a null pointer on failure. */
/* DIR과 같은 inode에 대한 새 디렉터리를 열고 리턴한다.
 * 실패하면 null pointer를 리턴한다. */
struct dir *
dir_reopen (struct dir *dir) {
	return dir_open (inode_reopen (dir->inode));
}

/* Destroys DIR and frees associated resources. */
/* DIR을 파괴하고 관련 리소스를 해제한다. */
void
dir_close (struct dir *dir) {
	if (dir != NULL) {
		inode_close (dir->inode);
		free (dir);
	}
}

/* Returns the inode encapsulated by DIR. */
/* DIR이 감싼 inode를 리턴한다. */
struct inode *
dir_get_inode (struct dir *dir) {
	return dir->inode;
}

/* Searches DIR for a file with the given NAME.
 * If successful, returns true, sets *EP to the directory entry
 * if EP is non-null, and sets *OFSP to the byte offset of the
 * directory entry if OFSP is non-null.
 * otherwise, returns false and ignores EP and OFSP. */
/* DIR에서 주어진 NAME을 가진 파일을 검색한다.
 * 성공하면 true를 리턴하고, EP가 non-null이면 *EP를 directory entry로 설정하며,
 * OFSP가 non-null이면 *OFSP를 directory entry의 byte offset으로 설정한다.
 * 실패하면 false를 리턴하고 EP와 OFSP를 무시한다. */
static bool
lookup (const struct dir *dir, const char *name,
		struct dir_entry *ep, off_t *ofsp) {
	struct dir_entry e;
	size_t ofs;

	ASSERT (dir != NULL);
	ASSERT (name != NULL);

	for (ofs = 0; inode_read_at (dir->inode, &e, sizeof e, ofs) == sizeof e;
			ofs += sizeof e)
		if (e.in_use && !strcmp (name, e.name)) {
			if (ep != NULL)
				*ep = e;
			if (ofsp != NULL)
				*ofsp = ofs;
			return true;
		}
	return false;
}

/* Searches DIR for a file with the given NAME
 * and returns true if one exists, false otherwise.
 * On success, sets *INODE to an inode for the file, otherwise to
 * a null pointer.  The caller must close *INODE. */
/* DIR에서 주어진 NAME을 가진 파일을 검색하고, 존재하면 true 아니면 false를
 * 리턴한다.
 * 성공하면 *INODE를 해당 파일의 inode로 설정하고, 실패하면 null pointer로
 * 설정한다. 호출자는 *INODE를 닫아야 한다. */
bool
dir_lookup (const struct dir *dir, const char *name,
		struct inode **inode) {
	struct dir_entry e;

	ASSERT (dir != NULL);
	ASSERT (name != NULL);

	if (lookup (dir, name, &e, NULL))
		*inode = inode_open (e.inode_sector);
	else
		*inode = NULL;

	return *inode != NULL;
}

/* Adds a file named NAME to DIR, which must not already contain a
 * file by that name.  The file's inode is in sector
 * INODE_SECTOR.
 * Returns true if successful, false on failure.
 * Fails if NAME is invalid (i.e. too long) or a disk or memory
 * error occurs. */
/* NAME이라는 파일을 DIR에 추가한다. DIR에는 같은 이름의 파일이 이미 있으면
 * 안 된다. 파일의 inode는 INODE_SECTOR sector에 있다.
 * 성공하면 true, 실패하면 false를 리턴한다.
 * NAME이 올바르지 않거나, 즉 너무 길거나, 디스크 또는 메모리 에러가 발생하면
 * 실패한다. */
bool
dir_add (struct dir *dir, const char *name, disk_sector_t inode_sector) {
	struct dir_entry e;
	off_t ofs;
	bool success = false;

	ASSERT (dir != NULL);
	ASSERT (name != NULL);

	/* Check NAME for validity. */
	/* NAME이 유효한지 검사한다. */
	if (*name == '\0' || strlen (name) > NAME_MAX)
		return false;

	/* Check that NAME is not in use. */
	/* NAME이 사용 중이 아닌지 검사한다. */
	if (lookup (dir, name, NULL, NULL))
		goto done;

	/* Set OFS to offset of free slot.
	 * If there are no free slots, then it will be set to the
	 * current end-of-file.

	 * inode_read_at() will only return a short read at end of file.
	 * Otherwise, we'd need to verify that we didn't get a short
	 * read due to something intermittent such as low memory. */
	/* OFS를 free slot의 offset으로 설정한다.
	 * free slot이 없다면 현재 end-of-file로 설정된다.

	 * inode_read_at()은 end of file에서만 short read를 리턴한다.
	 * 그렇지 않다면 low memory 같은 일시적 원인으로 short read가 발생한 것이
	 * 아닌지 검증해야 할 것이다. */
	for (ofs = 0; inode_read_at (dir->inode, &e, sizeof e, ofs) == sizeof e;
			ofs += sizeof e)
		if (!e.in_use)
			break;

	/* Write slot. */
	/* slot을 쓴다. */
	e.in_use = true;
	strlcpy (e.name, name, sizeof e.name);
	e.inode_sector = inode_sector;
	success = inode_write_at (dir->inode, &e, sizeof e, ofs) == sizeof e;

done:
	return success;
}

/* Removes any entry for NAME in DIR.
 * Returns true if successful, false on failure,
 * which occurs only if there is no file with the given NAME. */
/* DIR에서 NAME에 대한 엔트리를 제거한다.
 * 성공하면 true, 실패하면 false를 리턴한다.
 * 실패는 주어진 NAME을 가진 파일이 없을 때만 발생한다. */
bool
dir_remove (struct dir *dir, const char *name) {
	struct dir_entry e;
	struct inode *inode = NULL;
	bool success = false;
	off_t ofs;

	ASSERT (dir != NULL);
	ASSERT (name != NULL);

	/* Find directory entry. */
	/* 디렉터리 엔트리를 찾는다. */
	if (!lookup (dir, name, &e, &ofs))
		goto done;

	/* Open inode. */
	/* inode를 연다. */
	inode = inode_open (e.inode_sector);
	if (inode == NULL)
		goto done;

	/* Erase directory entry. */
	/* 디렉터리 엔트리를 지운다. */
	e.in_use = false;
	if (inode_write_at (dir->inode, &e, sizeof e, ofs) != sizeof e)
		goto done;

	/* Remove inode. */
	/* inode를 제거한다. */
	inode_remove (inode);
	success = true;

done:
	inode_close (inode);
	return success;
}

/* Reads the next directory entry in DIR and stores the name in
 * NAME.  Returns true if successful, false if the directory
 * contains no more entries. */
/* DIR에서 다음 디렉터리 엔트리를 읽고 이름을 NAME에 저장한다.
 * 성공하면 true를 리턴하고, 디렉터리에 더 이상 엔트리가 없으면 false를 리턴한다. */
bool
dir_readdir (struct dir *dir, char name[NAME_MAX + 1]) {
	struct dir_entry e;

	while (inode_read_at (dir->inode, &e, sizeof e, dir->pos) == sizeof e) {
		dir->pos += sizeof e;
		if (e.in_use) {
			if (!strcmp (e.name, ".") || !strcmp (e.name, ".."))
				continue;
			strlcpy (name, e.name, NAME_MAX + 1);
			return true;
		}
	}
	return false;
}

bool
dir_is_empty (struct dir *dir) {
	struct dir_entry e;
	off_t ofs;

	ASSERT (dir != NULL);
	for (ofs = 0; inode_read_at (dir->inode, &e, sizeof e, ofs) == sizeof e;
			ofs += sizeof e) {
		if (e.in_use && strcmp (e.name, ".") && strcmp (e.name, ".."))
			return false;
	}
	return true;
}
