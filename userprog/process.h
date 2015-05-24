#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "filesys/off_t.h"

struct file
{
	struct inode *inode;        /* File's inode. */
	off_t pos;                  /* Current position. */
	bool deny_write;            /* Has file_deny_write() been called? */
};

struct file_elem
{
	struct file *file;
	int fd;
	struct list_elem global_elem;    // global list 
	struct list_elem thread_elem;    // thread list
};

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

bool install_page (void *upage, void *kpage, bool writable);
#endif /* userprog/process.h */
