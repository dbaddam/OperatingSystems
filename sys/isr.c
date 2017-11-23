#include <sys/os.h>


void add_single_page(uint64_t addr)
{
   uint64_t page = (uint64_t) _get_page();
   create_page_tables(ADDR_FLOOR(addr), ADDR_FLOOR(addr) + PAGE_SIZE - 1,
                      PHYS_ADDR((uint64_t)page), 
                      (uint64_t*)VIRT_ADDR((uint64_t)get_cur_cr3()),
                      PG_P|PG_RW|PG_U);
}


void isr_page_fault(uint64_t eno, uint64_t cr2)
{
   task *t = cur_task;
   vma  *p = &t->mm_struct;
   vma  *lastp; 

   // Skip the dummy entry
   lastp = p;
   p = p->next;
   while (p != NULL)
   {
      if (cr2 >= p->start && cr2 < p->end)
      {
         add_single_page(cr2);
         return;
      }
      lastp = p;
      p = p->next;
   }

   // This is the stack vma
   if (p == NULL && cr2 > lastp->start - PAGE_SIZE)
   {
      add_single_page(cr2);
      lastp->start -= PAGE_SIZE;  // Increase the stack size
      return; 
   }
   ERROR("Invalid Page fault erno - %p, cr2 - %p", eno, cr2);
}
