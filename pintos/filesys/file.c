#include "filesys/file.h"
#include <debug.h>
#include "filesys/inode.h"
#include "threads/malloc.h"
#include "threads/synch.h"

/* filesys_lock은 filesys.c에서 정의된다. */
extern struct lock filesys_lock;

/* An open file. */
/* 열린 파일. */
struct file {
	struct inode *inode;        /* File's inode. */
	/* 파일의 inode. */
	off_t pos;                  /* Current position. */
	/* 현재 위치. */
	bool deny_write;            /* Has file_deny_write() been called? */
	/* file_deny_write()가 호출되었는지 여부. */
};

/* ── internal _unlocked helpers ──────────────────────────────
 * 아래 함수들은 filesys_lock이 이미 잡혀 있다고 가정한다.
 * file.c 내부 또는 filesys 내부 레이어에서만 호출한다.
 * ─────────────────────────────────────────────────────────── */

static void
file_allow_write_unlocked (struct file *file) {
	ASSERT (file != NULL);
	if (file->deny_write) {
		file->deny_write = false;
		inode_allow_write (file->inode);
	}
}

static void
file_deny_write_unlocked (struct file *file) {
	ASSERT (file != NULL);
	if (!file->deny_write) {
		file->deny_write = true;
		inode_deny_write (file->inode);
	}
}

static void
file_close_unlocked (struct file *file) {
	if (file != NULL) {
		file_allow_write_unlocked (file);
		inode_close (file->inode);
		free (file);
	}
}

static off_t
file_read_unlocked (struct file *file, void *buffer, off_t size) {
	off_t bytes_read = inode_read_at (file->inode, buffer, size, file->pos);
	file->pos += bytes_read;
	return bytes_read;
}

/* non-static: bitmap.c에서 _unlocked 경로로 사용한다. */
off_t
file_read_at_unlocked (struct file *file, void *buffer, off_t size,
		off_t file_ofs) {
	return inode_read_at (file->inode, buffer, size, file_ofs);
}

static off_t
file_write_unlocked (struct file *file, const void *buffer, off_t size) {
	off_t bytes_written = inode_write_at (file->inode, buffer, size, file->pos);
	file->pos += bytes_written;
	return bytes_written;
}

/* non-static: bitmap.c에서 _unlocked 경로로 사용한다. */
off_t
file_write_at_unlocked (struct file *file, const void *buffer, off_t size,
		off_t file_ofs) {
	return inode_write_at (file->inode, buffer, size, file_ofs);
}

static off_t
file_length_unlocked (struct file *file) {
	ASSERT (file != NULL);
	return inode_length (file->inode);
}

static void
file_seek_unlocked (struct file *file, off_t new_pos) {
	ASSERT (file != NULL);
	ASSERT (new_pos >= 0);
	file->pos = new_pos;
}

static off_t
file_tell_unlocked (struct file *file) {
	ASSERT (file != NULL);
	return file->pos;
}

static struct file *
file_duplicate_unlocked (struct file *file) {
	struct file *nfile = file_open (inode_reopen (file->inode));
	if (nfile) {
		nfile->pos = file->pos;
		if (file->deny_write)
			file_deny_write_unlocked (nfile);
	}
	return nfile;
}

/* ── public locked API ───────────────────────────────────────
 * 아래 함수들은 filesys_lock을 직접 잡고 _unlocked 헬퍼를 호출한다.
 * 외부 모듈(userprog 등)은 이 함수들만 사용한다.
 * ─────────────────────────────────────────────────────────── */

/* Opens a file for the given INODE, of which it takes ownership,
 * and returns the new file.  Returns a null pointer if an
 * allocation fails or if INODE is null. */
/* 주어진 INODE에 대한 파일을 열고 그 ownership을 가져간 뒤 새 파일을 리턴한다.
 * 할당에 실패하거나 INODE가 null이면 null pointer를 리턴한다. */
struct file *
file_open (struct inode *inode) {
	struct file *file = calloc (1, sizeof *file);
	if (inode != NULL && file != NULL) {
		file->inode = inode;
		file->pos = 0;
		file->deny_write = false;
		return file;
	} else {
		inode_close (inode);
		free (file);
		return NULL;
	}
}

/* Opens and returns a new file for the same inode as FILE.
 * Returns a null pointer if unsuccessful. */
/* FILE과 같은 inode에 대한 새 파일을 열고 리턴한다.
 * 실패하면 null pointer를 리턴한다. */
struct file *
file_reopen (struct file *file) {
	return file_open (inode_reopen (file->inode));
}

/* Duplicate the file object including attributes and returns a new file for the
 * same inode as FILE. Returns a null pointer if unsuccessful. */
/* attribute를 포함해 파일 오브젝트를 복제하고 FILE과 같은 inode에 대한 새 파일을
 * 리턴한다. 실패하면 null pointer를 리턴한다. */
struct file *
file_duplicate (struct file *file) {
	lock_acquire (&filesys_lock);
	struct file *result = file_duplicate_unlocked (file);
	lock_release (&filesys_lock);
	return result;
}

/* Closes FILE. */
/* FILE을 닫는다. */
void
file_close (struct file *file) {
	lock_acquire (&filesys_lock);
	file_close_unlocked (file);
	lock_release (&filesys_lock);
}

/* Returns the inode encapsulated by FILE. */
/* FILE이 감싼 inode를 리턴한다. */
struct inode *
file_get_inode (struct file *file) {
	return file->inode;
}

