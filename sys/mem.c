#include <sys/mem.h>
#include <sys/util.h>
#include <sys/kprintf.h>

#define PG_P    0x001
#define PG_RW   0x002
#define PG_U    0x004
#define PG_PS   0x080


#define PHYS_ADDR(vaddr) ((vaddr) - KERNEL_BASE)
#define VIRT_ADDR(paddr) ((paddr) + KERNEL_BASE)

void create_page_table_entry(uint64_t logical_address,
                             uint64_t physical_address,
                             uint64_t* pml4,
                             uint16_t flags);

void create_page_tables(uint64_t start_logical_address,
                        uint64_t end_logical_address, 
                        uint64_t start_physical_address, 
                        uint64_t* pml4,
                        uint16_t flags);

/*  TODOKISHAN - get_free_page should return virtual address which
 *  would cascade a set of changes in the way page entries are filled.
*/
mem_pd* start_pd;
mem_pd* head_freepd;
uint64_t last_physaddr;


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

  /* Making the last addressable memory 4K aligned*/
  last_physaddr = (last_access_mem/PAGE_SIZE)*PAGE_SIZE;
  kprintf("physfree %p\n", (uint64_t)physfree);
  kprintf("Highest physaddr %p\n", (uint64_t)last_physaddr);

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

       if (last_pd != 0)
       {
          last_pd->next = &start_pd[low];
       }

       for (i = low ;i < high;i++){
          start_pd[i].flags = 0;
          start_pd[i].next = &start_pd[i+1];
       }
       last_pd = &start_pd[high-1];
    }
  }

  last_pd->next = 0; /* The very last pd's next should be NULL */



  pd_entries = last_physaddr/PAGE_SIZE;

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

  uint64_t* pml4 = (uint64_t*) _get_page();
  kernel_pml4 = pml4;
  clear_page(pml4);

  create_page_tables(KERNEL_BASE, 
                     /* TODOKISHAN - We have to better this. At this point, a new page
                        table created after the following address will fail */
                     KERNEL_BASE + last_physaddr - 1,
//                     KERNEL_BASE + (1 << 30) - 1,
                     //KERNEL_BASE + (kernel_pages*PAGE_SIZE) + (100*PAGE_SIZE) - 1,
                     0, pml4, PG_U|PG_P|PG_RW);
  kprintf("Done creating page tables\n");
   __asm__ __volatile__("movq %0, %%cr3"
                         : 
                         : "r"(PHYS_ADDR((uint64_t)(pml4))));
}

uint64_t* init_user_pt()
{
  uint64_t* pml4 = (uint64_t*) _get_page();
  clear_page(pml4);

  // Copy kernel page tables into this process
  pml4[511] = kernel_pml4[511];
  return (uint64_t*)PHYS_ADDR((uint64_t)pml4);
}

/* _get_page_phys returns the pointer to the physical address
 * of the page */
void* _get_page_phys()
{
   void* ptr = (void*)((head_freepd-start_pd)*PAGE_SIZE);
   start_pd[head_freepd-start_pd].flags = USED_MEM_PD;
   head_freepd = head_freepd->next;

   if (head_freepd == 0)
      ERROR("Out of Memory head_freepd - %p\n", head_freepd);

   return (void*)ptr; 
}

/* _get_page returns the pointer to the virtual address
 * of the page. This virtual address is w.r.t the KERNEL_BASE */
void* _get_page()
{
   return (void*)(KERNEL_BASE + ((uint64_t)_get_page_phys()));
}

void* kmalloc(uint64_t size){
    if (size > PAGE_SIZE)
       ERROR("kmalloc more than a size of page");

    return _get_page();
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
   kprintf("orig_addr - %x, trans_addr - %x\n", ptr, ans);
   return ans;
}

