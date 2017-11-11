#ifndef __TASK_H__
#define __TASK_H__

#include <sys/defs.h>

#define MAX_PROCESSES 1024

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
   //uint64_t reg_cr3;
   //uint64_t reg_rflags;
   
   uint8_t* stack;
   struct _task* next;
};

typedef struct _task task;

void create_task(task* t, void (*main)(), uint64_t flags, uint64_t* pml4);
void init_task_system();
void yield();
void switch_task(task* old, task* new);
#endif
