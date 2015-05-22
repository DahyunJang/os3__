#include "vm/page.h"



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




void sup_init ()
{
  hash_init (&thread_current()->sup_hash, page_hash_func, page_less_func, NULL);
  list_init (&thread_current()->mmap_list);
  thread_current()->mapid_cnt = 0;
}

void sup_destroy ()
{
  struct mmap_entry *me;
  hash_destroy (&thread_current()->sup_hash, page_destroy_func);
  while (!list_empty (&thread_current()->mmap_list)) {
    me = list_entry (list_pop_front (&thread_current()->mmap_list));
    se_munmap (me->mapid);
    
  }
  thread_current()->mapid_cnt = 0;
}


/*generate default optioned sup entry*/
struct sup_entry* get_se (void *uva)
{
  struct sup_entry se;
  se.uva = pg_round_down(uva);

  struct hash_elem *e = hash_find(&thread_current()->sup_hash, &se.elem);
  if (e == NULL)
    {
      return NULL;
    }
  return hash_entry (e, struct sup_entry, elem);
}



bool grow_stack (void *uva) 
{
  if (PHYS_BASE - pg_round_down(uva) > MAX_STACK_SIZE) //????
      return false;

  struct sup_entry *se = malloc(sizeof(struct sup_entry));
  if (se == NULL)
      return false;

  se->uva = pg_round_down(uva); //????
  se->type = TYPE_SWAP;
  se->pinning = true; //???????????
  se->thread = thread_current();
  se->writable = true; //???????????

  
  if (fe_alloc (se) == NULL) {
    free (se);
    return false;
  }


  hash_insert(&thread_current()->sup_hash, &se->elem);
  return true;
}

bool se_mmap (struct file *file, off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable) 
{
  struct sup_entry *se;
  struct mmap_entry *me;
  struct frame_entry *fe;

  ASSERT ((read_bytes + zero_bytes) % PGSIZE == 0);
  ASSERT (pg_ofs (upage) == 0);
  ASSERT (ofs % PGSIZE == 0);

  file_seek (file, ofs);


  while (read_bytes > 0 || zero_bytes > 0) 
    {
      size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
      size_t page_zero_bytes = PGSIZE - page_read_bytes;

      se = malloc (sizeof (sup_entry));
      se->writable = writable;
      se->file = file;
      se->read_bytes = page_read_bytes;
      se->ofs = ofs;
      fe = fe_alloc (se);
      if (fe == NULL)
        return false;

      /* Load this page. */
      if (file_read (file, fe->frame, page_read_bytes) != (int) page_read_bytes)
        {
          fe_remove (fe);
          hash_delete (se->elem); 
          free (se);
          return false;
	}

      memset (fe->frame + page_read_bytes, 0, page_zero_bytes);

      me = malloc (sizeof (mmap_entry));
      me->mapid = &thread_current->mapid_cnt;
      list_push_back (&thread_current->mmap_list, me->elem);

      /* Advance. */
      read_bytes -= page_read_bytes;
      zero_bytes -= page_zero_bytes;
      ofs += page_read_bytes;
      upage += PGSIZE;
    }
  return true;

}

void se_munmap (int mapid)
{
  struct lash_elem *e; 
  struct sup_entry *se;
	
  for (e = list_begin(&thread_current()->mmap_list); e = list_end(&thread_current()->mmap_list); e = list_next (e))
    {
        me = list_entry (e, struct me_entry, elem);
        if (me->mapid == mapid) {//consider pinning ???????
	  fe_remove (se);
          
          hash_delete (se->elem); //?
          free (se);

          list_remove (e);
          free (me);

	  return;
	}
    }
  
  PANIC ("se_munmap : mapid is not exits");
}


bool load_mmap (struct sup_entry *se)
{
  struct frame_enry* fe;

  fe = fe_alloc (se);
  if (fe == NULL)
    return false;

  if (se->read_bytes > 0)
    {
      lock_acquire(&filesys_lock);
      if ((int) spte->read_bytes != file_read_at(se->file, se->fe->frame,
                                                 se->read_bytes,
                                                 se->offset))
        {
          lock_release(&filesys_lock);
          fe_remove (se->fe);
          return false;
        }
      lock_release(&filesys_lock);
      memset(frame + se->read_bytes, 0, spte->zero_bytes);
    }

  return true;
}


bool load_swap (struct sup_entry *se)
{
  struct frame_enry* fe;

  fe = fe_alloc (se);
  if (fe == NULL)
    return false;

  return swap_in (fe);
}

bool load_page (struct sup_entry *se) 
{
  if (se->fe != NULL)
    return true;

  if (se->type == TYPE_MMAP)
    return load_mmap (se);
  else if (se->type == TYPE_MMAP)
    return load_mmap (se);
  else 
    PANIC ("load : se type is not defined");
}





