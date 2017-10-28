#include <sys/mem.h>
#include <sys/kprintf.h>

#define PG_P    0x001                                                                                         
#define PG_RW   0x002                                                                                         
#define PG_U    0x004                                                                                         
#define PG_PS   0x080     

mem_pd* start_pd;
mem_pd* head_freepd;

void* _get_page();
void  _free_page(void* ptr);
void clear_page(void* ptr);

void init_mem(uint32_t *modulep, void* kernmem, void *physbase, void *physfree){
  uint64_t last_access_mem = 0;
  uint64_t kernel_start;
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
  start_pd = (mem_pd*)(kernmem + (uint64_t)physfree);
  kernel_start = (uint64_t)kernmem + (uint64_t)physfree;
   
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

  kernel_pages = (kernel_start - (uint64_t)kernmem)/PAGE_SIZE + pd_pages;


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
  //kprintf("pd pages - %d\n", pd_pages);
  kprintf("kernel pages - %d\n", kernel_pages);
  //kprintf("head_freepd - %x\n", head_freepd);
  //kprintf("head_freepd index - %d\n", (head_freepd-start_pd));

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

  pml4[PML4_INDEX(kernmem)] = (uint64_t)pdp;
  pml4[PML4_INDEX(kernmem)] |= PG_P|PG_RW;
  pdp[PDP_INDEX(kernmem)] = (uint64_t)pd;
  pdp[PDP_INDEX(kernmem)] |= PG_P|PG_RW;
  pd[PD_INDEX(kernmem)] = (uint64_t)pt1;
  pd[PD_INDEX(kernmem)] |= PG_P|PG_RW;
  pd[PD_INDEX(kernmem)+1] = (uint64_t)pt2;
  pd[PD_INDEX(kernmem)+1] |= PG_P|PG_RW;

  for ( i = 0;i < 512;i++)
  {
     pt1[i] = i*((uint64_t)PAGE_SIZE);
     pt1[i] |= PG_P|PG_RW;
  }

  for (i = 512;i < kernel_pages;i++)
  {
     pt2[i] = i*((uint64_t)PAGE_SIZE);
     pt2[i] |= PG_P|PG_RW;
  }

   __asm__ __volatile__("movq %0, %%cr3"
                         : 
                         : "r"((uint64_t)(pml4)));
}

void* _get_page()
{
   void* ptr = (mem_pd*)((head_freepd-start_pd)*PAGE_SIZE);
   start_pd[head_freepd-start_pd].flags = USED_MEM_PD;
   head_freepd = head_freepd->next;

   return (void*)ptr; 
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

