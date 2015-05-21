#include <debug.h>
#include <list.h>
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"

#include "vm/swap.h"
#include "vm/page.h"


struct frame_table {
  struct lock lock;
  struct list frame_list;
};

struct frame_entry {
  struct list_elem elem;
  struct sup_entry *se;

  void *frame;
};

void fe_init (void);
struct frame_entry* fe_alloc (struct sup_entry *se);
bool fe_remove (struct frame_entry *fe);
bool fe_evict (void);



