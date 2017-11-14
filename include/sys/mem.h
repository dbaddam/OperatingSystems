#ifndef _MEM_H
#define _MEM_H

#include <sys/defs.h>


#define PAGE_SIZE (1 << 12)
#define KERNEL_BASE (0xffffffff80000000)
// Page descriptor
struct mem_pd{
#define INVALID_MEM_PD    0x00000001
#define USED_MEM_PD       0x00000002
   uint32_t flags;
   struct mem_pd* next;
   uint32_t padding;
}__attribute__((__packed__));

typedef struct mem_pd mem_pd;
uint64_t* kernel_pml4;


void init_mem(uint32_t *modulep, void* kernmem, void *physbase, void *physfree);
void* _get_page();
void  _free_page(void* ptr);
uint64_t* create_user_page_tables();
#endif

