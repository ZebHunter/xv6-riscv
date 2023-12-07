// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#define REF_INDEX(x) PGROUNDDOWN(x) / PGSIZE

extern char end[];  // first address after kernel.
                    // defined by kernel.ld.

int ref_count[REF_INDEX(PHYSTOP) + 1];
struct spinlock ref_lock;

void kinit() {
  char *p = (char *)PGROUNDUP((uint64)end);
  bd_init(p, (void *)PHYSTOP);
  memset(ref_count, 0, REF_INDEX(PHYSTOP) * sizeof(int));
  initlock(&ref_lock, "ref_lock");
}

void acquire_ref(void){
  acquire(&ref_lock);
}

void release_ref(void){
  release(&ref_lock);
}

void inc_ref(void* pa){
  ++ref_count[REF_INDEX((uint64)pa)];
}

void dec_ref(void* pa){
  int ref = ref_count[REF_INDEX((uint64)pa)];
  if (ref <= 0) panic("ref < 0");
  ref_count[REF_INDEX((uint64)pa)] = ref - 1;
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void *pa) {
   acquire(&ref_lock);
   dec_ref(pa);
   if(ref_count[REF_INDEX((uint64) pa) <= 0]) bd_free(pa);
   release(&ref_lock); 
   }

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *kalloc(void) { 
  void* pa = bd_malloc(PGSIZE);
  acquire(&ref_lock);
  inc_ref(pa);
  release(&ref_lock);
  return pa;
  }