void _free_page(void* ptr)
{
   /* Raise an error if someone sent a free page's ptr
    */
   uint64_t index = ((uint64_t)(ptr)/PAGE_SIZE);

   if (!bit (start_pd[index].flags, USED_MEM_PD))
      ERROR("Memory never allocated - %p\n", ptr);

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


/* The caller of this function MUST send the start_logical address
 * aligned with page size and MUST send 
 * (end_logical_address-start_logical_address) to be a set of 
 * pages */
void create_page_tables(uint64_t start_logical_address,
                        uint64_t end_logical_address, 
                        uint64_t start_physical_address, 
                        uint64_t* pml4,
                        uint16_t flags)
{
   uint64_t logical_address;
   uint64_t physical_address = start_physical_address;

   if (start_logical_address % PAGE_SIZE)
   {
      ERROR("create_page_tables : Invalid start_logical_address - 0x%p\n", 
            start_logical_address);
      return; 
   }

   if ((end_logical_address + 1) % PAGE_SIZE)
   {
      ERROR("create_page_tables : Invalid end_logical_address - 0x%p\n", 
            end_logical_address);
      return;
   }

   int i = 0;
   for(logical_address = start_logical_address; 
       logical_address < end_logical_address;
       logical_address += PAGE_SIZE, physical_address += PAGE_SIZE
      )
   {
      if (i %1000 == 0)
      {
         //kprintf("page - %d\n", i);
         //sleep(1);
      }
      i++;
      create_page_table_entry(logical_address, physical_address, pml4, flags);
   }

}


#define PML4_INDEX(ptr) ((((uint64_t)ptr) & 0x0000ff8000000000) >> 39)
#define PDP_INDEX(ptr)  ((((uint64_t)ptr) & 0x0000007fc0000000) >> 30)
#define PD_INDEX(ptr)   ((((uint64_t)ptr) & 0x000000003fe00000) >> 21)
#define PT_INDEX(ptr)   ((((uint64_t)ptr) & 0x00000000001ff000) >> 12)
/* NOTE : If the logical address is already mapped to a physical address, which means that its entry
 * is already present in 4th level PT table, we are DOING NOTHING AS OF NOW!
 * may have to implement later for future use case
 * 
 * What this function does?
 * This function will create an entry in the page table for given logical address and map it to
 * given physical address(which is the address of first byte of a 4Kb page)
 */
void create_page_table_entry(uint64_t logical_address,
                             uint64_t physical_address,
                             uint64_t* pml4,
                             uint16_t flags)
{
#define CL12(addr) ((addr >> 12) << 12)
   uint64_t pml4_entry = PML4_INDEX(logical_address);
  
 
   // we should check if already this entry is present in the pml4 table
   // can verfiy that by checking the last 2 bits of that entry
   if(((pml4[pml4_entry] & PG_P) == PG_P) 
      /*&&
      ((pml4[pml4_entry] & PG_RW) == PG_RW)*/)
   {
      uint64_t* pdp = (uint64_t*) VIRT_ADDR(CL12(pml4[pml4_entry]));
      uint64_t pdp_entry = PDP_INDEX(logical_address);
      
      if(((pdp[pdp_entry] & PG_P) == PG_P) 
         /*&&
         ((pdp[pdp_entry] & PG_RW) == PG_RW)*/)
      {
         uint64_t* pd = (uint64_t *) VIRT_ADDR(CL12(pdp[pdp_entry]));
         uint64_t pd_entry = PD_INDEX(logical_address);
         
         if(((pd[pd_entry] & PG_P) == PG_P) 
            /* &&
            ((pd[pd_entry] & PG_RW) == PG_RW)*/)
         {
            uint64_t* pt = (uint64_t *) VIRT_ADDR(CL12(pd[pd_entry]));
            uint64_t pt_entry = PT_INDEX(logical_address);
            
            if(((pt[pt_entry] & PG_P) == PG_P) 
               /*&&
               ((pt[pt_entry] & PG_RW) == PG_RW)*/)
            {
               // this means that there is already an entry for the given logical address
               // also means that this logical address has already been mapped to a physical address earlier
           
               ;// DO NOTHING AS OF NOW
            }else
            {
               pt[pt_entry] = (uint64_t)physical_address;
               pt[pt_entry] |= flags;
            }
            
         }else
         {
            uint64_t* pt = (uint64_t*) _get_page();
            clear_page(pt);
            pd[pd_entry] = PHYS_ADDR((uint64_t)pt);
            pd[pd_entry] |= flags;

            uint64_t pt_entry = PT_INDEX(logical_address);
            pt[pt_entry] = (uint64_t)physical_address;
            pt[pt_entry] |= flags;
         }
      }else
      {
         uint64_t* pd = (uint64_t*) _get_page();
         clear_page(pd);
         pdp[pdp_entry] = PHYS_ADDR((uint64_t)pd);
         pdp[pdp_entry] |= flags;
      
         uint64_t pd_entry = PD_INDEX(logical_address);
         uint64_t* pt = (uint64_t*) _get_page();
         clear_page(pt);
         pd[pd_entry] = PHYS_ADDR((uint64_t)pt);
         pd[pd_entry] |= flags;

         uint64_t pt_entry = PT_INDEX(logical_address);
         pt[pt_entry] = (uint64_t)physical_address;
         pt[pt_entry] |= flags;
      }
      
   }else
   {
      uint64_t* pdp = (uint64_t*) _get_page();
      clear_page(pdp);
      pml4[pml4_entry] = PHYS_ADDR((uint64_t)pdp);
      pml4[pml4_entry] |= flags;
    
      uint64_t pdp_entry = PDP_INDEX(logical_address);
      uint64_t* pd = (uint64_t*) _get_page();
      clear_page(pd);
      pdp[pdp_entry] = PHYS_ADDR((uint64_t)pd);
      pdp[pdp_entry] |= flags;
    
      uint64_t pd_entry = PD_INDEX(logical_address);
      uint64_t* pt = (uint64_t*) _get_page();
      clear_page(pt);
      pd[pd_entry] = PHYS_ADDR((uint64_t)pt);
      pd[pd_entry] |= flags;
    
      uint64_t pt_entry = PT_INDEX(logical_address);
      pt[pt_entry] = (uint64_t)physical_address;
      pt[pt_entry] |= flags;
   }
}
