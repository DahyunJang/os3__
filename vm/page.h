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




struct sup_page_table {
  struct lock lock;
  struct hash sup_hash;
} spt;

struct sup_entry {
  struct hash_elem elem;
  block_sector_t swap_idx;
  //vaddr
  // validity?
  
  //free rsc when process terminated
  //exception error code? or page fault..
} 

struct frame_table {
  struct lock lock;
  struct list frame_list;
} ft;

struct frame_entry {
  struct list_elem elem;

  struct threads thread;
  struct sup_entry *s_ent;

  void *frame;
}

struct swap_disk {
  struct lock lock;
  struct bitmap *swap_map;

  struct block *swap_block;
  int num_slot; //number of slot

} sd;

void sup_init ();
get_disk_page();
stack_growth (); //interrupt frame


bool frame_init ();
bool frame_evict ();
bool frame_create ();
bool frame_destroy (struct frame_entry *);

bool swap_init ();
bool swap_in (struct frame_entry *);
bool swap_out (struct frame_entry *);

