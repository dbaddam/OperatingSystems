#include <sys/mem.h>
#include <sys/kprintf.h>

#define PG_P    0x001
#define PG_RW   0x002
#define PG_U    0x004
#define PG_PS   0x080


#define PHYS_ADDR(vaddr) (vaddr - KERNEL_BASE)
#define VIRT_ADDR(paddr) (paddr + KERNEL_BASE)

/*  TODOKISHAN - get_free_page should return virtual address which
 *  would cascade a set of changes in the way page entries are filled.
*/
mem_pd* start_pd;
mem_pd* head_freepd;

void clear_page(void* ptr);
uint64_t trans_addr(uint64_t ptr, uint64_t* pml4);

void init_mem(uint32_t *modulep, void* kernmem, void *physbase, void *physfree){
  uint64_t last_access_mem = 0;
  uint64_t kernel_pages;
  uint64_t pd_pages;
  uint64_t pd_entries;
  mem_pd* last_pd;
  int i;

  struct smap_t {
    uint64_t base, length;
    uint32_t type;
  }__attribute__((packed)) *smap;

  while(modulep[0] != 0x9001) modulep += modulep[1]+2;

  for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
    if (smap->type == 1 /* memory */ && smap->length != 0) {
      kprintf("Available Physical Memory [%p-%p]\n", smap->base, smap->base + smap->length);

      if (smap->base + smap->length > last_access_mem)
         last_access_mem = smap->base + smap->length;
   
    }
  }
  kprintf("physfree %p\n", (uint64_t)physfree);
  kprintf("last_mem %p\n", (uint64_t)last_access_mem);

  /* The page descriptors start at start_pd*/ 
  start_pd = (mem_pd*)(KERNEL_BASE + (uint64_t)physfree);
   
  /* Initially mark all the pages as invalid */
  for (i = 0; i < last_access_mem; i+=PAGE_SIZE)
  {
     start_pd[i/PAGE_SIZE].flags = INVALID_MEM_PD;
     start_pd[i/PAGE_SIZE].next = 0;
  }

  /* Mark all the valid physical pages and next pointer of all the
   * page descriptors */ 
  last_pd = 0;
  for(smap = (struct smap_t*)(modulep+2); 
      smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); 
      ++smap) 
  {
    if (smap->type == 1  && smap->length != 0) {
       int low = smap->base/PAGE_SIZE;
       int high = (smap->base + smap->length)/PAGE_SIZE;

       //kprintf("low - %d, high - %d\n", low, high);
       if (last_pd != 0)
       {
          last_pd->next = &start_pd[low];
       }

       for (i = low ;i < high;i++){
          start_pd[i].flags = 0;
          start_pd[i].next = &start_pd[i+1];
          //kprintf("pd[%d].next = %p, ", i, &pd[i+1]);
       }
       last_pd = &start_pd[high-1];
    }
  }

  last_pd->next = 0; /* The very last pd's next should be NULL */



  if (last_access_mem%PAGE_SIZE)
     pd_entries = last_access_mem/PAGE_SIZE + 1;
  else
     pd_entries = last_access_mem/PAGE_SIZE;

  if (pd_entries%PAGE_SIZE)
     pd_pages = (pd_entries*sizeof(mem_pd))/PAGE_SIZE + 1;
  else
     pd_pages = (pd_entries*sizeof(mem_pd))/PAGE_SIZE;

  kernel_pages = ((uint64_t)physfree)/PAGE_SIZE + pd_pages;


  int invalid_count = 0;
  int next_zero_count = 0;
  for (i = 0;i < pd_entries;i++)
  {
     if (start_pd[i].flags == INVALID_MEM_PD)
        invalid_count++;
     if (start_pd[i].next == 0)
        next_zero_count++;
  }

  for (i = 0;i < kernel_pages;i++)
  {
      start_pd[i].flags = USED_MEM_PD;
  }

  head_freepd = start_pd[kernel_pages-1].next;

  //kprintf("invalid_count - %d, next_zero_count - %d\n", invalid_count, next_zero_count);
  kprintf("page descriptor entries - %d\n", pd_entries);
  kprintf("kernmem - %p\n", kernmem);
  kprintf("pd pages - %d\n", pd_pages);
  kprintf("kernel pages - %d\n", kernel_pages);
  kprintf("head_freepd - %p\n", head_freepd);
  kprintf("head_freepd index - %d\n", (head_freepd-start_pd));

