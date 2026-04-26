#ifndef __LIB_SYSCALL_NR_H
#define __LIB_SYSCALL_NR_H

/* System call numbers. */
/* system call 번호들. */
enum {
	/* Projects 2 and later. */
	/* project 2 이후. */
	SYS_HALT,                   /* Halt the operating system. */
	/* 운영체제를 halt한다. */
	SYS_EXIT,                   /* Terminate this process. */
	/* 현재 프로세스를 종료한다. */
	SYS_FORK,                   /* Clone current process. */
	/* 현재 프로세스를 클론한다. */
	SYS_EXEC,                   /* Switch current process. */
	/* 현재 프로세스를 전환한다. */
	SYS_WAIT,                   /* Wait for a child process to die. */
	/* 자식 프로세스가 종료될 때까지 기다린다. */
	SYS_CREATE,                 /* Create a file. */
	/* 파일을 만든다. */
	SYS_REMOVE,                 /* Delete a file. */
	/* 파일을 삭제한다. */
	SYS_OPEN,                   /* Open a file. */
	/* 파일을 연다. */
	SYS_FILESIZE,               /* Obtain a file's size. */
	/* 파일 크기를 얻는다. */
	SYS_READ,                   /* Read from a file. */
	/* 파일에서 읽는다. */
	SYS_WRITE,                  /* Write to a file. */
	/* 파일에 쓴다. */
	SYS_SEEK,                   /* Change position in a file. */
	/* 파일 안의 위치를 바꾼다. */
	SYS_TELL,                   /* Report current position in a file. */
	/* 파일 안의 현재 위치를 보고한다. */
	SYS_CLOSE,                  /* Close a file. */
	/* 파일을 닫는다. */

	/* Project 3 and optionally project 4. */
	/* project 3, 그리고 선택적으로 project 4. */
	SYS_MMAP,                   /* Map a file into memory. */
	/* 파일을 메모리에 맵핑한다. */
	SYS_MUNMAP,                 /* Remove a memory mapping. */
	/* 메모리 맵핑을 제거한다. */

	/* Project 4 only. */
	/* project 4 전용. */
	SYS_CHDIR,                  /* Change the current directory. */
	/* 현재 디렉터리를 바꾼다. */
	SYS_MKDIR,                  /* Create a directory. */
	/* 디렉터리를 만든다. */
	SYS_READDIR,                /* Reads a directory entry. */
	/* 디렉터리 엔트리를 읽는다. */
	SYS_ISDIR,                  /* Tests if a fd represents a directory. */
	/* fd가 디렉터리를 나타내는지 검사한다. */
	SYS_INUMBER,                /* Returns the inode number for a fd. */
	/* fd에 대한 inode 번호를 리턴한다. */
	SYS_SYMLINK,                /* Returns the inode number for a fd. */
	/* fd에 대한 inode 번호를 리턴한다. */

	/* Extra for Project 2 */
	/* project 2 extra용. */
	SYS_DUP2,                   /* Duplicate the file descriptor */
	/* file descriptor를 복제한다. */

	SYS_MOUNT,
	SYS_UMOUNT,
};

#endif /* lib/syscall-nr.h */
