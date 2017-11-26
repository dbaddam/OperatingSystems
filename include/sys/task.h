#ifndef __TASK_H__
#define __TASK_H__

#include <sys/defs.h>

#define MAX_PROCESSES 1024
#define MAX_FILES 64
#define MAX_DIR 64
struct  _vma{
  uint64_t start;
  uint64_t end;
  struct _vma* next;
};
typedef struct _vma vma;

struct fd
{
   uint8_t  name[256];
   uint64_t size;
   uint64_t offset;
   uint64_t start_addr;
#define ALLOCATED_FD 0x001
   uint64_t flags;
};
typedef struct fd fd;

struct _task{
   /* It is very important that the registers stay at the top.
    * Bad things will happen otherwise. If you MUST change this, change
    * switch_task.s as well */
   uint64_t reg_r15;
   uint64_t reg_r14;
   uint64_t reg_r13;
   uint64_t reg_r12;
   uint64_t reg_rbp;
   uint64_t reg_rsp;
   uint64_t reg_rbx;
   uint64_t reg_rip;
   uint64_t reg_cr3;   /* We always store the PHYSICAL ADDRESS in this */
   uint64_t reg_rflags;
   uint64_t reg_ursp;
   
   vma* mm_struct;
   uint64_t  kstack[512];
   uint8_t* ustack;    /* Once we have the ability to read elf64 and figure out where the
                          stack is remove this*/
   struct _task* next;
   fd file[MAX_FILES];
   uint64_t state;
#define RUNNABLE_STATE 0x0001
   uint64_t flags_task;
#define KILL_TASK       0x0001
#define KERNEL_TASK     0x0002
#define ALLOCATED_TASK  0x0004
};

typedef struct _task task;
task tasks[MAX_PROCESSES];


task* cur_task;

void create_task(task* t, void (*main)(), uint64_t flags, uint64_t* pml4);
void init_task_system();
void yield();
//void switch_task(task* old, task* new, task** last);
void switch_task(task* old, task* new);
void uswitch_task(task* old, task* new);
void user_main();
uint64_t get_cur_cr3();
void start_sbush();
#endif
