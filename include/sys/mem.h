#ifndef _MEM_H
#define _MEM_H

#include <sys/defs.h>
#include <sys/task.h>

#define PAGE_SIZE (1 << 12)
#define KERNEL_BASE (0xffffffff80000000)

#define USER_STACK_TOP (0xffffff0000000000)

#define PG_P    0x001
#define PG_RW   0x002
#define PG_U    0x004
#define PG_PS   0x080
#define PG_COW  0x800

#define CL12(addr) ((addr >> 12) << 12)

#define PHYS_ADDR(vaddr) ((vaddr) - KERNEL_BASE)
#define VIRT_ADDR(paddr) ((paddr) + KERNEL_BASE)
#define ADDR_FLOOR(addr) ((addr/PAGE_SIZE)*PAGE_SIZE)

// Page descriptor
struct mem_pd{
#define INVALID_MEM_PD    0x00000001
#define USED_MEM_PD       0x00000002
   uint32_t flags;
   uint32_t pid;
   uint64_t ref_count;
   struct mem_pd* next;
}__attribute__((__packed__));

typedef struct mem_pd mem_pd;
uint64_t* kernel_pml4;


void init_mem(uint32_t *modulep, void* kernmem, void *physbase, void *physfree);
void* kmalloc(uint64_t size);
void* _get_page();
void  _free_page(void* ptr);
uint64_t* init_user_pt();

void create_page_tables(uint64_t start_logical_address,
                        uint64_t end_logical_address, 
                        uint64_t start_physical_address, 
                        uint64_t* pml4,
                        uint16_t flags);

uint64_t copy_page_tables(task* child, task* parent);
uint32_t trans_vaddr_pt(uint64_t vaddr, uint64_t* pt);
uint64_t get_pd_ref(uint64_t vaddr);
void incr_pd_ref(uint64_t vaddr);
void decr_pd_ref(uint64_t vaddr);
void mem_info();
void flush_tlb();
#endif

