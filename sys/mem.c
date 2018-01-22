#include <sys/os.h>



void create_page_table_entry(uint64_t logical_address,
                             uint64_t physical_address,
                             uint64_t* pml4,
                             uint16_t flags);

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
      //kprintf("Available Physical Memory [%p-%p]\n", smap->base, smap->base + smap->length);

      if (smap->base < MAX_RAM && smap->base + smap->length >= MAX_RAM)
      {
          last_access_mem = MAX_RAM - 1;
          break;
      }

      if (smap->base >= MAX_RAM)
         break;

      if (smap->base + smap->length > last_access_mem)
         last_access_mem = smap->base + smap->length;
   
    }
  }

  /* Making the last addressable memory 4K aligned*/
  last_physaddr = (last_access_mem/PAGE_SIZE)*PAGE_SIZE;

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
      start_pd[i].flags = USED_MEM_PD | BLOCKED_MEM_PD;
  }

  head_freepd = start_pd[kernel_pages-1].next;


  uint64_t* pml4 = (uint64_t*) _get_page();
  kernel_pml4 = pml4;
  clear_page(pml4);


  /* Create page table mappings so that the kernel can access
   * all the physical memory. We're doing identity mapping here.
   * Also, there is a limit of 2 GB physical memory that the kernel
   * can access.*/
  create_page_tables(KERNEL_BASE, 
                     KERNEL_BASE + last_physaddr - 1,
                     0, pml4, PG_U|PG_P|PG_RW);

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
   uint64_t index = (uint64_t)(head_freepd-start_pd);
   void* ptr = (void*)(index*PAGE_SIZE);
   start_pd[index].flags = USED_MEM_PD;
   start_pd[index].ref_count = 1;
   head_freepd = head_freepd->next;

   //kprintf("id - %d,",(int)index);
   if (head_freepd == 0)
      ERROR("Out of Memory\n");

   return (void*)ptr; 
}

uint64_t get_pd_ref(uint64_t vaddr)
{
   uint64_t ptr = PHYS_ADDR(vaddr);
   uint64_t index = ptr/PAGE_SIZE;

   return start_pd[index].ref_count;
}

void incr_pd_ref(uint64_t vaddr)
{
   uint64_t ptr = PHYS_ADDR(vaddr);
   uint64_t index = ptr/PAGE_SIZE;

   if (!bit (start_pd[index].flags, USED_MEM_PD))
      ERROR("incr_page_ref Memory never allocated - vaddr %p, paddr %p\n", 
            vaddr, ptr);

   start_pd[index].ref_count++;
}

void decr_pd_ref(uint64_t vaddr)
{
   uint64_t ptr = PHYS_ADDR(vaddr);
   uint64_t index = ptr/PAGE_SIZE;

   if (!bit (start_pd[index].flags, USED_MEM_PD))
      ERROR("decr_page_ref Memory never allocated - vaddr %p, paddr %p\n", 
            vaddr, ptr);

   if (start_pd[index].ref_count == 1)
      ERROR("decr_page_ref Invalid Reference count - vaddr %p, paddr %p\n",
            vaddr, ptr);

   start_pd[index].ref_count--;
}

/* _get_page returns the pointer to the virtual address
 * of the page. This virtual address is w.r.t the KERNEL_BASE */
void* _get_page()
{
   return (void*)(KERNEL_BASE + ((uint64_t)_get_page_phys()));
}

