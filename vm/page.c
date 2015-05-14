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
  if(fe->s_ent == NULL)
    PANIC ("swap_out : s_ent is NULL");
  if (!fe->s_ent->type_swap)
    PANIC ("swap_out : this is not type_swap");

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

  if(fe->s_ent == NULL)
    PANIC ("swap_in : s_ent is NULL");

  if (!fe->s_ent->type_swap)
    PANIC ("swap_in : this is not type_swap");


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


bool fe_init ()
{
  lock_init(&ft.lock);
  list_init(&ft.frame_list);
}

struct frame_entry* fe_alloc (struct sup_entry *se)
{ 
  struct frame_entry *fe;

  if (se == NULL) 
    PANIC ("frame_alloc: se is NULL");
  
  lock_acquire (&ft.lock);

  fe = malloc (sizeof(struct frame_entry));
 
  fe->frame = palloc_get_page (PAL_USER);
  if (fe->frame == NULL)
    {
      fe_evict();
      fe->frame = palloc_get_page (PAL_USER);
      ASSERT (fe->frame);
    }
  if (list_insert (&ft.frame_list, fe->elem) == NULL)
    PANIC ("frame_alloc : list insert failed");
 
  fe->s_ent = se;
  se->fe = fe;
  se->thread = thread_current ();

  lock_release (&ft.lock);
  return fe;
}

bool fe_remove (struct frame_entry *fe)
{
  struct list_elem *iter;
  struct frame_entry *fe_tmp;
  bool frame_removed = false;
  lock_acquire(&ft.lock);
  
  for (iter = list_begin(&ft); iter != list_end(&ft);
       iter = list_next(iter)){
    
    fe_tmp = list_entry(iter, struct frame_entry, elem);
    if (fe == fe_tmp)
      {	
	list_remove(iter);
	palloc_free_page(fe->frame);
	free(fe);
	frame_removed = true;
	break;
      }
  }
  lock_release (&ft.lock);

  return frame_removed; 
}


bool fe_evict ()
{
  struct list_elem *iter;
  struct frame_entry *fe;
  struct sup_entry *se_tmp;
  bool ret = false;
  bool is_accessed = false;
  int acc_cnt;

  //find fe to evict
  for (iter = list_begin (&ft.frame_list); 
       list_next(iter) != list_end(&ft.frame_list; 
       iter = list_next(iter) ) 
    {
      acc_cnt = 0;
      fe = list_entnry (iter, struct sup_entry, elem);
      se_tmp = fe->se;
      
      do {
	if (pagedir_is_accessed (se_tmp->thread->pagedir, se_tmp->uva))
	  {
	    pagedir_set_accessed (se_tmp->thread->pagedir, se_tmp->uva, false);
	    acc_cnt ++;
	  }

	se_tmp = se_tmp->ali_next;
      }while (se_tmp != NULL);

      if (acc_cnt == 0) break;
    }
  
  if (fe->s_ent->type_swap) 
    {
      swap_out (fe); 
      ret = fe_remove (fe);     	
    }

  else 
    { 
      //project 3-2, other types like file
      //if page is dirty write to file
    }

  
  return ret;
}




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
      if (se_h->ali_prev == NULL && se_h->ali_next == NULL) 
	fe_remove (se_h->fe);

      else { //when page is aliased, do not remove frame
	//aliased pages are chained in doubly linked list
	if (se_h->ali_prev == NULL)//head
	  se_h->fe->se = se_h->ali_next;

	else{
	  se_h->ali_prev->ali_next = se_h->ali_next;

	  if (se_h->ali_next != NULL)
	    se_h->ali_next->ali_prev = se_h->ali_prev;	  
	}
      }

      pagedir_clear_page(thread_current()->pagedir, se_h->uva);
    }
  free(se_h);
}

void sup_init ()
{
  hash_init (&thread_current()->spt.sup_hash, page_hash_func, page_less_func, NULL);
}

void pt_destroy ()
{
  hash_destroy (&thread_current()->spt.sup_hash, page_destroy_func);
}

struct sup_entry* get_se (void *uva)
{
  struct sup_entry se;
  se.uva = pg_round_down(uva);

  struct hash_elem *e = hash_find(&thread_current()->spt, &se.elem);
  if (e == NULL)
    {
      return NULL;
    }
  return hash_entry (e, struct sup_entry, elem);
}




void load_swap (struct sup_entry *se)
{
  struct sup_entry * se_tmp;
  se_tmp = se;

  //only ali header has swap_idx
  while (se_tmp->ali_prev == NULL) 
    se_tmp = se_tmp->ali_prev;

  struct frame_enry *fe = fe_alloc(se_tmp);
  swap_in (fe);
}

bool grow_stack (void *uva) 
{
  if (PHYS_BASE - pg_round_down(uva) > MAX_STACK_SIZE)
      return false;

  struct sup_entry *se = malloc(sizeof(struct sup_entry));
  if (se == NULL)
      return false;

  se->uva = pg_round_down(uva);
  se->type_swap = true;
  
  fe_alloc (se);


  if(!install_page (se->uva, se->fe->frame, se->writable))
    fe_remove (se->fe);
  return false;

  hash_insert(&thread_current()->spt, &se->elem);
  return true;
}

