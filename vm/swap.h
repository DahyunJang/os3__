#ifndef __VM_SWAP_H__
#define __VM_SWAP_H__

//#include "vm/page.h"

#define SECTORS_PER_PAGE (PGSIZE/DISK_SECTOR_SIZE) 

struct swap_disk {
  struct lock lock;
  struct bitmap *swap_map;

  struct disk *swap_disk;
  int num_slot; //number of slot

};


bool swap_init (void); 
bool swap_out (struct frame_entry *fe);
bool swap_in (struct frame_entry *fe);



#endif /* __VM_SWAP_H__*/
