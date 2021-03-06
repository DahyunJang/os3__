#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "filesys/filesys.h"
#include "userprog/process.h"
typedef int pid_t;

void syscall_init (void);

void halt (void);
void exit (int status);
pid_t exec (const char *cmd_line);
int wait (pid_t pid);
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize (int fd);
int read (int fd, void *buffer, unsigned size);
int write (int fd, const void *buffer, unsigned size);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);
int mmap (int fd , void * addr );

struct file_elem* process_get_file (int fd);
struct file* global_get_file (int fd);

struct lock filesys_lock;

#endif /* userprog/syscall.h */
