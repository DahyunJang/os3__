#include "vm/frame.h"

void fe_init ()
{
  lock_init(&thread_current->ft.lock);
  list_init(&thread_current->ft.frame_list);
}

struct frame_entry* fe_alloc (struct sup_entry *se)
{ 
  struct frame_entry *fe;

  if (se == NULL) 
    PANIC ("frame_alloc: se is NULL");
  
  lock_acquire (&thread_current->ft.lock);

  fe = malloc (sizeof(struct frame_entry));
 
  fe->frame = palloc_get_page (PAL_USER);
  if (fe->frame == NULL)
    {
      fe_evict();
      fe->frame = palloc_get_page (PAL_USER);
      ASSERT (fe->frame);
    }

  if (!install_page (se->uva, fe->frame, se->writable))
    {
      palloc_free_page (fe->frame);
      free (fe);
      return false;
    }

 
  fe->se = se;
  se->fe = fe;
  se->thread = thread_current ();
	
  list_push_back (&thread_current->ft.frame_list, &fe->elem);
  lock_release (&thread_current->ft.lock);
  return fe;
}

bool fe_remove (struct frame_entry *fe)
{
  struct list_elem *iter;
  struct frame_entry *fe_tmp;
  bool frame_removed = false;
  lock_acquire(&thread_current->ft.lock);
  
  for (iter = list_begin(&thread_current->ft.frame_list); iter != list_end(&thread_current->ft.frame_list);
       iter = list_next(iter)){
    
    fe_tmp = list_entry(iter, struct frame_entry, elem);
    if (fe == fe_tmp)
      {	
	list_remove(iter);
	palloc_free_page(fe->frame);
        fe->se->fe = NULL;
	free(fe);
	frame_removed = true;
	break;
      }
  }
  lock_release (&thread_current->ft.lock);

  return frame_removed; 
}


bool fe_evict ()
{
  struct list_elem *iter;
  struct frame_entry *fe = NULL;
  struct sup_entry *se_tmp;
  bool ret = false;
 
  int acc_cnt;

  //find fe to evict : second chance
  for (iter = list_begin (&thread_current->ft.frame_list); 
       iter != list_end(&thread_current->ft.frame_list); 
       iter = list_next(iter) ) 
    {
      acc_cnt = 0;
      fe = list_entry (iter, struct frame_entry, elem);
      if (fe == NULL)
	PANIC ("fe_evict: NULL error");
      se_tmp = fe->se;

      if (se_tmp->pinning) 
	continue;
      if (pagedir_is_accessed (se_tmp->thread->pagedir, 
			       se_tmp->uva))
	{
	  pagedir_set_accessed (se_tmp->thread->pagedir, 
				se_tmp->uva, false);
	}
      else break;	
    }

  if (iter == list_end(&thread_current->ft.frame_list)) 
    {
      for (iter = list_begin (&thread_current->ft.frame_list); 
	   iter != list_end(&thread_current->ft.frame_list); 
	   iter = list_next(iter) ) 
	{
	  if (!se_tmp->pinning) 
	    break;
	
	}
    }
  if (iter == list_end(&thread_current->ft.frame_list)) 
    PANIC ("fe_evict : all frames are pinned");
  

  if (fe->se->type == TYPE_SWAP) 
    {
      swap_out (fe); 
      ret = fe_remove (fe);     	
    }

  else if (fe->se->type == TYPE_MMAP) 
    { 
      if (pagedir_is_dirty(t->pagedir, fe->se->uva)){
	lock_acquire(&filesys_lock);
	file_write_at(fe->se->file, fe->frame,fe->se->size
		      ,fe->se->file_ofs);
	lock_release(&filesys_lock);
      }
      ret = fe_remove (fe);     
    }

  else 
    PANIC ("fe_evict: page type is undefined\n");

  
  return ret;
}

