#include <sys/os.h>
#include <sys/util.h>

static task* cur_task;
static task task1;
static task task2;
//static task dummytask;
static task maintask;


//static task task[MAX_PROCESSES];

//static task* queue;

void kill_task();

void create_kernel_task(task* t, void (*main)(), uint64_t flags, uint64_t* pml4)
{
   uint64_t* st;
   t->stack   = (uint8_t*)_get_page(); 
   t->reg_r15 = 0;
   t->reg_r14 = 0;
   t->reg_r13 = 0;
   t->reg_r12 = 0;
   t->reg_rbp = 0;
   t->reg_rsp = ((uint64_t)t->stack)+ 0x1000 - 16;
   t->reg_rbx = 0;
   t->reg_rip = (uint64_t) main;
   //t->reg_rflags = flags;
   //t->reg_cr3 = (uint64_t) pml4;

   st = (uint64_t*) t->stack;
   st[511] = (uint64_t)kill_task;
   //st[510] = (uint64_t)kill_task;
   //st[509] = (uint64_t)kill_task;
   //st[510] = (uint64_t)1234;
   t->next = cur_task->next;
   cur_task->next =t;
}

void create_user_task(task* t, void (*main)(), uint64_t flags, uint64_t* pml4)
{
   uint64_t* st;
   t->stack   = (uint8_t*)_get_page(); 
   t->reg_r15 = 0;
   t->reg_r14 = 0;
   t->reg_r13 = 0;
   t->reg_r12 = 0;
   t->reg_rbp = 0;
   t->reg_rsp = ((uint64_t)t->stack)+ 0x1000 - 16;
   t->reg_rbx = 0;
   t->reg_rip = (uint64_t) main;
   //t->reg_rflags = flags;
   //t->reg_cr3 = (uint64_t) pml4;

   st = (uint64_t*) t->stack;
   st[511] = (uint64_t)kill_task;
   //st[510] = (uint64_t)kill_task;
   //st[509] = (uint64_t)kill_task;
   //st[510] = (uint64_t)1234;
   t->next = cur_task->next;
   cur_task->next =t;
}

void kill_task()
{
   kprintf("Inside kill task\n");
   while(1);
   bis(cur_task->flags_task, KILL_TASK); 
}

static void main1()
{
   int x1,x2,x3,x4,x5,x6;
   kprintf("Inside main1\n");

   x1 = 1;
   x2 = 2;
   x3 = 3;
   x4 = 4;
   x5 = 5;
   x6 = 6;
   while(1){
   x1++;
   x2++;
   x3++;
   x4++;
   x5++;
   x6++;
   yield();
   kprintf("Inside main1 - %d,%d,%d,%d,%d,%d\n",x1,x2,x3,x4,x5,x6);
   sleep(5);
   }
   //while(1);
}

static void main2()
{
   while(1){
   kprintf("Inside main2\n");
   yield();
   kprintf("Inside main2 - After yield()\n");
   sleep(5);
   }
}

void init_task_system()
{
   cur_task = &maintask; 
   cur_task->next = &maintask; 
   create_kernel_task(&maintask, 0, 0, NULL);

   create_kernel_task(&task1, main1, 0, NULL);
   create_kernel_task(&task2, main2, 0, NULL);
//   task1.next = &task2;
//   task2.next = &task1;
//   dummytask.next = &task1;
//   cur_task = &dummytask;
}

void yield()
{
   task* me = cur_task;
   //task* last;
   cur_task = cur_task->next;
   //switch_task(me, cur_task, &last);
   switch_task(me, cur_task);
   //if (bit(last->flags_task, KILL_TASK)){
   //   kprintf("Killing a task\n");
      // Free stack page
   //}
}
