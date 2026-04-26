#include "filesys/inode.h"
#include <list.h>
#include <debug.h>
#include <round.h>
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/free-map.h"
#include "threads/malloc.h"

/* Identifies an inode. */
/* inode를 식별한다. */
#define INODE_MAGIC 0x494e4f44

/* On-disk inode.
 * Must be exactly DISK_SECTOR_SIZE bytes long. */
/* 디스크에 저장되는 inode.
 * 정확히 DISK_SECTOR_SIZE byte 길이여야 한다. */
struct inode_disk {
	disk_sector_t start;                /* First data sector. */
	/* 첫 data sector. */
	off_t length;                       /* File size in bytes. */
	/* byte 단위 파일 크기. */
	unsigned magic;                     /* Magic number. */
	/* magic number 값. */
	uint32_t unused[125];               /* Not used. */
	/* 사용하지 않는다. */
};

/* Returns the number of sectors to allocate for an inode SIZE
 * bytes long. */
/* SIZE byte 길이의 inode에 할당할 sector 수를 리턴한다. */
static inline size_t
bytes_to_sectors (off_t size) {
	return DIV_ROUND_UP (size, DISK_SECTOR_SIZE);
}

/* In-memory inode. */
/* 메모리에 있는 inode. */
struct inode {
	struct list_elem elem;              /* Element in inode list. */
	/* inode list 안의 element. */
	disk_sector_t sector;               /* Sector number of disk location. */
	/* 디스크 위치의 sector 번호. */
	int open_cnt;                       /* Number of openers. */
	/* open한 주체의 수. */
	bool removed;                       /* True if deleted, false otherwise. */
	/* 삭제되었으면 true, 아니면 false. */
	int deny_write_cnt;                 /* 0: writes ok, >0: deny writes. */
	/* 0이면 write 가능, 0보다 크면 write 거부. */
	struct inode_disk data;             /* Inode content. */
	/* inode 내용. */
};

/* Returns the disk sector that contains byte offset POS within
 * INODE.
 * Returns -1 if INODE does not contain data for a byte at offset
 * POS. */
/* INODE 안의 byte offset POS를 포함하는 disk sector를 리턴한다.
 * INODE가 offset POS에 해당하는 byte 데이터를 포함하지 않으면 -1을 리턴한다. */
static disk_sector_t
byte_to_sector (const struct inode *inode, off_t pos) {
	ASSERT (inode != NULL);
	if (pos < inode->data.length)
		return inode->data.start + pos / DISK_SECTOR_SIZE;
	else
		return -1;
}

/* List of open inodes, so that opening a single inode twice
 * returns the same `struct inode'. */
/* 열린 inode들의 리스트. 같은 inode를 두 번 열면 같은 `struct inode`를 리턴하게
 * 한다. */
static struct list open_inodes;

/* Initializes the inode module. */
/* inode 모듈을 초기화한다. */
void
inode_init (void) {
	list_init (&open_inodes);
}

/* Initializes an inode with LENGTH bytes of data and
 * writes the new inode to sector SECTOR on the file system
 * disk.
 * Returns true if successful.
 * Returns false if memory or disk allocation fails. */
/* LENGTH byte의 데이터로 inode를 초기화하고 새 inode를 파일 시스템 디스크의
 * SECTOR sector에 쓴다.
 * 성공하면 true를 리턴한다.
 * 메모리 또는 디스크 할당이 실패하면 false를 리턴한다. */
bool
inode_create (disk_sector_t sector, off_t length) {
	struct inode_disk *disk_inode = NULL;
	bool success = false;

	ASSERT (length >= 0);

	/* If this assertion fails, the inode structure is not exactly
	 * one sector in size, and you should fix that. */
	/* 이 assertion이 실패하면 inode 구조체가 정확히 한 sector 크기가 아니라는
	 * 뜻이며, 그 부분을 수정해야 한다. */
	ASSERT (sizeof *disk_inode == DISK_SECTOR_SIZE);

	disk_inode = calloc (1, sizeof *disk_inode);
	if (disk_inode != NULL) {
		size_t sectors = bytes_to_sectors (length);
		disk_inode->length = length;
		disk_inode->magic = INODE_MAGIC;
		if (free_map_allocate (sectors, &disk_inode->start)) {
			disk_write (filesys_disk, sector, disk_inode);
			if (sectors > 0) {
				static char zeros[DISK_SECTOR_SIZE];
				size_t i;

				for (i = 0; i < sectors; i++) 
					disk_write (filesys_disk, disk_inode->start + i, zeros); 
			}
			success = true; 
		} 
		free (disk_inode);
	}
	return success;
}

/* Reads an inode from SECTOR
 * and returns a `struct inode' that contains it.
 * Returns a null pointer if memory allocation fails. */
/* SECTOR에서 inode를 읽고 그것을 담은 `struct inode`를 리턴한다.
 * 메모리 할당이 실패하면 null pointer를 리턴한다. */
