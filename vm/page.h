#include <debug.h>
#include <hash.h>
#include <list.h>
#include <round.h>
#include "threads/interrupt.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "userprog/pagedir.h"
#include "threads/synch.h"
#include "userprog/syscall.h"
#include "userprog/process.h"
#include "filesys/file.h"
#include "filesys/filesys.h"

#define MAX_STACK_SIZE 8000000

#define TYPE_MMAP = 0;
#define TYPE_SWAP = 1;

struct sup_entry {
  void *uva;
  bool type;
  bool pinning;

  struct hash_elem elem;  
  struct thread *thread;
  struct frame_entry *fe;  
  
  bool writable;

  //for mmap
  struct file *file;
  uint32_t read_bytes;
  off_t ofs;
  struct mmap_entry *me;

  //for swap
  disk_sector_t swap_idx;
  
};

struct mmap_entry {
  int mapid;
  struct sup_entry *se;
  struct list_elem elem;
};



void sup_init (void);
void sup_destroy (void);
struct sup_entry* get_se (void *uva);

/*pinning-itrt is not handled yet*/

bool grow_stack (void *uva);

bool se_mmap (struct file *file, off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable) 
void se_munmap (int mapid); 

bool load_mmap (struct sup_entry *se);
bool load_swap (struct sup_entry *se);
bool load_page (struct sup_entry *se);








/* 
hash supplemental functions */
static unsigned page_hash_func (const struct hash_elem *e, void *aux UNUSED)
{
  struct sup_entry *se = hash_entry(e, struct sup_entry, elem);
  return hash_int((int) se->uva);
}

static bool page_less_func (const struct hash_elem *a,
			    const struct hash_elem *b,
			    void *aux UNUSED)
{
  struct sup_entry *sa = hash_entry(a, struct sup_entry, elem);
  struct sup_entry *sb = hash_entry(b, struct sup_entry, elem);
 
  return (sa->uva < sb->uva);
}

static void page_destroy_func (struct hash_elem *e, void *aux UNUSED)
{  
  struct sup_entry *se_h = hash_entry(e, struct sup_entry, elem);
  
  if (se_h->fe != NULL)
    {
      pagedir_clear_page(thread_current()->pagedir, se_h->uva);
    }
  free(se_h);
}