#define PML4_INDEX(ptr) ((((uint64_t)ptr) & 0x0000ff8000000000) >> 39)
#define PDP_INDEX(ptr)  ((((uint64_t)ptr) & 0x0000007fc0000000) >> 30)
#define PD_INDEX(ptr)   ((((uint64_t)ptr) & 0x000000003fe00000) >> 21)
#define PT_INDEX(ptr)   ((((uint64_t)ptr) & 0x00000000001ff000) >> 12)
  uint64_t* pml4 = (uint64_t*) _get_page();
  uint64_t* pdp = (uint64_t*) _get_page();
  uint64_t* pd = (uint64_t*) _get_page();
  uint64_t* pt1 = (uint64_t*) _get_page();
  uint64_t* pt2 = (uint64_t*) _get_page();

  clear_page(pml4);
  clear_page(pdp);
  clear_page(pd);
  clear_page(pt1);
  clear_page(pt2);

  pml4[PML4_INDEX(KERNEL_BASE)] = PHYS_ADDR((uint64_t)pdp);
  pml4[PML4_INDEX(KERNEL_BASE)] |= PG_P|PG_RW;
  pdp[PDP_INDEX(KERNEL_BASE)] = PHYS_ADDR((uint64_t)pd);
  pdp[PDP_INDEX(KERNEL_BASE)] |= PG_P|PG_RW;
  pd[PD_INDEX(KERNEL_BASE)] = PHYS_ADDR((uint64_t)pt1);
  pd[PD_INDEX(KERNEL_BASE)] |= PG_P|PG_RW;
  pd[PD_INDEX(KERNEL_BASE)+1] = PHYS_ADDR((uint64_t)pt2);
  pd[PD_INDEX(KERNEL_BASE)+1] |= PG_P|PG_RW;

  kprintf("PML4 - %p, index = %d\n", pml4, PML4_INDEX(KERNEL_BASE));
  kprintf("PDP - %p, index = %d\n", pdp, PDP_INDEX(KERNEL_BASE));
  kprintf("PD - %p, index = %d\n", pd, PD_INDEX(KERNEL_BASE));
  kprintf("pt1 - %p, index = %d\n", pt1, 1);
  kprintf("pt2 - %p, index = %d\n", pt2, 2);
  for ( i = 0;i < 512;i++)
  {
     pt1[i] = i*((uint64_t)PAGE_SIZE);
     pt1[i] |= PG_P|PG_RW;
  }

  /* Why only upto kernel_pages?? */
  /* You may have to add another set of pages to map lower memory */
  //for (i = 512;i < kernel_pages;i++)
  for (i = 512;i < 1024;i++)
  {
     pt2[i-512] = i*((uint64_t)PAGE_SIZE);
     pt2[i-512] |= PG_P|PG_RW;
  }

  /*DELETE at some point of time*/ 
  uint64_t* pdp1 = (uint64_t*) _get_page();
  uint64_t* pd1 = (uint64_t*) _get_page();
  uint64_t* pt3 = (uint64_t*) _get_page();

  clear_page(pdp1);
  clear_page(pd1);
  clear_page(pt3);
  pml4[0] = PHYS_ADDR((uint64_t)pdp1) | PG_P|PG_RW;
  pdp1[0] = PHYS_ADDR((uint64_t)pd1) | PG_P|PG_RW;
  pd1[0] = PHYS_ADDR((uint64_t)pt3) | PG_P|PG_RW;
  for ( i = 0;i < 256;i++)
  {
     pt3[i] = i*((uint64_t)PAGE_SIZE);
     pt3[i] |= PG_P|PG_RW;
  }
  
  trans_addr(0xffffffff8020062e, PHYS_ADDR(pml4));
  kprintf("About to go into loop\n");
/*
   __asm__ __volatile__("movq %0, %%cr4"
                         : 
                         : "r"((uint64_t)0));
*/
   __asm__ __volatile__("movq %0, %%cr3"
                         : 
                         : "r"(PHYS_ADDR((uint64_t)(pml4))));
}



void* _get_page_phys()
{
   void* ptr = (void*)((head_freepd-start_pd)*PAGE_SIZE);
   start_pd[head_freepd-start_pd].flags = USED_MEM_PD;
   head_freepd = head_freepd->next;

   return (void*)ptr; 
}

void* _get_page()
{
   return (void*)(KERNEL_BASE + ((uint64_t)_get_page_phys()));
}

uint64_t trans_addr(uint64_t ptr, uint64_t* pml4)
{
#define PML4_INDEX(ptr) ((((uint64_t)ptr) & 0x0000ff8000000000) >> 39)
#define PDP_INDEX(ptr)  ((((uint64_t)ptr) & 0x0000007fc0000000) >> 30)
#define PD_INDEX(ptr)   ((((uint64_t)ptr) & 0x000000003fe00000) >> 21)
#define PT_INDEX(ptr)   ((((uint64_t)ptr) & 0x00000000001ff000) >> 12)
   uint64_t x = (ptr >> 39) & 0x1ff;
   uint64_t y = (ptr >> 30) & 0x1ff;
   uint64_t z = (ptr >> 21) & 0x1ff;
   uint64_t q = (ptr >> 12) & 0x1ff;

   uint64_t* p1 = (uint64_t*)pml4[x];
   uint64_t* p2 = (uint64_t*)((uint64_t*)((((uint64_t)p1) >> 2) << 2))[y];
   uint64_t* p3 = (uint64_t*)((uint64_t*)((((uint64_t)p2) >> 2) << 2))[z];
   uint64_t* p4 = (uint64_t*)((uint64_t*)((((uint64_t)p3) >> 2) << 2))[q];
   uint64_t  ans = (uint64_t)(((((uint64_t)p4 >> 2) << 2) + (ptr & 0xfff)));
   return ans;
}

void _free_page(void* ptr)
{
   uint64_t index = ((uint64_t)(ptr)/PAGE_SIZE);
   start_pd[index].next = head_freepd;

   start_pd[index].flags &= ~USED_MEM_PD;

   head_freepd = &start_pd[index];
}

void clear_page(void* ptr)
{
   int i;
   uint64_t* p = (uint64_t*)ptr;
   for (i = 0;i < PAGE_SIZE/8;i++,p++)
       *p = 0;
}

