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
#include "devices/block.h"
#include "userprog/syscall.h"
#include "userprog/exception.h"

#define SECTORS_PER_PAGE (PGSIZE/BLOCK_SECTOR_SIZE) 


/*------------------- SWAP --------------------*/
struct swap_disk {
  struct lock lock;
  struct bitmap *swap_map;

  struct block *swap_block;
  int num_slot; //number of slot

} sd;


bool swap_init ();
bool swap_in (struct frame_entry *);
bool swap_out (struct frame_entry *);

/*----------------- FRAME TABLE -----------------*/
struct frame_table {
  struct lock lock;
  struct list frame_list;
} ft;

struct frame_entry {
  struct list_elem elem;

  
  struct sup_entry *s_ent;

  void *frame;
}



/*---------------SUP PAGE TABLE-----------------*/

struct sup_table {
  struct lock lock;
  struct hash sup_hash;
}; 
//in threads structure, 
//plz declare "struct sup_table spt";

struct sup_entry {
  void *uva;
  struct threads *thread;
  struct hash_elem elem;
  bool type_swap;



  struct frame_entry *fe;
  /* aliasing -> page list : modify page installation  */  
  struct sup_entry *ali_prev;
  struct sup_entry *ali_next;
  //only head(ali_prev == NULL) alias have swap_idx.
  block_sector_t swap_idx;
  
  //free rsc when process terminated
  //exception error code? or page fault..
} 


bool sup_init ();

