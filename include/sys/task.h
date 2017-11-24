#ifndef __TASK_H__
#define __TASK_H__

#include <sys/defs.h>

#define MAX_PROCESSES 1024

#define MAX_FILES 64

struct _vma{
  uint64_t start;
  uint64_t end;
  struct _vma* next;
};
typedef struct _vma vma;

struct _fd
{
   uint8_t  name[256];
   uint64_t offset;
#define ALLOCATED_FD 0x001
   uint64_t flags;
};
typedef struct _fd fd;

struct _task{
   /* It is very important that the registers stay at the top.
    * Bad things will happen otherwise. If you MUST change this, change
    * switch_task.s as well */
   uint64_t reg_r15;   // 0
   uint64_t reg_r14;   // 8
   uint64_t reg_r13;   //16
   uint64_t reg_r12;   //24
   uint64_t reg_rbp;   //32
   uint64_t reg_rsp;   //40
   uint64_t reg_rbx;   //48
   uint64_t reg_rip;   //56
   uint64_t reg_cr3;   /* We always store the PHYSICAL ADDRESS in this */
   uint64_t reg_rflags;//72
   uint64_t reg_ursp;  //80
   uint64_t kstack[512]; //88
  
   char     name[256]; 
   vma      mm_struct;  /* This is a vma struct, the first entry is a dummy */
   fd       file[MAX_FILES];
   uint64_t state;
   uint64_t pid;
   uint64_t ppid;
#define RUNNABLE_STATE 0x0001
   uint64_t flags_task;
#define KILL_TASK       0x0001
#define KERNEL_TASK     0x0002
#define ALLOCATED_TASK  0x0004
};

typedef struct _task task;


task* cur_task;

void create_task(task* t, void (*main)(), uint64_t flags, uint64_t* pml4);
void init_task_system();
void schedule();
//void switch_task(task* old, task* new, task** last);
void switch_task(task* old, task* new);
void uswitch_task(task* old, task* new);
void user_main();
uint64_t get_cur_cr3();
void start_sbush();
void add_vma(uint64_t start, uint64_t size);
void save_child_state(task* p, task* c);
uint64_t fork();
#endif
