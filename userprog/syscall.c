#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "filesys/file.h"
#include "vm/page.h"

static void syscall_handler (struct intr_frame *);


struct list file_list;

int fd = 2;

	void
syscall_init (void) 
{
	list_init (&file_list);
	lock_init (&filesys_lock);
	intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

	static void
syscall_handler (struct intr_frame *f UNUSED) 
{
	int *p = f->esp;
	
	switch (*p)
	{
		case SYS_HALT:
			power_off();
			break;
		case SYS_EXIT:
			if (!is_user_vaddr((const void *)(p + 1)))
				exit(-1);
			exit (*(p + 1));
			break;
		case SYS_EXEC:
			if (!is_user_vaddr((const void *)*(p + 1)))
				exit(-1);
			f->eax = exec((const char *)*(p + 1));
			break;
		case SYS_WAIT:
			f->eax = process_wait((int)*(p + 1));
			break;
		case SYS_CREATE:
			f->eax = create((const char *)*(p + 1), (unsigned)*(p + 2));
			break;
		case SYS_REMOVE:
			if (!is_user_vaddr((const void *)*(p + 1)))
				exit (-1);
			f->eax = remove((const char *)*(p + 1));
			break;
		case SYS_OPEN:
			if (!is_user_vaddr((const void *)*(p + 1)))
				exit (-1);
			f->eax = open((const char *)*(p + 1));
			break;
		case SYS_FILESIZE:
			f->eax = filesize((int)*(p + 1));
			break;
		case SYS_READ:
			if (!is_user_vaddr((const void *)*(p + 2)))
				exit (-1);		
			f->eax = read((int)*(p + 1), (void *)*(p + 2), (unsigned)*(p + 3));
			break;
		case SYS_WRITE:
			if (!is_user_vaddr((const void *)*(p + 2)))
				exit (-1);		
			f->eax = write((int)*(p + 1), (const void*)*(p + 2), (unsigned)*(p + 3));
			break;
		case SYS_SEEK:
			seek(*(p + 1), *(p + 2));
			f->eax = 0;
			break;
		case SYS_TELL:		
			f->eax = tell((int)*(p + 1));
			break;
		case SYS_CLOSE:
			close((int)*(p + 1));
			f->eax = 0;
			break;
		case SYS_MMAP:
			f->eax = mmap((int)*(p + 1), *(p + 2));
			break;

		case SYS_MUNMAP:
			se_munmap ((int)*(p + 1));
			break;
	}
}

	void 
exit (int status)
{
	thread_current()->exit_status = status;
	thread_exit();
}

	pid_t 
exec (const char *cmd_line)
{
	if (!cmd_line)
		return -1;

	return process_execute(cmd_line); // lock?
}

	bool 
create (const char *file, unsigned initial_size)
{
	if (!file)
		exit (-1);

	return filesys_create (file, initial_size);
}

	bool 
remove (const char *file)
{
	if (!file) 
		return false;

	return filesys_remove(file);
}

	int 
open (const char *file)
{
	struct file *f;
	struct file_elem *file_elem;

	if (!file)
		return -1;

	f = filesys_open(file);
	if (!f)
		return -1;

	file_elem = (struct file_elem*)malloc(sizeof(struct file_elem)); // error check gogo
	file_elem->file = f;
	file_elem->fd = fd++;

	list_push_back(&file_list, &file_elem->global_elem);
	list_push_back(&thread_current()->file_list, &file_elem->thread_elem);

	return file_elem->fd;
}

	int 
filesize (int fd)
{
	struct file *f = global_get_file(fd);

	if (!f)
		return -1;

	return file_length(f);
}

	int 
read (int fd, void *buffer, unsigned size)
{
	struct file *f;
	unsigned i;
	int ret;

	if (fd == 0) {
		for(i = 0; i < size; i++)
			*(char *)(buffer + i) = input_getc();
		return size;
	}

	f = global_get_file(fd);
	if (!f)
		return -1;

	lock_acquire(&filesys_lock);
	ret = file_read(f, buffer, size);
	lock_release(&filesys_lock);

	return ret;
}

	int 
write (int fd, const void *buffer, unsigned size)
{
	struct file *f;
	int ret;

	if (fd == 1) {
		putbuf(buffer, size);
		return size;
	}

	f = global_get_file(fd);
	if (!f) 
		return -1;

	lock_acquire(&filesys_lock);
	ret = file_write(f, buffer, size);
	lock_release(&filesys_lock);

	return ret;
}

	void 
seek(int fd , unsigned position)
{
	struct file *f = global_get_file(fd);

	if (!f)
		return;

	file_seek(f, position);
}

	unsigned 
tell (int fd)
{
	struct file *f = global_get_file(fd);

	if (!f)
		return -1;

	return file_tell(f);
}

	void 
close (int fd)
{
	struct file_elem *fe = process_get_file(fd);

	if (fe)
	{
		list_remove(&fe->global_elem);
		list_remove(&fe->thread_elem);
		file_close(fe->file);
		free(fe);
	}	
}

	struct file_elem* 
process_get_file (int fd)
{
	struct thread *t = thread_current();
	struct list_elem *e;
	struct file_elem *fe;

	for (e = list_begin(&t->file_list); e != list_end(&t->file_list);
			e = list_next(e)) {
		fe = list_entry(e, struct file_elem, thread_elem);
		if (fe->fd == fd)
			return fe;
	}
	return NULL;
}

	struct file* 
global_get_file (int fd)
{
	struct list_elem *e;
	struct file_elem *fe;

	for (e = list_begin(&file_list); e != list_end(&file_list);
			e = list_next(e)) 	{
		fe = list_entry(e, struct file_elem, global_elem);
		if (fe->fd == fd)
			return fe->file;
	}
	return NULL;
}



int mmap (int fd , void * addr ) { 
  struct file *file = global_get_file (fd); // ????
  if (file == NULL)
    PANIC ("syscall mmap: file is null");

  off_t ofs = 0;
  uint32_t read_bytes = file_length(file);
  uint32_t zero_bytes = PGSIZE - read_bytes % PGSIZE; 
  bool writable = true;

  thread_current()->mapid_cnt++;

  if(!se_mmap (file, ofs, addr, read_bytes, zero_bytes, writable)){
    PANIC ("syscall mmap: mmap is failed");
    thread_current()->mapid_cnt--;
  }

  return thread_current()->mapid_cnt;
}