/* Reads SIZE bytes from FILE into BUFFER,
 * starting at the file's current position.
 * Returns the number of bytes actually read,
 * which may be less than SIZE if end of file is reached.
 * Advances FILE's position by the number of bytes read. */
/* FILE의 현재 위치부터 SIZE 바이트를 BUFFER로 읽는다.
 * 실제로 읽은 byte 수를 리턴한다.
 * end of file에 도달하면 SIZE보다 작을 수 있다.
 * 읽은 byte 수만큼 FILE의 위치를 전진시킨다. */
off_t
file_read (struct file *file, void *buffer, off_t size) {
	lock_acquire (&filesys_lock);
	off_t result = file_read_unlocked (file, buffer, size);
	lock_release (&filesys_lock);
	return result;
}

/* Reads SIZE bytes from FILE into BUFFER,
 * starting at offset FILE_OFS in the file.
 * Returns the number of bytes actually read,
 * which may be less than SIZE if end of file is reached.
 * The file's current position is unaffected. */
/* 파일의 FILE_OFS offset부터 SIZE 바이트를 FILE에서 BUFFER로 읽는다.
 * 실제로 읽은 byte 수를 리턴한다.
 * end of file에 도달하면 SIZE보다 작을 수 있다.
 * 파일의 현재 위치는 바뀌지 않는다. */
off_t
file_read_at (struct file *file, void *buffer, off_t size, off_t file_ofs) {
	lock_acquire (&filesys_lock);
	off_t result = file_read_at_unlocked (file, buffer, size, file_ofs);
	lock_release (&filesys_lock);
	return result;
}

/* Writes SIZE bytes from BUFFER into FILE,
 * starting at the file's current position.
 * Returns the number of bytes actually written,
 * which may be less than SIZE if end of file is reached.
 * (Normally we'd grow the file in that case, but file growth is
 * not yet implemented.)
 * Advances FILE's position by the number of bytes read. */
/* BUFFER의 SIZE 바이트를 FILE의 현재 위치부터 쓴다.
 * 실제로 쓴 byte 수를 리턴한다.
 * end of file에 도달하면 SIZE보다 작을 수 있다.
 * 보통은 이 경우 파일을 키우지만 file growth는 아직 구현되어 있지 않다.
 * 읽은 byte 수만큼 FILE의 위치를 전진시킨다. */
off_t
file_write (struct file *file, const void *buffer, off_t size) {
	lock_acquire (&filesys_lock);
	off_t result = file_write_unlocked (file, buffer, size);
	lock_release (&filesys_lock);
	return result;
}

/* Writes SIZE bytes from BUFFER into FILE,
 * starting at offset FILE_OFS in the file.
 * Returns the number of bytes actually written,
 * which may be less than SIZE if end of file is reached.
 * (Normally we'd grow the file in that case, but file growth is
 * not yet implemented.)
 * The file's current position is unaffected. */
/* BUFFER의 SIZE 바이트를 파일의 FILE_OFS offset부터 FILE에 쓴다.
 * 실제로 쓴 byte 수를 리턴한다.
 * end of file에 도달하면 SIZE보다 작을 수 있다.
 * 보통은 이 경우 파일을 키우지만 file growth는 아직 구현되어 있지 않다.
 * 파일의 현재 위치는 바뀌지 않는다. */
off_t
file_write_at (struct file *file, const void *buffer, off_t size,
		off_t file_ofs) {
	lock_acquire (&filesys_lock);
	off_t result = file_write_at_unlocked (file, buffer, size, file_ofs);
	lock_release (&filesys_lock);
	return result;
}

/* Prevents write operations on FILE's underlying inode
 * until file_allow_write() is called or FILE is closed. */
/* file_allow_write()가 호출되거나 FILE이 닫힐 때까지 FILE의 underlying inode에
 * 대한 write operation을 막는다. */
void
file_deny_write (struct file *file) {
	lock_acquire (&filesys_lock);
	file_deny_write_unlocked (file);
	lock_release (&filesys_lock);
}

/* Re-enables write operations on FILE's underlying inode.
 * (Writes might still be denied by some other file that has the
 * same inode open.) */
/* FILE의 underlying inode에 대한 write operation을 다시 활성화한다.
 * 같은 inode를 연 다른 파일 때문에 write가 여전히 거부될 수도 있다. */
void
file_allow_write (struct file *file) {
	lock_acquire (&filesys_lock);
	file_allow_write_unlocked (file);
	lock_release (&filesys_lock);
}

/* Returns the size of FILE in bytes. */
/* FILE의 크기를 byte 단위로 리턴한다. */
off_t
file_length (struct file *file) {
	lock_acquire (&filesys_lock);
	off_t result = file_length_unlocked (file);
	lock_release (&filesys_lock);
	return result;
}

/* Sets the current position in FILE to NEW_POS bytes from the
 * start of the file. */
/* FILE의 현재 위치를 파일 시작점에서 NEW_POS byte 떨어진 곳으로 설정한다. */
void
file_seek (struct file *file, off_t new_pos) {
	lock_acquire (&filesys_lock);
	file_seek_unlocked (file, new_pos);
	lock_release (&filesys_lock);
}

/* Returns the current position in FILE as a byte offset from the
 * start of the file. */
/* FILE의 현재 위치를 파일 시작점 기준 byte offset으로 리턴한다. */
off_t
file_tell (struct file *file) {
	lock_acquire (&filesys_lock);
	off_t result = file_tell_unlocked (file);
	lock_release (&filesys_lock);
	return result;
}
