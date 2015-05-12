#include "vm/page.h"


bool swap_init () 
{
  sd.swap_block = block_get_role (BLOCK_SWAP);
  if (sd.swap_block == NULL)
    return false;
  sd.num_slot = block_size(sd.swap_block)/SECTORS_PER_PAGE;

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
  block_sector_t sect_start;

  if (fe == NULL || sd.swap_block == NULL || sd.swap_map == NULL)
    PANIC ("swap_out : NULL failed");
  lock_acquire(&sd.lock);

  sect_start = bitmap_scan_and_flip (sd.swap_map, 0, 1, false); 
  if (sect_start == BITMAP_ERROR)
    PANIC("swap_out : bitmap error");

  for (i = 0; i < SECTORS_PER_PAGE; i++) {
    block_write (sd.swap_block, sect_start*SECTORS_PER_PAGE + i,
		 (char *)(fe->frame + i*BLOCK_SECTOR_SIZE));
  }
  fe->s_ent->swap_idx = sect_start;

  lock_release(&sd.lock);
  return true;
} 

bool swap_in (struct frame_entry *fe)
{
  int i;
  block_sector_t sect_start;

  if (fe == NULL || sd.swap_block == NULL || sd.swap_map == NULL)
    PANIC ("swap_in : NULL failed");
  lock_acquire(&sd.lock);

  sect_start = fe->s_ent->swap_idx;
  if (sect_start > sd.num_slot)
     PANIC ("swap_in : sector index error ");

  for (i = 0; i < SECTORS_PER_PAGE; i++) {
    block_read (sd.swap_block, sect_start*SECTORS_PER_PAGE + i,
		 (char *)(fe->frame + i*BLOCK_SECTOR_SIZE));
  }

  lock_release(&sd.lock);
  return true;
}  


bool frame_init ()
{
  lock_init(&ft.lock);
  list_init(&ft.frame_list);
}


bool frame_evict () 
{


}


bool frame_destroy (struct frame_entry *fe) {
  if (fe == NULL) return;

  //remove frame from list
  //free allocated frame

}