struct inode *
inode_open (disk_sector_t sector) {
	struct list_elem *e;
	struct inode *inode;

	/* Check whether this inode is already open. */
	/* 이 inode가 이미 열려 있는지 확인한다. */
	for (e = list_begin (&open_inodes); e != list_end (&open_inodes);
			e = list_next (e)) {
		inode = list_entry (e, struct inode, elem);
		if (inode->sector == sector) {
			inode_reopen (inode);
			return inode; 
		}
	}

	/* Allocate memory. */
	/* 메모리를 할당한다. */
	inode = malloc (sizeof *inode);
	if (inode == NULL)
		return NULL;

	/* Initialize. */
	/* 초기화한다. */
	list_push_front (&open_inodes, &inode->elem);
	inode->sector = sector;
	inode->open_cnt = 1;
	inode->deny_write_cnt = 0;
	inode->removed = false;
	disk_read (filesys_disk, inode->sector, &inode->data);
	return inode;
}

/* Reopens and returns INODE. */
/* INODE를 다시 열고 리턴한다. */
struct inode *
inode_reopen (struct inode *inode) {
	if (inode != NULL)
		inode->open_cnt++;
	return inode;
}

/* Returns INODE's inode number. */
/* INODE의 inode 번호를 리턴한다. */
disk_sector_t
inode_get_inumber (const struct inode *inode) {
	return inode->sector;
}

/* Closes INODE and writes it to disk.
 * If this was the last reference to INODE, frees its memory.
 * If INODE was also a removed inode, frees its blocks. */
/* INODE를 닫고 디스크에 쓴다.
 * 이것이 INODE에 대한 마지막 reference라면 메모리를 해제한다.
 * INODE가 제거된 inode이기도 했다면 그 block들을 해제한다. */
void
inode_close (struct inode *inode) {
	/* Ignore null pointer. */
	/* null pointer는 무시한다. */
	if (inode == NULL)
		return;

	/* Release resources if this was the last opener. */
	/* 이것이 마지막 opener였다면 리소스를 해제한다. */
	if (--inode->open_cnt == 0) {
		/* Remove from inode list and release lock. */
		/* inode list에서 제거하고 락을 release한다. */
		list_remove (&inode->elem);

		/* Deallocate blocks if removed. */
		/* 제거된 상태라면 block들을 deallocate한다. */
		if (inode->removed) {
			free_map_release (inode->sector, 1);
			free_map_release (inode->data.start,
					bytes_to_sectors (inode->data.length)); 
		}

		free (inode); 
	}
}

/* Marks INODE to be deleted when it is closed by the last caller who
 * has it open. */
/* INODE를 열고 있는 마지막 호출자가 닫을 때 삭제되도록 표시한다. */
void
inode_remove (struct inode *inode) {
	ASSERT (inode != NULL);
	inode->removed = true;
}

/* Reads SIZE bytes from INODE into BUFFER, starting at position OFFSET.
 * Returns the number of bytes actually read, which may be less
 * than SIZE if an error occurs or end of file is reached. */
/* OFFSET 위치부터 INODE에서 SIZE byte를 BUFFER로 읽는다.
 * 실제로 읽은 byte 수를 리턴하며, 에러가 발생하거나 end of file에 도달하면 SIZE보다
 * 작을 수 있다. */
off_t
inode_read_at (struct inode *inode, void *buffer_, off_t size, off_t offset) {
	uint8_t *buffer = buffer_;
	off_t bytes_read = 0;
	uint8_t *bounce = NULL;

	while (size > 0) {
		/* Disk sector to read, starting byte offset within sector. */
		/* 읽을 disk sector와 sector 안의 시작 byte offset. */
		disk_sector_t sector_idx = byte_to_sector (inode, offset);
		int sector_ofs = offset % DISK_SECTOR_SIZE;

		/* Bytes left in inode, bytes left in sector, lesser of the two. */
		/* inode에 남은 byte, sector에 남은 byte, 그리고 둘 중 작은 값. */
		off_t inode_left = inode_length (inode) - offset;
		int sector_left = DISK_SECTOR_SIZE - sector_ofs;
		int min_left = inode_left < sector_left ? inode_left : sector_left;

		/* Number of bytes to actually copy out of this sector. */
		/* 이 sector에서 실제로 복사해 나갈 byte 수. */
		int chunk_size = size < min_left ? size : min_left;
		if (chunk_size <= 0)
			break;

		if (sector_ofs == 0 && chunk_size == DISK_SECTOR_SIZE) {
			/* Read full sector directly into caller's buffer. */
			/* 전체 sector를 호출자의 buffer로 직접 읽는다. */
			disk_read (filesys_disk, sector_idx, buffer + bytes_read); 
		} else {
			/* Read sector into bounce buffer, then partially copy
			 * into caller's buffer. */
			/* sector를 bounce buffer로 읽은 뒤, 호출자의 buffer로 일부만 복사한다. */
			if (bounce == NULL) {
				bounce = malloc (DISK_SECTOR_SIZE);
				if (bounce == NULL)
					break;
			}
			disk_read (filesys_disk, sector_idx, bounce);
			memcpy (buffer + bytes_read, bounce + sector_ofs, chunk_size);
		}

		/* Advance. */
		/* 다음 위치로 진행한다. */
		size -= chunk_size;
		offset += chunk_size;
		bytes_read += chunk_size;
	}
	free (bounce);

	return bytes_read;
}

