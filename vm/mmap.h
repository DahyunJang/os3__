#include <debug.h>
#include <round.h>
#include <stdio.h>

#include "threads/vaddr.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "userprog/syscall.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "filesys/file.h"

#include "vm/page.h"
#include "vm/frame.h"


int mmap (int fd , void * addr);
void munmap (int mapping);
