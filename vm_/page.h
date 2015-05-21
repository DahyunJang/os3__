#include <debug.h>
#include <bitmap.h>
#include <hash.h>
#include <list.h>
#include <round.h>
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/interrupt.h"
#include "threads/synch.h"
#include "devices/disk.h"
#include "userprog/syscall.h"
#include "userprog/exception.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "filesys/file.h"

#define SECTORS_PER_PAGE (PGSIZE/DISK_SECTOR_SIZE) 
#define MAX_STACK_SIZE 8000000
#define TYPE_FILE = 0;
#define TYPE_SWAP = 1;



/*------------------- SWAP --------------------*/
struct swap_disk {
  struct lock lock;
  struct bitmap *swap_map;

  struct disk *swap_disk;
  int num_slot; //number of slot

} sd;

/*----------------- FRAME TABLE -----------------*/
struct frame_table {
  struct lock lock;
  struct list frame_list;
} ft;

struct frame_entry {
  struct list_elem elem;
  struct sup_entry *se;

  void *frame;
};

/*---------------SUP PAGE TABLE-----------------*/

//in threads structure, 
//plz declare "struct sup_table spt";

struct sup_entry {
  void *uva;
  struct thread *thread;
  struct hash_elem elem;
  bool type;

  struct frame_entry *fe;
  
  //only head(ali_prev == NULL) alias have swap_idx.
  struct sup_entry *ali_prev; //aliased se
  struct sup_entry *ali_next;
  
  disk_sector_t swap_idx;

  bool writable; //when is it becomes false?

  struct file *file;

  off_t size;
  off_t file_ofs;
  //free rsc when process terminated
  //exception error code? or page fault..
};

/*---------------FUNCTIONS-----------------*/

bool swap_init (void); 
bool swap_out (struct frame_entry *fe);
bool swap_in (struct frame_entry *fe);

void fe_init (void);
struct frame_entry* fe_alloc (struct sup_entry *se);
bool fe_remove (struct frame_entry *fe);
bool fe_evict (void);

void sup_init (void);
void pt_destroy (void);
struct sup_entry* get_se (void *uva);
void load_swap (struct sup_entry *se);
bool grow_stack (void *uva);