/* Writes SIZE bytes from BUFFER into INODE, starting at OFFSET.
 * Returns the number of bytes actually written, which may be
 * less than SIZE if end of file is reached or an error occurs.
 * (Normally a write at end of file would extend the inode, but
 * growth is not yet implemented.) */
/* OFFSET부터 BUFFER의 SIZE byte를 INODE에 쓴다.
 * 실제로 쓴 byte 수를 리턴하며, end of file에 도달하거나 에러가 발생하면 SIZE보다
 * 작을 수 있다.
 * 보통 end of file에서 write하면 inode를 확장하지만, growth는 아직 구현되어 있지
 * 않다. */
off_t
inode_write_at (struct inode *inode, const void *buffer_, off_t size,
		off_t offset) {
	const uint8_t *buffer = buffer_;
	off_t bytes_written = 0;
	uint8_t *bounce = NULL;

	if (inode->deny_write_cnt)
		return 0;

	while (size > 0) {
		/* Sector to write, starting byte offset within sector. */
		/* 쓸 sector와 sector 안의 시작 byte offset. */
		disk_sector_t sector_idx = byte_to_sector (inode, offset);
		int sector_ofs = offset % DISK_SECTOR_SIZE;

		/* Bytes left in inode, bytes left in sector, lesser of the two. */
		/* inode에 남은 byte, sector에 남은 byte, 그리고 둘 중 작은 값. */
		off_t inode_left = inode_length (inode) - offset;
		int sector_left = DISK_SECTOR_SIZE - sector_ofs;
		int min_left = inode_left < sector_left ? inode_left : sector_left;

		/* Number of bytes to actually write into this sector. */
		/* 이 sector에 실제로 쓸 byte 수. */
		int chunk_size = size < min_left ? size : min_left;
		if (chunk_size <= 0)
			break;

		if (sector_ofs == 0 && chunk_size == DISK_SECTOR_SIZE) {
			/* Write full sector directly to disk. */
			/* 전체 sector를 디스크에 직접 쓴다. */
			disk_write (filesys_disk, sector_idx, buffer + bytes_written); 
		} else {
			/* We need a bounce buffer. */
			/* bounce buffer가 필요하다. */
			if (bounce == NULL) {
				bounce = malloc (DISK_SECTOR_SIZE);
				if (bounce == NULL)
					break;
			}

			/* If the sector contains data before or after the chunk
			   we're writing, then we need to read in the sector
			   first.  Otherwise we start with a sector of all zeros. */
			/* 쓰려는 chunk 앞이나 뒤에 sector 데이터가 있으면 먼저 sector를 읽어야
			   한다. 그렇지 않으면 전체가 zero인 sector로 시작한다. */
			if (sector_ofs > 0 || chunk_size < sector_left) 
				disk_read (filesys_disk, sector_idx, bounce);
			else
				memset (bounce, 0, DISK_SECTOR_SIZE);
			memcpy (bounce + sector_ofs, buffer + bytes_written, chunk_size);
			disk_write (filesys_disk, sector_idx, bounce); 
		}

		/* Advance. */
		/* 다음 위치로 진행한다. */
		size -= chunk_size;
		offset += chunk_size;
		bytes_written += chunk_size;
	}
	free (bounce);

	return bytes_written;
}

/* Disables writes to INODE.
   May be called at most once per inode opener. */
/* INODE에 대한 write를 비활성화한다.
   inode opener마다 최대 한 번만 호출할 수 있다. */
	void
inode_deny_write (struct inode *inode) 
{
	inode->deny_write_cnt++;
	ASSERT (inode->deny_write_cnt <= inode->open_cnt);
}

/* Re-enables writes to INODE.
 * Must be called once by each inode opener who has called
 * inode_deny_write() on the inode, before closing the inode. */
/* INODE에 대한 write를 다시 활성화한다.
 * 해당 inode에 대해 inode_deny_write()를 호출한 각 inode opener는 inode를 닫기
 * 전에 이 함수를 한 번 호출해야 한다. */
void
inode_allow_write (struct inode *inode) {
	ASSERT (inode->deny_write_cnt > 0);
	ASSERT (inode->deny_write_cnt <= inode->open_cnt);
	inode->deny_write_cnt--;
}

/* Returns the length, in bytes, of INODE's data. */
/* INODE 데이터의 길이를 byte 단위로 리턴한다. */
off_t
inode_length (const struct inode *inode) {
	return inode->data.length;
}
