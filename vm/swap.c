#include "vm/swap.h"

struct swap_disk sd;

bool swap_init () 
{
  sd.swap_disk = disk_get (1,1); //swap
  if (sd.swap_disk == NULL)
    return false;
  sd.num_slot = disk_size(sd.swap_disk)/SECTORS_PER_PAGE;

  sd.swap_map = bitmap_create(sd.num_slot);
  if (sd.swap_map == NULL)
    return false;

  bitmap_set_all(sd.swap_map, false);

  lock_init(&sd.lock);
  return true;
}


bool swap_out (struct frame_entry *fe)
{
  int i;
  disk_sector_t sect_start;

  if (fe == NULL || sd.swap_disk == NULL || sd.swap_map == NULL)
    PANIC ("swap_out : NULL failed");
  if(fe->se == NULL)
    PANIC ("swap_out : se is NULL");
  if (!fe->se->type == TYPE_SWAP)
    PANIC ("swap_out : this is not type_swap");

  lock_acquire(&sd.lock);

  sect_start = bitmap_scan_and_flip (sd.swap_map, 0, 1, false); 
  if (sect_start == BITMAP_ERROR)
    PANIC("swap_out : bitmap error");

  for (i = 0; i < SECTORS_PER_PAGE; i++) {
    disk_write (sd.swap_disk, sect_start*SECTORS_PER_PAGE + i,
		 (char *)(fe->frame + i*DISK_SECTOR_SIZE));
  }
  fe->se->swap_idx = sect_start;

  lock_release(&sd.lock);
  return true;
} 


bool swap_in (struct frame_entry *fe)
{
  int i;
  disk_sector_t sect_start;

  if (fe == NULL || sd.swap_disk == NULL || sd.swap_map == NULL)
    PANIC ("swap_in : NULL failed");

  if(fe->se == NULL)
    PANIC ("swap_in : se is NULL");

  if (!fe->se->type == TYPE_SWAP)
    PANIC ("swap_in : this is not type_swap");


  lock_acquire(&sd.lock);

  sect_start = fe->se->swap_idx;
  if (sect_start > (unsigned)sd.num_slot)
     PANIC ("swap_in : sector index error ");

  for (i = 0; i < SECTORS_PER_PAGE; i++) {
    disk_read (sd.swap_disk, sect_start*SECTORS_PER_PAGE + i,
		 (char *)(fe->frame + i*DISK_SECTOR_SIZE));
  }

  lock_release(&sd.lock);
  return true;
}  


