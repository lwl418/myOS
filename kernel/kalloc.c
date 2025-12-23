// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.


#include "include/types.h"
#include "include/param.h"
#include "include/memlayout.h"
#include "include/riscv.h"
#include "include/spinlock.h"
#include "include/kalloc.h"
#include "include/string.h"
#include "include/printf.h"

void freerange(void *pa_start, void *pa_end);

extern char kernel_end[]; // first address after kernel.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  uint64 npage;
} kmem;

// Page reference counting for COW
#define MAX_PAGE_COUNT ((PHYSTOP - KERNBASE) / PGSIZE)

struct {
  struct spinlock lock;
  int ref_count[MAX_PAGE_COUNT];
} page_ref;

static inline int
pa2index(uint64 pa)
{
  return (pa - KERNBASE) / PGSIZE;
}

static inline uint64
index2pa(int index)
{
  return KERNBASE + index * PGSIZE;
}

// Increment reference count for a physical page
void
incref(uint64 pa)
{
  acquire(&page_ref.lock);
  int idx = pa2index(pa);
  if (idx >= 0 && idx < MAX_PAGE_COUNT) {
    page_ref.ref_count[idx]++;
  }
  release(&page_ref.lock);
}

// Decrement reference count for a physical page
void
decref(uint64 pa)
{
  acquire(&page_ref.lock);
  int idx = pa2index(pa);
  if (idx >= 0 && idx < MAX_PAGE_COUNT) {
    if (page_ref.ref_count[idx] > 0) {
      page_ref.ref_count[idx]--;
    }
  }
  release(&page_ref.lock);
}

// Get reference count for a physical page
int
getref(uint64 pa)
{
  int ref = 0;
  acquire(&page_ref.lock);
  int idx = pa2index(pa);
  if (idx >= 0 && idx < MAX_PAGE_COUNT) {
    ref = page_ref.ref_count[idx];
  }
  release(&page_ref.lock);
  return ref;
}

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&page_ref.lock, "page_ref");
  kmem.freelist = 0;
  kmem.npage = 0;
  // Initialize reference counts to 0
  for (int i = 0; i < MAX_PAGE_COUNT; i++) {
    page_ref.ref_count[i] = 0;
  }
  freerange(kernel_end, (void*)PHYSTOP);
  #ifdef DEBUG
  printf("kernel_end: %p, phystop: %p\n", kernel_end, (void*)PHYSTOP);
  printf("kinit\n");
  #endif
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < kernel_end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Decrement reference count, only free if count reaches 0
  acquire(&page_ref.lock);
  int idx = pa2index((uint64)pa);
  if (idx >= 0 && idx < MAX_PAGE_COUNT) {
    if (page_ref.ref_count[idx] > 0) {
      page_ref.ref_count[idx]--;
    }
    if (page_ref.ref_count[idx] > 0) {
      // Page still referenced, don't actually free it
      release(&page_ref.lock);
      return;
    }
  }
  release(&page_ref.lock);

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  kmem.npage++;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r) {
    kmem.freelist = r->next;
    kmem.npage--;
  }
  release(&kmem.lock);

  if(r) {
    memset((char*)r, 5, PGSIZE); // fill with junk
    // Initialize reference count to 1
    acquire(&page_ref.lock);
    int idx = pa2index((uint64)r);
    if (idx >= 0 && idx < MAX_PAGE_COUNT) {
      page_ref.ref_count[idx] = 1;
    }
    release(&page_ref.lock);
  }
  return (void*)r;
}

uint64
freemem_amount(void)
{
  return kmem.npage << PGSHIFT;
}
