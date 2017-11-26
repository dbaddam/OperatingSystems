#include <sys/os.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/tarfs.h>
#include <sys/ahci.h>
#include <sys/pic.h>
#include <sys/pci.h>
#include <sys/ahci.h>

#define INITIAL_STACK_SIZE 4096
uint8_t initial_stack[INITIAL_STACK_SIZE]__attribute__((aligned(16)));
uint32_t* loader_stack;
extern char kernmem, physbase;

void start(uint32_t *modulep, void *physbase, void *physfree)
{
  kprintf("tarfs in [%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);
  init_mem(modulep, &kernmem, physbase, physfree);
  picremap(MASTER_PIC_OFFSET, SLAVE_PIC_OFFSET);
  kprintf("tarfs in [%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);
  init_idt();
  init_syscall();
  init_task_system();
  //register_all_irqs();
  //pci_enum();

  //start_sbush();
  init_tarfs();  

  while(1);
  while (1)
  {
    kprintf("Idle task\n");
    yield();
  }
  //ahci();
  while(1);
}

void boot(void)
{
  // note: function changes rsp, local stack variables can't be practically used
 register char *temp2;
 for(temp2 = (char*)0xb8001; temp2 < (char*)0xb8000+160*25; temp2 += 2) *temp2 = 7 /* white */;
 for(temp2 = (char*)0xb8000; temp2 < (char*)0xb8000+160*25; temp2 += 2) 
     *temp2 = ' ' /* Let's start with a black screen */;
  __asm__ volatile (
    "cli;"
    "movq %%rsp, %0;"
    "movq %1, %%rsp;"
    :"=g"(loader_stack)
    :"r"(&initial_stack[INITIAL_STACK_SIZE])
  );
  init_gdt();
  start(
    (uint32_t*)((char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase),
    (uint64_t*)&physbase,
    (uint64_t*)(uint64_t)loader_stack[4]
  );

  while(1) __asm__ volatile ("hlt");
}
