#ifndef VM_FILE_H
#define VM_FILE_H
#include <stddef.h>
#include "filesys/file.h"
#include "filesys/off_t.h"
#include "vm/vm.h"

struct page;
enum vm_type;

struct file_page {
	struct file *file;
	off_t ofs;
	size_t read_bytes;
	size_t zero_bytes;
};

struct mmap_page_aux {
	struct file *file;
	off_t ofs;
	size_t read_bytes;
	size_t zero_bytes;
};

void vm_file_init (void);
bool file_backed_initializer (struct page *page, enum vm_type type, void *kva);
void *do_mmap (void *addr, size_t u_length, int writable,
		struct file *file, off_t offset);
void do_munmap (void *va);
#endif
