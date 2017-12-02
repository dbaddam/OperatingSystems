#ifndef __TASK_H__
#define __TASK_H__

#include <sys/defs.h>

#define MAX_PROCESSES 1024
#define MAX_FILES 64

#define MAX_FILE_NAME_SIZE 256


struct _inode{
   uint64_t fstart;
   uint64_t fsize;
   uint64_t msize;
}__attribute__((__packed__));

typedef struct _inode inode;

struct _vma{
  uint64_t start;    // This might not be page-aligned BEWARE
  uint64_t end;
  inode    node;
  uint64_t anon;
  uint64_t flags;
#define HEAP_VMA 0x001
  struct _vma* next;
}__attribute__((__packed__));
typedef struct _vma vma;

struct fd
{
   uint8_t  name[MAX_FILE_NAME_SIZE];
   uint64_t size;
   uint64_t offset;
   uint64_t start_addr;
#define ALLOCATED_FD 0x001
   uint64_t flags;
};
typedef struct fd fd;

/*
 * A task can be in SUSPENDED state if it waiting for a read or if it
 * is sleeping.
 * A task can be in WAITING state only when a wait() is called on it.
*/

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
  
   char     name[MAX_FILE_NAME_SIZE]; 
   vma      mm_struct;  /* This is a vma struct, the first entry is a dummy */
   fd       file[MAX_FILES];

   char     pwd[MAX_FILE_NAME_SIZE];
   uint64_t sleep_time;
   uint32_t state;
#define RUNNING_STATE 0x0001
#define WAITING_STATE 0x0002
#define SUSPEND_STATE 0x0004
#define ZOMBIE_STATE  0x0008
#define AVAIL_STATE   0x0010
#define COOK_STATE    0x0020

   uint32_t exit_status;

   uint32_t pid;
   uint32_t ppid;

}__attribute__((__packed__));

typedef struct _task task;


task* cur_task;

void init_task_system();
void schedule();
//void switch_task(task* old, task* new, task** last);
void switch_task(task* old, task* new);
//void uswitch_task(task* old, task* new);
uint64_t get_cur_cr3();
void start_sbush();
void add_vma_anon(uint64_t start, uint64_t size);
void add_vma_file(uint64_t vaddr, uint64_t fstart, 
                  uint64_t fsize, uint64_t msize);
void save_child_state(task* p, task* c);

void add_run_queue(task* t);
void add_wait_queue(task* t);
void add_suspend_queue(task* t);
void add_avail_queue(task* t);

uint64_t fork();
uint64_t execve(char* filename, char* argv[], char* envp[]);
void exit(uint32_t status);
uint32_t wait(int *status);
char* getcwd(char* buf, uint32_t size);
int32_t chdir(char* path);
uint32_t getpid();
uint32_t getppid();
uint32_t sleep(uint32_t secs);
void decrement_sleep();
void wakeup_read();
uint64_t sbrk(int32_t incr);
int32_t access(char* fname);
#endif
