//#ifndef __VM_PAGE_H__
//`#define __VM_PAGE_H__
#define VM 1

#define MAX_STACK_SIZE 8000000 //heuristic

#define TYPE_MMAP 0
#define TYPE_SWAP 1

//#include "threads/thread.h"
#include <hash.h>
//#include <stdint.h>
//typedef int32_t off_t;

struct sup_entry {
  void *uva;
  bool type;
  bool pinning;

  struct hash_elem elem;  
  struct thread *thread;
  struct frame_entry *fe;  
  
  bool writable;

  //for mmap
  struct file *file;
  uint32_t read_bytes;
 // off_t ofs;
  uint32_t ofs;

  //for swap
  //disk_sector_t swap_idx;
  size_t swap_idx;
};

struct mmap_entry {
  int mapid;
  struct sup_entry *se;
  struct list_elem elem;
};

//#endif /* __VM_PAGE_H__*/





void sup_init (void);
void sup_destroy (void);

struct sup_entry* get_se (void *uva);

/*pinning-itrt is not handled yet*/
bool grow_stack (void *uva);


bool se_mmap (struct file *file, uint32_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable);
void se_munmap (int mapid); 

bool load_mmap (struct sup_entry *se);

bool load_swap (struct sup_entry *se);
bool load_page (struct sup_entry *se);






