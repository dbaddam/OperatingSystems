#include <sys/os.h>
#include <unistd.h>

//static task task1;
//static task task2;
//static task dummytask;
//static task maintask;
//static task usertask;


//static task task[MAX_PROCESSES];

//static task* queue;

void kill_task();

void user_main1()
{
   while (1){
   char* buf = "Sending message from userland1";
   kprintf("Hello user\n");
   write(1, buf, sizeof(buf));
   //__asm__ __volatile__("syscall\n\t"::);
   while(1);
   //uyield();
   }
}

void user_main2()
{
   while (1){
   char* buf = "Sending message from userland2";
   kprintf("Hello user\n");
   write(1, buf, sizeof(buf));
   //__asm__ __volatile__("syscall\n\t"::);
   //while(1);
   uyield();
   }
}

void create_kernel_task(task* t, void (*main)(), uint64_t flags)
{
   t->reg_r15 = 0;
   t->reg_r14 = 0;
   t->reg_r13 = 0;
   t->reg_r12 = 0;
   t->reg_rbp = 0;
   t->reg_rsp = ((uint64_t)&(t->kstack[510]));
   t->reg_rbx = 0;
   t->reg_rip = (uint64_t) main;
   //t->reg_rflags = flags;
   t->reg_cr3 = (uint64_t) kernel_pml4;
   t->state = RUNNABLE_STATE;
   bis(t->flags_task, KERNEL_TASK);
 
   t->kstack[510] = 0; 
   t->next = cur_task->next;
   cur_task->next =t;
}

void create_user_task(void (*main)(), uint64_t flags)
{
   task* t;
   int i;
   for (i = 0;i < MAX_PROCESSES;i++)
   {
       if (!bit(tasks[i].flags_task, ALLOCATED_TASK))
          break;
   }

   if (i == MAX_PROCESSES)
      ERROR("Out of processes");

   t = &tasks[i];
   bis(t->flags_task, ALLOCATED_TASK);
   //uint64_t* pml4 = create_user_page_tables();
   create_kernel_task(t, main, flags);
   t->ustack = (uint8_t*) kmalloc(PAGE_SIZE);
   t->reg_ursp = ((uint64_t)t->ustack)+ PAGE_SIZE - 8;
   //t->reg_cr3 = (uint64_t)pml4;
   //t->reg_rip = t->reg_rip;//%(1<<30);
   bic(t->flags_task, KERNEL_TASK);
}

void kill_task()
{
   kprintf("Inside kill task\n");
   while(1);
   bis(cur_task->flags_task, KILL_TASK); 
}
/*
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
*/
/*task* next_task()
{
    cur_task 
}*/

void init_task_system()
{
   //void* rsp = _get_page();
   //cur_task = &maintask; 
   //cur_task->next = &maintask; 
   //create_kernel_task(&maintask, 0, 0);

   //create_kernel_task(&task1, main1, 0);
   //create_kernel_task(&task2, main2, 0);

   cur_task = &tasks[0]; 
   cur_task->next = cur_task;
   create_user_task(user_main1, 0);
   create_user_task(user_main2, 0);
//   task1.next = &task2;
//   task2.next = &task1;
//   dummytask.next = &task1;
//   cur_task = &dummytask;
}


int first_yield = 1;
void yield()
{
   task* me;
   task dummy_task;

   if (first_yield)
   {
      me = &dummy_task;
      cur_task = &tasks[0];
      first_yield = 0;
   }
   else
   {
      me = cur_task;
      cur_task = cur_task->next;
   }
   //task* last;
   //switch_task(me, cur_task, &last);

   set_tss_rsp((void*)cur_task->reg_rsp);
   if (cur_task->kstack[510] != 0)
      switch_task(me, cur_task);
   else
      uswitch_task(me, cur_task);
   //if (bit(last->flags_task, KILL_TASK)){
   //   kprintf("Killing a task\n");
      // Free stack page
   //}
}