void _free_page(void* ptr)
{
   /* Raise an error if someone sent a free page's ptr
    */
   uint64_t index = (PHYS_ADDR((uint64_t)(ptr))/PAGE_SIZE);

   if (!bit (start_pd[index].flags, USED_MEM_PD))
      ERROR("Memory never allocated - %p\n", ptr);

   if (bit (start_pd[index].flags, BLOCKED_MEM_PD))
      ERROR("Memory not supposed to be deleted - %p\n", ptr);

   start_pd[index].ref_count--;
   if (start_pd[index].ref_count == 0)
   {
      start_pd[index].next = head_freepd;
      start_pd[index].flags &= ~USED_MEM_PD;
      head_freepd = &start_pd[index];
   }
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

uint32_t trans_vaddr_pt(uint64_t vaddr, uint64_t* pt)
{
   uint32_t pml4_index = (vaddr >> 39) & 0x1ff;
   uint32_t pdp_index  = (vaddr >> 30) & 0x1ff;
   uint32_t pd_index   = (vaddr >> 21) & 0x1ff;
   uint32_t pt_index   = (vaddr >> 12) & 0x1ff;
   
   uint64_t* pml4 = (uint64_t*) VIRT_ADDR((uint64_t)get_cur_cr3());
   uint64_t p1 = pml4[pml4_index];
   if (!bit(p1, PG_P))
      return 0;

   uint64_t p2 = ((uint64_t*)VIRT_ADDR(CL12(p1)))[pdp_index];
   if (!bit(p2, PG_P))
      return 0;

   uint64_t p3 = ((uint64_t*)VIRT_ADDR(CL12(p2)))[pd_index];
   if (!bit(p3, PG_P))
      return 0;

   uint64_t p4 = ((uint64_t*)VIRT_ADDR(CL12(p3)))[pt_index];
   if (!bit(p4, PG_P))
      return 0;

   *pt = p4;
   return 1;
}

void clear_page(void* ptr)
{
   int i;
   uint64_t* p = (uint64_t*)ptr;
   for (i = 0;i < PAGE_SIZE/8;i++,p++)
       *p = 0;
}


/*
 * Free the entire memory allocated to the task inside page tables
 * and the actual pages that back them.
 */
void destroy_address_space(task* t)
{
   
   uint64_t* cr3 = (uint64_t*) VIRT_ADDR(t->reg_cr3);
   int i,j,k,l;

   for (i = 0;i < 511;i++)
   {
      if (bit(cr3[i], PG_P))
      {
         uint64_t* pdp = (uint64_t*)VIRT_ADDR(CL12(cr3[i]));

         for (j = 0;j < 512;j++)
         {
             if (bit(pdp[j], PG_P) > 0)
             {
                uint64_t* pd = (uint64_t*) VIRT_ADDR(CL12(pdp[j]));

                for (k = 0;k < 512;k++)
                {
                    if (bit(pd[k],PG_P) > 0)
                    {
                       uint64_t* pt = (uint64_t*) VIRT_ADDR(CL12(pd[k]));

                       for (l = 0;l < 512;l++)
                       {
                          if (bit(pt[l], PG_P))
                          {
                             _free_page((void*)VIRT_ADDR(CL12(pt[l])));
                          }
                       }
                       _free_page((void*)pt);
                    }
                }
                _free_page((void*)pd);
             } 
         } 
         _free_page((void*)pdp);
      }
      cr3[i] = 0; 
   }
}


/* Creates a copy of page tables from parent task to child task.
 * We don't copy the 511-entry of pml4 since it is shared among
 * all the tasks.
 */
uint64_t copy_page_tables(task* child, task* parent)
{
   uint64_t* parent_cr3 = (uint64_t*) VIRT_ADDR(parent->reg_cr3);
   uint64_t* child_cr3 = (uint64_t*) VIRT_ADDR(((uint64_t)init_user_pt())); 
   int i,j,k,l;

#define PFLAGS(addr) (addr & 0xFFF)
   for (i = 0;i < 511;i++)
   {
      if (parent_cr3[i] > 0)
      {
         uint64_t* newpdp = (uint64_t*) _get_page();
         uint64_t* pdp = (uint64_t*)VIRT_ADDR(CL12(parent_cr3[i]));
         clear_page(newpdp);
         child_cr3[i] = PHYS_ADDR((uint64_t)newpdp) | PFLAGS(parent_cr3[i]);

         for (j = 0;j < 512;j++)
         {
             if (pdp[j] > 0)
             {
                uint64_t* newpd = (uint64_t*) _get_page();
                uint64_t* pd = (uint64_t*) VIRT_ADDR(CL12(pdp[j]));
                clear_page(newpd);
                newpdp[j] = PHYS_ADDR((uint64_t)newpd) | PFLAGS(pdp[j]);

                for (k = 0;k < 512;k++)
                {
                    if (pd[k] > 0)
                    {
                       uint64_t* newpt = (uint64_t*) _get_page();
                       uint64_t* pt = (uint64_t*) VIRT_ADDR(CL12(pd[k]));
                       clear_page(newpt);
                       newpd[k] = PHYS_ADDR((uint64_t)newpt) | PFLAGS(pd[k]);

                       for (l = 0;l < 512;l++)
                       {
                          if (pt[l] > 0)
                          {
                             newpt[l] = pt[l];
                             bic(newpt[l], PG_RW);
                             bic(pt[l], PG_RW);
                             bis(newpt[l], PG_COW);
                             bis(pt[l], PG_COW);

                             incr_pd_ref(VIRT_ADDR(CL12(pt[l])));
                          }
                       }
                    }
                }
             } 
         } 
      } 
   }

   return PHYS_ADDR(((uint64_t)child_cr3));
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

   //int i = 0;
   for(logical_address = start_logical_address; 
       logical_address < end_logical_address;
       logical_address += PAGE_SIZE, physical_address += PAGE_SIZE
      )
   {
      //if (i %1000 == 0)
      //{
         //kprintf("page - %d\n", i);
         //sleep(1);
      //}
      //i++;
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
   uint64_t pml4_entry = PML4_INDEX(logical_address);
  
 
   // we should check if already this entry is present in the pml4 table
   // can verfiy that by checking the last 2 bits of that entry
   if((pml4[pml4_entry] & PG_P) == PG_P)
   {
      uint64_t* pdp = (uint64_t*) VIRT_ADDR(CL12(pml4[pml4_entry]));
      uint64_t pdp_entry = PDP_INDEX(logical_address);
      
      if((pdp[pdp_entry] & PG_P) == PG_P)
      {
         uint64_t* pd = (uint64_t *) VIRT_ADDR(CL12(pdp[pdp_entry]));
         uint64_t pd_entry = PD_INDEX(logical_address);
         
         if((pd[pd_entry] & PG_P) == PG_P) 
         {
            uint64_t* pt = (uint64_t *) VIRT_ADDR(CL12(pd[pd_entry]));
            uint64_t pt_entry = PT_INDEX(logical_address);
            
            /* COW case, we may have to update physical address and/or flags */
            if((pt[pt_entry] & PG_P) == PG_P)
            {
               //kprintf("COW update %p\n", logical_address);
               pt[pt_entry] = (uint64_t)physical_address;
               pt[pt_entry] |= flags;
            
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

void mem_info()
{
   uint64_t i;
   uint64_t ucnt = 0;
   uint64_t fcnt = 0;

   for (i = 0;i < last_physaddr;i += PAGE_SIZE)
   {
       if (bit(start_pd[i/PAGE_SIZE].flags, USED_MEM_PD))
          ucnt++;
   }
 
   for (i = 0;i < last_physaddr;i += PAGE_SIZE)
   {
       if (!bit(start_pd[i/PAGE_SIZE].flags, USED_MEM_PD))
          fcnt++;
   } 
   //kprintf("head_freepd - %p\n", head_freepd);
   //kprintf("head_freepd index - %d\n", (head_freepd-start_pd));
   //kprintf("Next page physaddr - %p\n", (head_freepd-start_pd)*PAGE_SIZE);
   kprintf("Used pages - %d, free pages - %d\n", (int)ucnt, (int)fcnt);
}
