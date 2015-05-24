#ifndef __VM_FRMAE_H__
#define __VM_FRMAE_H__



struct frame_table {
  struct lock lock;
  struct list frame_list;
};

struct frame_entry {
  struct list_elem elem;
  struct sup_entry *se;

  void *frame;
};


#endif /* __VM_FRMAE_H__*/

void fe_init (void);
struct frame_entry* fe_alloc (struct sup_entry *se);
bool fe_remove (struct frame_entry *fe);
bool fe_evict (void);



